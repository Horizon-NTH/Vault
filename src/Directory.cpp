#include "Directory.h"
#include <format>
#include <chrono>

Directory::Directory(std::string name, const std::filesystem::file_time_type lastWriteTime):
	Node(std::move(name), lastWriteTime)
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
	node.append_attribute("lastWriteTime").set_value(std::format("{}", m_lastWriteTime).c_str());
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
	last_write_time(directory_path, m_lastWriteTime);
}
