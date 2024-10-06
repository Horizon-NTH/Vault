#pragma once

#include "XMLNode.h"

#include <array>
#include <string_view>
#include <vector>

using std::literals::string_view_literals::operator""sv;

class XMLParser
{
	using existingAttributes = std::vector<std::string_view>;
	using isSelfClosing = bool;
	using existingTags = std::tuple<std::string_view, existingAttributes, isSelfClosing>;

	static inline const auto m_tags = std::array{
				existingTags{"vault"sv, existingAttributes{
					             "name"sv,
					             "extension"sv,
				             },
				             false},
				existingTags{"file"sv, existingAttributes{
					             "name"sv,
					             "data"sv,
				             },
				             true},
				existingTags{"directory"sv, existingAttributes{
					             "name"sv,
				             },
				             false},
			};

public:
	XMLParser() = delete;

	static std::unique_ptr<XMLNode> parse(std::string&& content);

private:
	static std::vector<std::string> tokenize(std::string&& content);
	static bool is_tag(const std::string_view& token);
	static bool is_self_closing_tag(const std::string_view& tag);
	static bool is_attribute_of(const std::string_view& tag, const std::string_view& token);
	static const existingAttributes& get_attribute_of(const std::string_view& tag);
};
