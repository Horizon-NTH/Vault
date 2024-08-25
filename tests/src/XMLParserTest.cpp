#include "XMLParser.h"
#include <gtest/gtest.h>

TEST(XMLParserTest, MissingClosingVaultTag)
{
    const auto xml = R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="leGFtcGxlI"/>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, ImproperlyClosedVaultTag)
{
    const auto xml = R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="leGFtcGxlI"/><vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, MalformedFileTag)
{
    const auto xml = R"(<vault name="test_vault_bad"><file name="fileExemple1.txt">leGFtcGxlI</file></vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, MalformedDirectoryTag)
{
    const auto xml = R"(<vault name="test_vault_bad"><directory name="folderExemple1"/></vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, ImproperlyClosedDirectoryTag)
{
    const auto xml = R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="leGFtcGxlI"/><directory name="folderExemple1"><file name="fileExemple2.txt" data="GUgaG9vay"/></vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, TooMuchClosingDirectoryTags)
{
    const auto xml = R"(<vault name="test_vault_bad"><directory name="folderExemple1"><file name="fileExemple1.txt" data="GUgaG9vay"/></directory></directory></vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, MissingFileAttributes)
{
    const auto xml = R"(<vault name="test_vault_bad"><file data="GUgaG9vay"/></vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, UnknowFileAttribute)
{
    const auto xml = R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="GUgaG9vay" date="2021-09-01"/></vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, MissingDirectoryAttributes)
{
    const auto xml = R"(<vault name="test_vault_bad"><directory><file name="fileExemple1.txt" data="GUgaG9vay"/></directory></vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}

TEST(XMLParserTest, UnknowDirectoryAttribute)
{
    const auto xml = R"(<vault name="test_vault_bad"><directory date="2021-08-01" name="folderExemple1"><file name="fileExemple1.txt" data="GUgaG9vay"/></directory></vault>)";

    EXPECT_THROW(XMLParser::parse(xml), std::runtime_error);
}
