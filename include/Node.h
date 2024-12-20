#pragma once

#include <filesystem>
#include <pugixml.hpp>

class Node
{
public:
	Node(std::string name, std::filesystem::file_time_type lastWriteTime, std::filesystem::perms permissions);
	virtual ~Node() = default;

	virtual void write_content(pugi::xml_node& parentNode) const = 0;
	virtual void create(const std::filesystem::path& path) const = 0;

protected:
	std::string m_name;
	std::filesystem::file_time_type m_lastWriteTime;
	std::filesystem::perms m_permissions;
};
