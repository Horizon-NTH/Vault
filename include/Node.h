#pragma once

#include <filesystem>

class Node
{
public:
	explicit Node(std::string name);
	virtual ~Node() = default;

	virtual void write_content(std::ostream& os, size_t indentation) const = 0;
	virtual void create(const std::filesystem::path& path) const = 0;

protected:
	std::string m_name;
};
