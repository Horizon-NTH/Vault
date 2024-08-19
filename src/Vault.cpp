#include "../include/Vault.h"
#include "../include/File.h"
#include "../include/Base64.h"
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

	std::ifstream vault_file(vault_path.string());
	if (!vault_file.is_open())
		throw std::ios_base::failure("Failed to open the file: " + vault_path.string());

	const std::stringstream ss = std::stringstream{} << vault_file.rdbuf();
	std::string content = ss.str();

	auto tokens = tokenize(std::move(content));
	parse_tokens(tokens);

	vault_file.close();
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

void Vault::parse_tokens(std::vector<std::string>& tokens)
{
	std::stack<std::reference_wrapper<Directory>> dirs;
	dirs.emplace(std::ref(*this));
	const auto get_data_without_quotes = [](std::string&& data)
		{
			return data.substr(1, data.size() - 2);
		};

	if (tokens.empty())
		throw std::runtime_error("Invalid vault file format: empty file");
	if (tokens.front() != "<vault")
		throw std::runtime_error("Invalid vault file format: missing vault tag");
	if (tokens.back() != "</vault>")
		throw std::runtime_error("Invalid vault file format: missing closing vault tag");
	if (tokens.size() < 3)
		throw std::runtime_error("Invalid vault file format: missing vault attributes");
	for (auto it = tokens.begin(); it != tokens.end();)
	{
		if (*it == "<vault")
		{
			while (*++it != ">")
			{
				if (*it == "name=")
					m_status->name = get_data_without_quotes(std::move(*++it));
				else
					throw std::runtime_error("Invalid vault file format: unknown vault attribute");
			}
		}
		else if (*it == "<file")
		{
			auto status = std::make_unique<Status>();
			std::string data;
			while (*++it != "/>")
			{
				if (*it == "name=")
					status->name = get_data_without_quotes(std::move(*++it));
				else if (*it == "data=")
					data = get_data_without_quotes(std::move(*++it));
				else
					throw std::runtime_error("Invalid vault file format: unknown file attribute");
			}
			dirs.top().get().children().push_back(std::make_unique<File>(std::move(status), std::move(data)));
		}
		else if (*it == "<directory")
		{
			auto status = std::make_unique<Status>();
			while (*++it != ">")
			{
				if (*it == "name=")
					status->name = get_data_without_quotes(std::move(*++it));
				else
					throw std::runtime_error("Invalid vault file format: unknown directory attribute");
			}
			auto directory = std::make_unique<Directory>(std::move(status));
			auto ref = std::ref(*directory);
			dirs.top().get().children().push_back(std::move(directory));
			dirs.push(ref);
		}
		else if (*it == "</directory>")
		{
			dirs.pop();
		}
		else if (*it == "</vault>")
		{
			if (dirs.size() != 1)
				throw std::runtime_error("Invalid vault file format: missing closing directory tag");
			if (++it != tokens.end())
				throw std::runtime_error("Invalid vault file format: unexpected tokens after closing vault tag");
			return;
		}
		else
		{
			throw std::runtime_error("Invalid vault file format: unknown tag");
		}
		++it;
	}
}

std::vector<std::string> Vault::tokenize(std::string&& content)
{
	std::vector<std::string> tokens;
	const size_t size = content.size();
	size_t pos = content.find_first_of('<');
	const auto get_until = [&](const char c, const bool includeDelimiter = true) -> std::string
		{
			std::string res;
			do
				res += content[pos++]; while (pos < size && content[pos] != c);
			if (pos == size)
				throw std::runtime_error(std::string("Invalid vault file format: missing closing tag ") + c);
			if (includeDelimiter)
				res += content[pos++];
			return res;
		};
	const auto skip_blanks = [&]
		{
			while (pos < size && (content[pos] == ' ' || content[pos] == '\t' || content[pos] == '\n'))
				++pos;
		};
	const auto append = [&](std::string&& token)
		{
			tokens.push_back(std::move(token));
			skip_blanks();
		};

	while (pos < size)
	{
		if (content[pos] == '<')
		{
			if (content[pos + 1] == '/')
				append(get_until('>'));
			else
				append(get_until(' ', false));
		}
		else if (content[pos] == '/')
		{
			append(get_until('>'));
		}
		else if (content[pos] == '>')
		{
			append((++pos, ">"));
		}
		else if (content[pos] == '"')
		{
			append(get_until('"'));
		}
		else
		{
			append(get_until('='));
		}
	}

	return std::move(tokens);
}
