#include "Vault.h"
#include "File.h"
#include "Utils.h"
#include "CompressionManager.h"

#include <stack>
#include <fstream>
#include <sstream>
#include <botan/base64.h>
#include <chrono>
#include <date.h>
#include <iostream>

Vault::Vault(const std::filesystem::path& file):
	Directory(file.stem().string(), last_write_time(file), status(file).permissions()),
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
	catch (const std::exception& e)
	{
		if (!std::string(e.what()).ends_with("already exists"))
			remove_all(m_file);
		m_file = backUp;
		rename(tempMove, m_file);
		throw;
	}
	remove_all(tempMove);
	m_opened = true;
}

void Vault::close(const std::optional<std::filesystem::path>& destination, const std::optional<std::string>& extension, const bool compress, const bool encrypt)
{
	if (!m_opened)
		throw std::invalid_argument("You can't close a vault that is already closed");
	read_from_dir();
	const auto backUp = m_file;
	const auto tempMove = get_temp_name(backUp.path().parent_path());
	rename(m_file, tempMove);
	m_file = std::filesystem::directory_entry((destination.value_or(m_file.path().parent_path()) / m_name).replace_extension(extension.value_or(".vlt")));
	try { write_to_file(compress, encrypt); }
	catch (const std::exception& e)
	{
		if (!std::string(e.what()).ends_with("already exists"))
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
			{
				const auto& path = entry.path();
				dir.get().children().push_back(std::make_unique<File>(path.filename().string(), entry.last_write_time(), entry.status().permissions(), Botan::base64_encode(File::read(path))));
			}
			else if (entry.is_directory())
			{
				auto directory = std::make_unique<Directory>(entry.path().filename().string(), entry.last_write_time(), entry.status().permissions());
				dirs_to_visit.emplace(entry.path(), *directory);
				dir.get().children().push_back(std::move(directory));
			}
			else
				throw std::runtime_error("Invalid vault file format: " + entry.path().string() + " is not a regular file or directory");
		}
	}
}

void Vault::write_to_dir() const
{
	if (m_file.exists())
		throw std::runtime_error(m_file.path().string() + " already exists");

	Directory::create(m_file.path().parent_path());
}

void Vault::read_from_file()
{
	if (m_opened)
		throw std::runtime_error("The vault " + m_file.path().string() + " is not closed");

	const auto vault_path = m_file.path();
	if (!exists(vault_path))
		throw std::runtime_error(vault_path.string() + " doesn't exists");

	auto doc = pugi::xml_document();
	if (!doc.load_file(vault_path.string().c_str()))
		throw std::runtime_error("Failed to load the XML file: " + vault_path.string());
	auto root = doc.document_element();

	using namespace std::string_view_literals;
	if (root.name() == "encrypted"sv)
	{
		const auto password = ask_password_with_confirmation();
		if (!password)
			throw std::runtime_error("Password confirmation failed");
		const auto data = Botan::base64_decode(root.attribute("data").value());
		const auto nonce = Botan::base64_decode(root.attribute("nonce").value());
		const auto salt = Botan::base64_decode(root.attribute("salt").value());
		const auto decrypted_data = EncryptionManager::decrypt(data, *password, salt, nonce);
		if (!doc.load_buffer(decrypted_data.data(), decrypted_data.size()))
			throw std::runtime_error("Failed to load the decrypted XML data");
		root = doc.document_element();
	}
	if (root.name() == "compressed"sv)
	{
		const auto data = Botan::base64_decode(root.attribute("data").value());
		const auto originalSize = std::stoul(root.attribute("originalSize").value());
		const auto decompressedData = CompressionManager::uncompress(data, originalSize);
		if (!doc.load_buffer(decompressedData.data(), decompressedData.size()))
			throw std::runtime_error("Failed to load the decrypted XML data");
		root = doc.document_element();
	}
	if (root.name() != "vault"sv)
		throw std::runtime_error("Invalid vault file format: missing vault tag");
	m_name = root.attribute("name").value();
	if (root.attribute("permissions"))
		m_permissions = static_cast<std::filesystem::perms>(root.attribute("permissions").as_uint());
	else
		m_permissions = std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all;
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
	std::chrono::system_clock::time_point lastWriteTime;
	std::istringstream(root.attribute("lastWriteTime").value()) >> date::parse("%F %T", lastWriteTime);
	m_lastWriteTime = std::chrono::clock_cast<std::chrono::file_clock>(lastWriteTime);
#endif

	std::deque<std::pair<pugi::xml_node, std::reference_wrapper<Directory>>> dirs;
	dirs.emplace_back(root, std::ref(*this));
	while (!dirs.empty())
	{
		for (auto& [xmlNode, dir] = dirs.front(); auto& child : xmlNode.children())
		{
			if (child.name() == "file"sv)
			{
				const auto name = child.attribute("name").value();
				std::filesystem::perms permissions;
				if (child.attribute("permissions"))
					permissions = static_cast<std::filesystem::perms>(child.attribute("permissions").as_uint());
				else
					permissions = std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all;
				auto data = child.attribute("data").value();
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
				std::istringstream(child.attribute("lastWriteTime").value()) >> date::parse("%F %T", lastWriteTime);
				dir.get().children().push_back(std::make_unique<File>(name, std::chrono::clock_cast<std::chrono::file_clock>(lastWriteTime), permissions, data));
#else
				dir.get().children().push_back(std::make_unique<File>(name, std::filesystem::file_time_type::clock::now(), permissions, data));
#endif
			}
			else if (child.name() == "directory"sv)
			{
				const auto name = child.attribute("name").value();
				std::filesystem::perms permissions;
				if (child.attribute("permissions"))
					permissions = static_cast<std::filesystem::perms>(child.attribute("permissions").as_uint());
				else
					permissions = std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all;
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
				std::istringstream(child.attribute("lastWriteTime").value()) >> date::parse("%F %T", lastWriteTime);
				auto directory = std::make_unique<Directory>(name, std::chrono::clock_cast<std::chrono::file_clock>(lastWriteTime), permissions);
#else
				auto directory = std::make_unique<Directory>(name, std::filesystem::file_time_type::clock::now(), permissions);
#endif
				dirs.emplace_back(child, std::ref(*directory));
				dir.get().children().push_back(std::move(directory));
			}
			else
				throw std::runtime_error("Invalid vault file format: unknown tag " + std::string(child.name()));
		}
		dirs.pop_front();
	}
}

