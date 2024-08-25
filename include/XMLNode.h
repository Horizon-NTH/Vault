#pragma once

#include <map>
#include <memory>
#include <string_view>
#include <vector>

class XMLNode
{
public:
	using Value = std::string;
	using Attribute = std::string;

	XMLNode() = default;
	explicit XMLNode(std::string tag);

	[[nodiscard]] const std::map<Attribute, Value>& attributes() const;
	[[nodiscard]] const std::string& tag() const;
	[[nodiscard]] const std::vector<std::unique_ptr<XMLNode>>& children() const;

	[[nodiscard]] std::map<Attribute, Value>& attributes();
	[[nodiscard]] std::vector<std::unique_ptr<XMLNode>>& children();

private:
	std::map<Attribute, Value> m_attributes;
	std::string m_tag;
	std::vector<std::unique_ptr<XMLNode>> m_children;
};
