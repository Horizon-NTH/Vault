#include <gtest/gtest.h>
#include "ArgumentsParser.h"

class ArgumentsParserTest : public testing::Test
{
protected:
	std::unique_ptr<ArgumentsParser> parser;

	void init(const std::span<const char*> args)
	{
		parser = std::make_unique<ArgumentsParser>(args);
	}
};

TEST_F(ArgumentsParserTest, NoArgumentsBehaveAsHelp)
{
	const char* argv[] = {"program"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "help");
	EXPECT_TRUE(parser->options().empty());
}

TEST_F(ArgumentsParserTest, ShortHelpArgument)
{
	const char* argv[] = {"program", "-h"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "help");
	EXPECT_TRUE(parser->options().empty());
}

TEST_F(ArgumentsParserTest, LongHelpArgument)
{
	const char* argv[] = {"program", "--help"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "help");
	EXPECT_TRUE(parser->options().empty());
}

TEST_F(ArgumentsParserTest, ShortVersionArgument)
{
	const char* argv[] = {"program", "-v"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "version");
	EXPECT_TRUE(parser->options().empty());
}

TEST_F(ArgumentsParserTest, LongVersionArgument)
{
	const char* argv[] = {"program", "--version"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "version");
	EXPECT_TRUE(parser->options().empty());
}

TEST_F(ArgumentsParserTest, OtherArgumentsAfterHelpIgnored)
{
	const char* argv[] = {"program", "-h", "other"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "help");
	EXPECT_TRUE(parser->options().empty());
}

TEST_F(ArgumentsParserTest, InvalidCommand)
{
	const char* argv[] = {"program", "invalid"};
	EXPECT_THROW(init(std::span(argv)), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, InvalidVaultShortOption)
{
	const char* argv[] = {"program", "-u"};
	EXPECT_THROW(init(std::span(argv)), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, InvalidVaultLongOption)
{
	const char* argv[] = {"program", "--unknown"};
	EXPECT_THROW(init(std::span(argv)), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, ValidCommandWithInvalidOptionOfOtherCommand)
{
	const char* argv[] = {"program", "open", "-e", ".ext"};
	EXPECT_THROW(init(std::span(argv)), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, OpenCommandWithoutExplicitFlagsAndNoDestination)
{
	const char* argv[] = {"program", "open", "/path/to/vault.vlt"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "open");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 1);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
}

TEST_F(ArgumentsParserTest, OpenCommandWithoutExplicitFlagsAndDestination)
{
	const char* argv[] = {"program", "open", "/path/to/vault.vlt", "/path/to/destination"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "open");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 2);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
}

TEST_F(ArgumentsParserTest, OpenCommandWithExplicitShortFlagsAndNoDestination)
{
	const char* argv[] = {"program", "open", "-v", "/path/to/vault.vlt"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "open");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 1);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
}

TEST_F(ArgumentsParserTest, OpenCommandWithExplicitShortFlagsAndNoVault)
{
	const char* argv[] = {"program", "open", "-d", "/path/to/destination"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "open");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 1);
	EXPECT_FALSE(options.contains("vault"));
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
}

TEST_F(ArgumentsParserTest, OpenCommandWithExplicitShortFlagsAndDestination)
{
	const char* argv[] = {"program", "open", "-v", "/path/to/vault.vlt", "-d", "/path/to/destination"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "open");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 2);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
}

TEST_F(ArgumentsParserTest, OpenCommandWithExplicitShortAndLongFlagsAndDestination)
{
	const char* argv1[] = {"program", "open", "--vault", "/path/to/vault.vlt", "-d", "/path/to/destination"};
	EXPECT_NO_THROW(init(std::span(argv1)));
	EXPECT_EQ(parser->command(), "open");
	ArgumentsParser::optionsList options = parser->options();
	EXPECT_EQ(options.size(), 2);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");

	const char* argv2[] = {"program", "open", "-v", "/path/to/vault.vlt", "--destination", "/path/to/destination"};
	EXPECT_NO_THROW(init(std::span(argv2)));
	EXPECT_EQ(parser->command(), "open");
	options = parser->options();
	EXPECT_EQ(options.size(), 2);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
}

TEST_F(ArgumentsParserTest, OpenCommandWithExplicitAndPositionalFlagsAndDestination)
{
	const char* argv1[] = {"program", "open", "--vault", "/path/to/vault.vlt", "/path/to/destination"};
	EXPECT_NO_THROW(init(std::span(argv1)));
	EXPECT_EQ(parser->command(), "open");
	ArgumentsParser::optionsList options = parser->options();
	EXPECT_EQ(options.size(), 2);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");

	const char* argv2[] = {"program", "open", "/path/to/vault.vlt", "--destination", "/path/to/destination"};
	EXPECT_NO_THROW(init(std::span(argv2)));
	EXPECT_EQ(parser->command(), "open");
	options = parser->options();
	EXPECT_EQ(options.size(), 2);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
}