void Vault::write_to_file(bool compress, const bool encrypt) const
{
	if (m_file.exists())
		throw std::runtime_error(m_file.path().string() + " already exists");

	std::ofstream vault_file(m_file.path().string());
	if (!vault_file.is_open())
		throw std::ios_base::failure("Failed to open the file: " + m_file.path().string());

	auto doc = pugi::xml_document();
	write_content(doc);
	const auto docToData = [&doc]() -> Botan::secure_vector<std::uint8_t>
		{
			std::ostringstream vaultContent;
			doc.save(vaultContent);
			auto str = vaultContent.str();
			return {str.begin(), str.end()};
		};
	if (compress)
	{
		CompressionManager::Data data = docToData();
		const auto compressedData = CompressionManager::compress(data);
		doc.reset();
		auto root = doc.append_child("compressed");
		if (!root)
			throw std::runtime_error("Failed to create the XML node");
		root.append_attribute("originalSize").set_value(std::to_string(data.size()).c_str());
		root.append_attribute("data").set_value(base64_encode(compressedData).c_str());
	}
	if (encrypt)
	{
		const auto password = ask_password_with_confirmation();
		if (!password)
			throw std::runtime_error("Password confirmation failed");
		const auto salt = EncryptionManager::generate_new_salt();
		EncryptionManager::Data data = docToData();
		const auto [encryptedData, nonce] = EncryptionManager::encrypt(std::move(data), *password, salt);
		doc.reset();
		auto root = doc.append_child("encrypted");
		if (!root)
			throw std::runtime_error("Failed to create the XML node");
		root.append_attribute("data").set_value(base64_encode(encryptedData).c_str());
		root.append_attribute("nonce").set_value(base64_encode(nonce).c_str());
		root.append_attribute("salt").set_value(base64_encode(salt).c_str());
	}
	doc.save(vault_file, "\t", pugi::format_no_declaration | pugi::format_indent);
}

void Vault::write_content(pugi::xml_node& parentNode) const
{
	auto node = parentNode.append_child("vault");
	if (!node)
		throw std::runtime_error("Failed to create the XML node");
	node.append_attribute("name").set_value(m_name.c_str());
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
	node.append_attribute("lastWriteTime").set_value(date::format("%F %T", std::chrono::clock_cast<std::chrono::system_clock>(m_lastWriteTime)).c_str());
#endif
	node.append_attribute("permissions").set_value(std::to_string(static_cast<int>(m_permissions)).c_str());
	for (const auto& child : m_children)
	{
		child->write_content(node);
	}
}
