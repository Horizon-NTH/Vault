#pragma once

#include <filesystem>

class Node
{
public:
	explicit Node(std::filesystem::path name);
	virtual ~Node() = default;

	virtual void write_content(std::ostream& os, size_t indentation) const = 0;
	virtual void create(const std::filesystem::path& path) const = 0;

protected:
	std::filesystem::path m_name;
};