TEST_F(ArgumentsParserTest, OpenCommandWithExplicitAndPositionalInvertedFlagsAndDestination)
{
	const char* argv[] = {"program", "open", "--destination", "/path/to/destination", "/path/to/vault.vlt"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "open");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 2);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
}

TEST_F(ArgumentsParserTest, OpenCommandWithUnkownOption)
{
	{
		const char* argv[] = {"program", "open", "--unknown", "/path/to/vault.vlt"};
		EXPECT_THROW(init(std::span(argv)), std::invalid_argument);
	}
	{
		const char* argv[] = {"program", "open", "-u", "/path/to/vault.vlt"};
		EXPECT_THROW(init(std::span(argv)), std::invalid_argument);
	}
}

TEST_F(ArgumentsParserTest, OpenCommandWithOptionWithIntegratedValue)
{
	const char* argv[] = {"program", "open", "-v/path/to/vault.vlt", "--destination=/path/to/destination"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "open");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 2);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
}

TEST_F(ArgumentsParserTest, OpenCommandWithOptionWithInvalidIntegratedValue)
{
	const char* argv[] = {"program", "open", "--value/path/to/vault.vlt"};
	EXPECT_THROW(init(std::span(argv)), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, CloseCommandWithoutExplicitFlagsAndNoDestinationOrExtension)
{
	const char* argv[] = {"program", "close", "/path/to/vault.vlt"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "close");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 1);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
}

TEST_F(ArgumentsParserTest, CloseCommandWithoutExplicitFlagsAndDestinationAndExtension)
{
	const char* argv[] = {"program", "close", "/path/to/vault.vlt", "/path/to/destination", ".ext"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "close");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 3);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
	EXPECT_TRUE(options.contains("extension"));
	EXPECT_TRUE(options.at("extension").has_value());
	EXPECT_EQ(options.at("extension"), ".ext");
}

TEST_F(ArgumentsParserTest, CloseCommandWithExplicitShortFlagsAndDestinationAndExtension)
{
	const char* argv[] = {"program", "close", "-v", "/path/to/vault.vlt", "-d", "/path/to/destination", "-e", ".ext"};
	EXPECT_NO_THROW(init(std::span(argv)));
	EXPECT_EQ(parser->command(), "close");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 3);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
	EXPECT_TRUE(options.contains("extension"));
	EXPECT_TRUE(options.at("extension").has_value());
	EXPECT_EQ(options.at("extension"), ".ext");
}

TEST_F(ArgumentsParserTest, CloseCommandWithExplicitShortAndLongFlagsAndPositionalAndDestinationAndExtension)
{
	const char* argv1[] = {"program", "close", "--vault", "/path/to/vault.vlt", "/path/to/destination", "-e", ".ext"};
	EXPECT_NO_THROW(init(std::span(argv1)));
	EXPECT_EQ(parser->command(), "close");
	const auto& options = parser->options();
	EXPECT_EQ(options.size(), 3);
	EXPECT_TRUE(options.contains("vault"));
	EXPECT_TRUE(options.at("vault").has_value());
	EXPECT_EQ(options.at("vault"), "/path/to/vault.vlt");
	EXPECT_TRUE(options.contains("destination"));
	EXPECT_TRUE(options.at("destination").has_value());
	EXPECT_EQ(options.at("destination"), "/path/to/destination");
	EXPECT_TRUE(options.contains("extension"));
	EXPECT_TRUE(options.at("extension").has_value());
	EXPECT_EQ(options.at("extension"), ".ext");
}

TEST_F(ArgumentsParserTest, InvlaidCloseCommandWithDuplicateOption)
{
	const char* argv1[] = {"program", "close", "--vault", "/path/to/vault.vlt", "-v", "/path/to/destination", "-e", ".ext"};
	EXPECT_THROW(init(std::span(argv1)), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, InvlaidCloseCommandWithExplicitFlagsAfterPositional)
{
	const char* argv1[] = {"program", "close", "--vault", "/path/to/vault.vlt", "/path/to/destination", "-d", ".ext"};
	EXPECT_THROW(init(std::span(argv1)), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, CommandWithEmptyLastFlag)
{
	const char* argv1[] = {"program", "close", "--vault"};
	EXPECT_NO_THROW(init(std::span(argv1)));
}
