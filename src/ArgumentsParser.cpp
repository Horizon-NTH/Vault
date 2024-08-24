#include "ArgumentsParser.h"

#include <stdexcept>
#include <algorithm>
#include <ranges>

ArgumentsParser::ArgumentsParser(const std::span<const char*>& args)
{
	m_args.reserve(args.size());
	std::copy(args.begin() + 1, args.end(), std::back_inserter(m_args));
	parse();
}

const std::string_view& ArgumentsParser::command() const
{
	return m_command;
}

const ArgumentsParser::optionsList& ArgumentsParser::options() const
{
	return m_options;
}

void ArgumentsParser::parse()
{
	if (m_args.empty())
	{
		m_command = "help"sv;
		return;
	}
	const auto front = m_args.front();
	if (!is_sub_command(front))
	{
		const auto option = get_option(front, "vault"sv);
		if (!option)
			throw std::invalid_argument("Invalid vault option: " + std::string(front));
		m_command = option.value();
		return;
	}
	m_command = front;
	for (auto it = m_args.begin() + 1; it != m_args.end(); ++it)
	{
		if (const auto option = get_option(*it, m_command); !option)
			add_positional_option(*it);
		else
		{
			if (m_options.contains(*option))
				throw std::invalid_argument("Option " + std::string(*option) + " already has the value " + std::string(m_options[*option].value()));
			if (const auto value = get_integrated_value(*it); !value && it + 1 != m_args.end() && !get_option(*(it + 1), m_command))
				m_options[*option] = *++it;
			else
				m_options[*option] = value;
		}
	}
}

void ArgumentsParser::add_positional_option(const std::string_view& arg)
{
	for (const auto& option : std::ranges::find_if(m_commands, [&](const auto& c) { return c.first == m_command; })->second)
	{
		if (!m_options.contains(option))
		{
			m_options[option] = arg;
			return;
		}
	}
}

std::optional<std::string_view> ArgumentsParser::get_option(const std::string_view& arg, const std::string_view& command)
{
	if (arg.starts_with("--"))
	{
		auto option = arg.substr(2);
		if (const auto equalIndex = arg.find_first_of('='); equalIndex != std::string_view::npos)
			option = arg.substr(2, equalIndex - 2);
		if (is_option_of(command, option))
			return option;
		throw std::invalid_argument("Invalid option: " + std::string(arg) + " is not an option of command " + std::string(command));
	}
	if (arg.size() > 1 && arg[0] == '-')
	{
		const auto existingOptions = get_existing_options_of(command);
		if (const auto it = std::ranges::find_if(existingOptions, [&](const auto& opt) { return opt[0] == arg[1]; }); it != existingOptions.end())
			return *it;
		throw std::invalid_argument("Invalid option: " + std::string(arg) + " is not an option of command " + std::string(command));
	}
	return std::nullopt;
}

std::optional<std::string_view> ArgumentsParser::get_integrated_value(const std::string_view& arg)
{
	if (arg.starts_with("--"))
	{
		if (const auto equalIndex = arg.find_first_of('='); equalIndex != std::string_view::npos)
			return arg.substr(equalIndex + 1);
		return std::nullopt;
	}
	if (arg[0] == '-')
	{
		if (arg.size() > 2)
			return arg.substr(2);
		if (arg.size() == 2)
			return std::nullopt;
	}
	return arg;
}

bool ArgumentsParser::is_sub_command(const std::string_view& arg)
{
	return std::any_of(m_commands.begin() + 1, m_commands.end(), [&](const auto& command) { return command.first == arg; });
}

bool ArgumentsParser::is_option_of(const std::string_view& command, const std::string_view& option)
{
	if (!is_sub_command(command) && command != "vault"sv)
		throw std::invalid_argument("Not a valid command: " + std::string(command));
	return std::ranges::any_of(get_existing_options_of(command), [&](const auto& opt) { return opt == option; });
}

const std::vector<std::string_view>& ArgumentsParser::get_existing_options_of(const std::string_view& command)
{
	if (!is_sub_command(command) && command != "vault"sv)
		throw std::invalid_argument("Not a valid command: " + std::string(command));
	return std::ranges::find_if(m_commands, [&](const auto& c) { return c.first == command; })->second;
}
