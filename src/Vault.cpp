#include "Vault.h"
#include "File.h"
#include "Base64.h"
#include "XMLParser.h"

#include <stack>
#include <fstream>
#include <sstream>

Vault::Status::Status(const std::filesystem::path& name, bool opened, const std::optional<std::string>& extension):
	Directory::Status(name),
	opened(opened),
	extension(extension)
{
}

Vault::Vault(const std::filesystem::directory_entry& file, const std::optional<std::string>& extension):
	Directory(nullptr),
	m_file(file)
{
	if (!m_file.exists())
		throw std::runtime_error(file.path().string() + " does not exist");
	if (!m_file.is_regular_file() && !m_file.is_directory())
		throw std::invalid_argument(file.path().string() + " is not a file or directory");
	if (m_file.is_regular_file())
	{
		m_status = std::make_unique<Status>(m_file.path().stem(), false, extension);
		read_from_file();
	}
	else
	{
		m_status = std::make_unique<Status>(m_file.path().filename(), true, extension);
		read_from_dir();
	}
}

void Vault::open(const std::filesystem::path& path, const std::optional<std::filesystem::path>& destination)
{
	auto vault = Vault(std::filesystem::directory_entry(path));
	if (const auto status = dynamic_cast<Status*>(vault.status().get()); !status || status->opened)
		throw std::invalid_argument("You can't open a vault that is already opened");
	vault.remove();
	vault.m_file = std::filesystem::directory_entry(destination.value_or(path.parent_path()) / vault.m_status->name);
	vault.write_to_dir();
}

void Vault::close(const std::filesystem::path& path, const std::optional<std::filesystem::path>& destination, const std::optional<std::string>& extension)
{
	auto vault = Vault(std::filesystem::directory_entry(path), extension);
	if (const auto status = dynamic_cast<Status*>(vault.status().get()); !status || !status->opened)
		throw std::invalid_argument("You can't close a vault that is already closed");
	vault.remove();
	vault.m_file = std::filesystem::directory_entry(destination.value_or(path.parent_path()) / vault.m_status->name);
	vault.write_to_file();
}

void Vault::read_from_dir()
{
	if (const auto status = dynamic_cast<Status*>(m_status.get()); !status || !status->opened)
		throw std::runtime_error("The directory " + m_file.path().string() + " is not opened");

	std::stack<std::pair<std::filesystem::path, std::reference_wrapper<Directory>>> dirs_to_visit;
	dirs_to_visit.emplace(m_file.path(), std::reference_wrapper<Directory>(*this));

	while (!dirs_to_visit.empty())
	{
		auto [dir_path, dir] = dirs_to_visit.top();
		dirs_to_visit.pop();

		for (const auto& entry : std::filesystem::directory_iterator(dir_path))
		{
			if (entry.is_regular_file())
				dir.get().children().push_back(std::make_unique<File>(std::make_unique<Directory::Status>(entry.path().filename()), Base64::encode(File::read(entry.path()))));
			else if (entry.is_directory())
			{
				auto directory = std::make_unique<Directory>(std::make_unique<Directory::Status>(entry.path().filename()));
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
	auto vault_path = std::filesystem::path(m_file.path()).replace_extension();

	if (exists(vault_path))
		throw std::runtime_error(m_file.path().string() + " already exists");

	create(vault_path.remove_filename());
}

void Vault::read_from_file()
{
	if (const auto status = dynamic_cast<Status*>(m_status.get()); !status || status->opened)
		throw std::runtime_error("The file " + m_file.path().string() + " is not closed");

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

void Vault::write_to_file() const
{
	const auto status = dynamic_cast<Status*>(m_status.get());
	const auto vault_path = std::filesystem::path(m_file.path()).replace_extension(status ? status->extension.value_or(".vlt") : ".vlt");

	if (exists(vault_path))
		throw std::runtime_error(vault_path.string() + " already exists");

	std::ofstream vault_file(vault_path.string());
	if (!vault_file.is_open())
		throw std::ios_base::failure("Failed to open the file: " + vault_path.string());

	write_content(vault_file, 0);
	vault_file.close();
}

void Vault::remove() const
{
	remove_all(m_file);
}

void Vault::extract_from_xml(std::string&& content)
{
	auto node = XMLParser::parse(std::move(content));
	if (node->tag() != "vault")
		throw std::runtime_error("Invalid vault file format: missing vault tag");
	m_status->name = node->attributes().at("name");

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
				dir.get().children().push_back(std::make_unique<File>(std::make_unique<File::Status>(name), std::move(data)));
			}
			else if (child->tag() == "directory")
			{
				const auto name = child->attributes().at("name");
				auto directory = std::make_unique<Directory>(std::make_unique<Directory::Status>(name));
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
	os << indentation_str << "<vault name=" << m_status->name << ">" << std::endl;
	for (const auto& child : m_children)
	{
		child->write_content(os, indentation + 1);
	}
	os << indentation_str << "</vault>" << std::endl;
}
