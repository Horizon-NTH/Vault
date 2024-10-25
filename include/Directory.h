#pragma once

#include "Node.h"
#include <vector>

class Directory : public Node
{
public:
	Directory(std::string name, std::filesystem::file_time_type lastWriteTime, std::filesystem::perms permissions);

	[[nodiscard]] std::vector<std::unique_ptr<Node>>& children();
	[[nodiscard]] const std::vector<std::unique_ptr<Node>>& children() const;

protected:
	std::vector<std::unique_ptr<Node>> m_children;

	void write_content(pugi::xml_node& parentNode) const override;
	void create(const std::filesystem::path& parentPath) const override;
};
