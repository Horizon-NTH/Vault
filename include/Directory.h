#pragma once

#include "Node.h"
#include <vector>

class Directory : public Node
{
public:
	explicit Directory(std::string name);

	[[nodiscard]] std::vector<std::unique_ptr<Node>>& children();
	[[nodiscard]] const std::vector<std::unique_ptr<Node>>& children() const;

protected:
	std::vector<std::unique_ptr<Node>> m_children;

	void write_content(pugi::xml_node& parentNode) const override;
	void create(const std::filesystem::path& parentPath) const override;
};
