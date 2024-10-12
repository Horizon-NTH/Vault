#include "Vault.h"
#include "File.h"
#include "XMLParser.h"
#include "Utils.h"

#include <stack>
#include <fstream>
#include <sstream>
#include <botan/base64.h>

Vault::Vault(const std::filesystem::path& file):
	Directory(file.stem().string()),
	m_file(file),
	m_opened(!m_file.is_regular_file())
{
	if (!m_file.exists())
		throw std::runtime_error(file.string() + " does not exist");
	if (!m_file.is_regular_file() && !m_file.is_directory())
		throw std::runtime_error(file.string() + " is not a valid vault file");
}

void Vault::open(const std::optional<std::filesystem::path>& destination)
{
	if (m_opened)
		throw std::runtime_error("You can't open a vault that is already opened");
	read_from_file();
	const auto backUp = m_file;
	const auto tempMove = get_temp_name(backUp.path().parent_path());
	rename(m_file, tempMove);
	m_file = std::filesystem::directory_entry(destination.value_or(m_file.path().parent_path()) / m_name);
	try { write_to_dir(); }
	catch ([[maybe_unused]] const std::exception& e)
	{
		remove_all(m_file);
		m_file = backUp;
		rename(tempMove, m_file);
		throw;
	}
	remove_all(tempMove);
	m_opened = true;
}

void Vault::close(const std::optional<std::filesystem::path>& destination, const std::optional<std::string>& extension, const bool encrypt)
{
	if (!m_opened)
		throw std::invalid_argument("You can't close a vault that is already closed");
	read_from_dir();
	const auto backUp = m_file;
	const auto tempMove = get_temp_name(backUp.path().parent_path());
	rename(m_file, tempMove);
	m_file = std::filesystem::directory_entry((destination.value_or(m_file.path().parent_path()) / m_name).replace_extension(extension.value_or(".vlt")));
	try { write_to_file(encrypt); }
	catch ([[maybe_unused]] const std::exception& e)
	{
		remove_all(m_file);
		m_file = backUp;
		rename(tempMove, m_file);
		throw;
	}
	remove_all(tempMove);
	m_opened = false;
}

void Vault::read_from_dir()
{
	if (!m_opened)
		throw std::runtime_error("The vault " + m_file.path().string() + " is not opened");

	std::stack<std::pair<std::filesystem::path, std::reference_wrapper<Directory>>> dirs_to_visit;
	dirs_to_visit.emplace(m_file.path(), std::reference_wrapper<Directory>(*this));

	while (!dirs_to_visit.empty())
	{
		auto [dir_path, dir] = dirs_to_visit.top();
		dirs_to_visit.pop();

		for (const auto& entry : std::filesystem::directory_iterator(dir_path))
		{
			if (entry.is_symlink())
				throw std::runtime_error("Invalid vault file format: " + entry.path().string() + " is a symlink");
			if (entry.is_regular_file())
				dir.get().children().push_back(std::make_unique<File>(entry.path().filename().string(), Botan::base64_encode(File::read(entry.path()))));
			else if (entry.is_directory())
			{
				auto directory = std::make_unique<Directory>(entry.path().filename().string());
				dirs_to_visit.emplace(entry.path(), *directory);
				dir.get().children().push_back(std::move(directory));
			}
			else
				throw std::runtime_error("Invalid vault file format: " + entry.path().string() + " is not a regular file or directory");
		}
	}
}

void Vault::write_to_dir()
{
	const auto vault_path = std::filesystem::path(m_file.path().parent_path() / m_name).replace_extension();

	if (exists(vault_path))
		throw std::runtime_error(m_file.path().string() + " already exists");

	Directory::create(vault_path.parent_path());
	m_file.assign(vault_path);
}

void Vault::read_from_file()
{
	if (m_opened)
		throw std::runtime_error("The vault " + m_file.path().string() + " is not closed");

	const auto vault_path = m_file.path();
	if (!exists(vault_path))
		throw std::runtime_error(vault_path.string() + " doesn't exists");

	const std::ifstream vault_file(vault_path.string());
	if (!vault_file.is_open())
		throw std::ios_base::failure("Failed to open the file: " + vault_path.string());

	const std::stringstream ss = std::stringstream{} << vault_file.rdbuf();
	std::string content = ss.str();

	extract_from_xml(std::move(content));
}

void Vault::write_to_file(const bool encrypt) const
{
	if (m_file.exists())
		throw std::runtime_error(m_file.path().string() + " already exists");

	std::ofstream vault_file(m_file.path().string());
	if (!vault_file.is_open())
		throw std::ios_base::failure("Failed to open the file: " + m_file.path().string());

	if (!encrypt)
	{
		write_content(vault_file, 0);
	}
	else
	{
		std::stringstream vaultContent;
		write_content(vaultContent, 0);
		const auto password = ask_password_with_confirmation();
		if (!password)
			throw std::runtime_error("Password confirmation failed");
		const auto salt = EncryptionManager::generate_new_salt();
		auto str = vaultContent.str();
		auto data = EncryptionManager::Data(str.begin(), str.end());
		const auto [encrypted_data, nonce] = EncryptionManager::encrypt(std::move(data), *password, salt);
		vault_file << "<encrypted data=\"" << Botan::base64_encode(encrypted_data) << "\" nonce=\"" << Botan::base64_encode(nonce) << "\" salt=\""
				<< Botan::base64_encode(salt) << "\"/>";
	}
}

void Vault::extract_from_xml(std::string&& content)
{
	auto node = XMLParser::parse(std::move(content));

	if (node->tag() == "encrypted")
	{
		const auto data = node->attributes().at("data");
		const auto nonce = node->attributes().at("nonce");
		const auto salt = node->attributes().at("salt");
		const auto password = ask_password_with_confirmation();
		if (!password)
			throw std::runtime_error("Password confirmation failed");
		auto decrypted_data = EncryptionManager::decrypt(Botan::base64_decode(data), *password, Botan::base64_decode(salt), Botan::base64_decode(nonce));
		node = XMLParser::parse(std::string(decrypted_data.begin(), decrypted_data.end()));
	}

	if (node->tag() != "vault")
		throw std::runtime_error("Invalid vault file format: missing vault tag");
	m_name = node->attributes().at("name");

	std::deque<std::pair<std::unique_ptr<XMLNode>, std::reference_wrapper<Directory>>> dirs;
	dirs.emplace_back(std::move(node), std::ref(*this));
	while (!dirs.empty())
	{
		for (auto& [xmlNode, dir] = dirs.front(); auto& child : xmlNode->children())
		{
			if (child->tag() == "file")
			{
				const auto name = child->attributes().at("name");
				auto data = child->attributes().at("data");
				dir.get().children().push_back(std::make_unique<File>(name, std::move(data)));
			}
			else if (child->tag() == "directory")
			{
				const auto name = child->attributes().at("name");
				auto directory = std::make_unique<Directory>(name);
				dirs.emplace_back(std::move(child), std::ref(*directory));
				dir.get().children().push_back(std::move(directory));
			}
			else
				throw std::runtime_error("Invalid vault file format: unknown tag " + std::string(child->tag()));
		}
		dirs.pop_front();
	}
}

void Vault::write_content(std::ostream& os, const size_t indentation) const
{
	const std::string indentation_str(indentation, '\t');
	os << indentation_str << "<vault name=\"" << m_name << "\">" << std::endl;
	for (const auto& child : m_children)
	{
		child->write_content(os, indentation + 1);
	}
	os << indentation_str << "</vault>" << std::endl;
}
