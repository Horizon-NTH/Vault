#include "Directory.h"
#include <date.h>

Directory::Directory(std::string name, const std::filesystem::file_time_type lastWriteTime, const std::filesystem::perms permissions):
	Node(std::move(name), lastWriteTime, permissions)
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
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
	node.append_attribute("lastWriteTime").set_value(date::format("%F %T", std::chrono::clock_cast<std::chrono::system_clock>(m_lastWriteTime)).c_str());
#endif
	node.append_attribute("permissions").set_value(std::to_string(static_cast<int>(m_permissions)).c_str());
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
	permissions(directory_path, m_permissions);
	std::filesystem::last_write_time(directory_path, m_lastWriteTime);
}
