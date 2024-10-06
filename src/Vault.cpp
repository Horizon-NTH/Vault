#include "Vault.h"
#include "File.h"
#include "XMLParser.h"

#include <stack>
#include <fstream>
#include <sstream>
#include <botan/base64.h>
#include <yaml-cpp/yaml.h>

Vault::Vault(const std::string& name, const std::optional<std::filesystem::directory_entry>& from, const std::optional<std::filesystem::path>& destination, const std::optional<std::string>& extension):
	Directory(name),
	m_opened(true),
	m_name(name),
	m_extension(extension.value_or(".vlt"))
{
	const auto vault_path = destination.value_or(std::filesystem::current_path()) / name;
	if (from && !from->exists())
		throw std::runtime_error(from->path().string() + " does not exist");
	if (exists(vault_path) && (from && from->path() != vault_path))
		throw std::runtime_error(vault_path.string() + " already exists");
	if (from)
		std::filesystem::rename(*from, vault_path);
	create_directory(vault_path);
	m_file.assign(vault_path);
	if (!m_extension.empty() && !m_extension.starts_with('.'))
		m_extension.insert(0, ".");
	write_settings();
}

Vault::Vault(const std::filesystem::path& file):
	Directory(file.stem()),
	m_file(file),
	m_opened(!m_file.is_regular_file())
{
	if (!m_file.exists())
		throw std::runtime_error(file.string() + " does not exist");
	if (!is_vault(file))
		throw std::runtime_error(file.string() + " is not a vault");
}

void Vault::open(const std::optional<std::filesystem::path>& destination)
{
	if (m_opened)
		throw std::runtime_error("You can't open a vault that is already opened");
	read_from_file();
	remove();
	m_file = std::filesystem::directory_entry(destination.value_or(m_file.path().parent_path()) / m_name);
	write_to_dir();
	m_opened = true;
	write_settings();
}

void Vault::close(const std::optional<std::filesystem::path>& destination)
{
	if (!m_opened)
		throw std::invalid_argument("You can't close a vault that is already closed");
	read_settings();
	read_from_dir();
	remove();
	m_file = std::filesystem::directory_entry(destination.value_or(m_file.path().parent_path()) / m_name);
	write_to_file();
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
			if (entry.path() == m_file.path() / ".vlt")
				continue;
			if (entry.is_regular_file())
				dir.get().children().push_back(std::make_unique<File>(entry.path().filename(), Botan::base64_encode(File::read(entry.path()))));
			else if (entry.is_directory())
			{
				auto directory = std::make_unique<Directory>(entry.path().filename());
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

void Vault::write_to_file()
{
	const auto vault_path = std::filesystem::path(m_file.path().parent_path() / m_name).replace_extension(m_extension);

	if (exists(vault_path))
		throw std::runtime_error(vault_path.string() + " already exists");

	std::ofstream vault_file(vault_path.string());
	if (!vault_file.is_open())
		throw std::ios_base::failure("Failed to open the file: " + vault_path.string());

	write_content(vault_file, 0);
	m_file.assign(vault_path);
}

void Vault::read_settings()
{
	if (!m_opened)
		throw std::runtime_error("The vault " + m_file.path().string() + " is not opened");

	const auto settingsPath = m_file.path() / ".vlt";
	if (!exists(settingsPath))
		throw std::runtime_error("The vault " + m_file.path().string() + " doesn't have settings file");

	YAML::Node settings = YAML::LoadFile(settingsPath.string());

	if (!settings.IsMap())
		throw std::runtime_error("Invalid vault settings file format: the file is not a map");

	if (!settings["name"])
		throw std::runtime_error("Invalid vault settings file format: missing name key");
	m_name = settings["name"].as<std::string>();

	if (!settings["extension"])
		throw std::runtime_error("Invalid vault settings file format: missing extension key");
	m_extension = settings["extension"].as<std::string>();
}

void Vault::write_settings() const
{
	if (!m_opened)
		throw std::runtime_error("The vault " + m_file.path().string() + " is not opened");

	std::ofstream settingsFile(m_file.path() / ".vlt");
	if (!settingsFile.is_open())
		throw std::ios_base::failure("Failed to open the file: " + (m_file.path() / ".vlt").string());

	YAML::Emitter emitter(settingsFile);
	emitter << YAML::BeginMap;
	emitter << YAML::Key << "name" << YAML::Value << m_name;
	emitter << YAML::Key << "extension" << YAML::Value << m_extension << YAML::EndMap;
}

void Vault::remove() const
{
	remove_all(m_file);
}

bool Vault::is_vault(const std::filesystem::path& path)
{
	return is_regular_file(path) || (is_directory(path) && exists(path / ".vlt"));
}

void Vault::extract_from_xml(std::string&& content)
{
	auto node = XMLParser::parse(std::move(content));
	if (node->tag() != "vault")
		throw std::runtime_error("Invalid vault file format: missing vault tag");
	m_name = node->attributes().at("name");
	m_extension = node->attributes().at("extension");

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
	os << indentation_str << "<vault name=\"" << m_name << "\" extension=\"" << m_extension << "\">" << std::endl;
	for (const auto& child : m_children)
	{
		child->write_content(os, indentation + 1);
	}
	os << indentation_str << "</vault>" << std::endl;
}
