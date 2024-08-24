#pragma once

#include <span>
#include <array>
#include <map>
#include <optional>
#include <vector>
#include <string_view>

using namespace std::literals::string_view_literals;

class ArgumentsParser
{
	using existingOptions = std::vector<std::string_view>;
	using existingCommand = std::pair<std::string_view, existingOptions>;

	static inline const auto m_commands = std::array{
				existingCommand{"vault"sv, existingOptions{
					                "help"sv,
					                "version"sv,
				                }},
				existingCommand{"open"sv, existingOptions{
					                "vault"sv,
					                "destination"sv,
				                }},
				existingCommand{"close"sv, existingOptions{
					                "vault"sv,
					                "destination"sv,
					                "extension"sv,
				                }},
			};

public:
	using optionsList = std::map<std::string_view, std::optional<std::string_view>>;

	explicit ArgumentsParser(const std::span<const char*>& args);

	[[nodiscard]] const std::string_view& command() const;
	[[nodiscard]] const optionsList& options() const;

private:
	std::vector<std::string_view> m_args;
	std::string_view m_command;
	optionsList m_options;

	void parse();
	void add_positional_option(const std::string_view& arg);
	static std::optional<std::string_view> get_option(const std::string_view& arg, const std::string_view& command);
	static std::optional<std::string_view> get_integrated_value(const std::string_view& arg);
	[[nodiscard]] static bool is_sub_command(const std::string_view& arg);
	[[nodiscard]] static bool is_option_of(const std::string_view& command, const std::string_view& option);
	[[nodiscard]] static const std::vector<std::string_view>& get_existing_options_of(const std::string_view& command);
};
