#include "Directory.h"

Directory::Directory(std::string name):
	Node(std::move(name))
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

void Directory::write_content(pugi::xml_node& parentNode) const
{
	auto node = parentNode.append_child("directory");
	if (!node)
		throw std::runtime_error("Failed to create directory node");
	node.append_attribute("name").set_value(m_name.c_str());
	for (const auto& child : m_children)
	{
		child->write_content(node);
	}
}

void Directory::create(const std::filesystem::path& parentPath) const
{
	const auto directory_path = parentPath / m_name;
	create_directory(directory_path);
	for (const auto& child : m_children)
	{
		child->create(directory_path);
	}
}
