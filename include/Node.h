#pragma once

#include <filesystem>
#include <pugixml.hpp>

class Node
{
public:
	explicit Node(std::string name);
	virtual ~Node() = default;

	virtual void write_content(pugi::xml_node& parentNode) const = 0;
	virtual void create(const std::filesystem::path& path) const = 0;

protected:
	std::string m_name;
};
