#include "Directory.h"

Directory::Directory(std::unique_ptr<Status> status):
	Node(std::move(status))
{
}

std::vector<std::unique_ptr<Node>>& Directory::children()
{
	return m_children;
}

const std::vector<std::unique_ptr<Node>>& Directory::children() const
{
	return m_children;
}

void Directory::write_content(std::ostream& os, const size_t indentation) const
{
	const std::string indentation_str(indentation, '\t');
	os << indentation_str << "<directory name=" << status()->name << ">" << std::endl;
	for (const auto& child : m_children)
	{
		child->write_content(os, indentation + 1);
	}
	os << indentation_str << "</directory>" << std::endl;
}

void Directory::create(const std::filesystem::path& parentPath) const
{
	const auto directory_path = parentPath / m_status->name;
	create_directory(directory_path);
	for (const auto& child : m_children)
	{
		child->create(directory_path);
	}
}
