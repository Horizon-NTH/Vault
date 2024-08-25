#include "XMLParser.h"

#include <stdexcept>
#include <algorithm>
#include <stack>

std::unique_ptr<XMLNode> XMLParser::parse(std::string&& content)
{
	auto tokens = tokenize(std::move(content));
	if (tokens.empty())
		throw std::runtime_error("Invalid file format: empty file");

	const auto get_data_without_quotes = [](std::string&& data)
		{
			return data.substr(1, data.size() - 2);
		};

	std::unique_ptr<XMLNode> root;
	std::stack<std::reference_wrapper<XMLNode>> nodes;
	for (auto it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (it->starts_with("</"))
		{
			const auto tag = it->substr(2, it->size() - 3);
			if (nodes.empty())
				throw std::runtime_error("Invalid file format: unexpected closing tag " + tag);
			if (const auto& node = nodes.top().get(); node.tag() != tag)
				throw std::runtime_error("Invalid file format: expected closing tag " + std::string(node.tag()) + " but got " + tag);
			nodes.pop();
		}
		else if (it->starts_with('<'))
		{
			const auto tag = it->substr(1);
			if (!is_tag(tag))
				throw std::runtime_error("Invalid file format: unknown tag " + tag);
			auto nodePTR = std::make_unique<XMLNode>(tag);
			auto node = std::ref(*nodePTR);
			if (!root)
				root = std::move(nodePTR);
			else
				nodes.top().get().children().push_back(std::move(nodePTR));

			auto delimiter = "/>"sv;
			if (!is_self_closing_tag(tag))
			{
				delimiter = ">"sv;
				nodes.push(node);
			}
			while (*++it != delimiter)
			{
				if (!it->ends_with('='))
					throw std::runtime_error("Invalid file format: attribute badly formatted");
				const auto attribute = it->substr(0, it->size() - 1);
				if (!is_attribute_of(tag, attribute))
					throw std::runtime_error("Invalid file format: unknown attribute " + attribute + " for tag " += tag);

				auto& attributes = node.get().attributes();
				if (attributes.contains(attribute))
					throw std::runtime_error("Invalid file format: duplicate attribute " + attribute + " for tag " += tag);
				attributes[attribute] = get_data_without_quotes(std::move(*++it));
			}
			if (const auto& attributes = node.get().attributes(); std::ranges::any_of(get_attribute_of(tag), [&](const auto& attribute) { return !attributes.contains(std::string(attribute)); }))
				throw std::runtime_error("Invalid file format: missing attribute for tag " + tag);
		}
		else
			throw std::runtime_error("Invalid file format: unexpected token " + *it);
	}
	if (!nodes.empty())
		throw std::runtime_error("Invalid file format: missing closing tag " + std::string(nodes.top().get().tag()));
	if (!root)
		throw std::runtime_error("Invalid file format: missing root tag");

	return root;
}

std::vector<std::string> XMLParser::tokenize(std::string&& content)
{
	std::vector<std::string> tokens;
	const size_t size = content.size();
	size_t pos = content.find_first_of('<');
	const auto get_until = [&](const char c, const bool includeDelimiter = true) -> std::string
		{
			std::string res;
			do
				res += content[pos++];
			while (pos < size && content[pos] != c);
			if (pos == size)
				throw std::runtime_error(std::string("Invalid file format: missing closing tag ") + c);
			if (includeDelimiter)
				res += content[pos++];
			return res;
		};
	const auto skip_blanks = [&]
		{
			while (pos < size && (content[pos] == ' ' || content[pos] == '\t' || content[pos] == '\n'))
				++pos;
		};
	const auto append = [&](std::string&& token)
		{
			tokens.push_back(std::move(token));
			skip_blanks();
		};

	while (pos < size)
	{
		if (content[pos] == '<')
		{
			if (content[pos + 1] == '/')
				append(get_until('>'));
			else
				append(get_until(' ', false));
		}
		else if (content[pos] == '/')
		{
			append(get_until('>'));
		}
		else if (content[pos] == '>')
		{
			append((++pos, ">"));
		}
		else if (content[pos] == '"')
		{
			append(get_until('"'));
		}
		else
		{
			append(get_until('='));
		}
	}

	return std::move(tokens);
}

bool XMLParser::is_tag(const std::string_view& token)
{
	return std::ranges::any_of(m_tags, [&](const auto& tag) { return std::get<0>(tag) == token; });
}

bool XMLParser::is_self_closing_tag(const std::string_view& tag)
{
	if (!is_tag(tag))
		throw std::invalid_argument("Invalid tag: " + std::string(tag) + " is not a tag");
	return std::ranges::any_of(m_tags, [&](const auto& existingTag) { return std::get<0>(existingTag) == tag && std::get<2>(existingTag); });
}

bool XMLParser::is_attribute_of(const std::string_view& tag, const std::string_view& token)
{
	if (!is_tag(tag))
		throw std::invalid_argument("Invalid tag: " + std::string(tag) + " is not a tag");
	const auto& attributes = *std::ranges::find_if(m_tags, [&](const auto& existingTag) { return std::get<0>(existingTag) == tag; });
	return std::ranges::any_of(std::get<1>(attributes), [&](const auto& attribute) { return attribute == token; });
}

const XMLParser::existingAttributes& XMLParser::get_attribute_of(const std::string_view& tag)
{
	if (!is_tag(tag))
		throw std::invalid_argument("Invalid tag: " + std::string(tag) + " is not a tag");
	return std::get<1>(*std::ranges::find_if(m_tags, [&](const auto& existingTag) { return std::get<0>(existingTag) == tag; }));
}
