#include "Vault.h"
#include <fstream>
#include <gtest/gtest.h>

void write_file(const std::string& path, const std::string& content)
{
    std::ofstream file(path);
    file << content;
    file.close();
}

std::string read_file(const std::string& path)
{
    const std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void create_test_vault_directory()
{
    std::filesystem::create_directory("test_vault");
    std::filesystem::create_directory("test_vault/inner");
    std::filesystem::create_directory("test_vault/inner/inner");

    write_file("test_vault/inner/inner/file.txt", "Content of file.txt");
    write_file("test_vault/inner/inner/file2.txt", "Content of file2.txt");
    write_file("test_vault/inner/file.txt", "Content of inner/file.txt");
    write_file("test_vault/inner/file2.txt", "Content of inner/file2.txt");
    write_file("test_vault/file.txt", "Content of test_vault/file.txt");
    write_file("test_vault/file2.txt", "Content of test_vault/file2.txt");
}

void cleanup_test_environment()
{
    std::filesystem::remove_all("test_vault");
    std::filesystem::remove_all("test_destination");
    std::filesystem::remove_all("test_vault_bad");
    std::filesystem::remove("test_vault.vlt");
    std::filesystem::remove("test_vault_bad.vlt");
}

class VaultTest : public testing::Test
{
protected:
    void SetUp() override
    {
        cleanup_test_environment();
    }

    void TearDown() override
    {
        cleanup_test_environment();
    }
};

TEST_F(VaultTest, Close)
{
    create_test_vault_directory();

    Vault::close("test_vault");

    EXPECT_FALSE(std::filesystem::exists("test_vault"));
    EXPECT_TRUE(std::filesystem::exists("test_vault.vlt"));
    EXPECT_EQ(read_file("test_vault.vlt"),
        std::string("<vault name=\"test_vault\">\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUyLnR4dA==\"/>\n\t<directory name=\"inner\">\n\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlLnR4dA==\"/>\n\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlMi50eHQ=\"/>\n\t\t<directory name=\"inner\">\n\t\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBmaWxlLnR4dA==\"/>\n\t\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBmaWxlMi50eHQ=\"/>\n\t\t</directory>\n\t</directory>\n</vault>\n"));
}

TEST_F(VaultTest, CloseEmptyVault)
{
    std::filesystem::create_directory("test_vault");

    Vault::close("test_vault");

    EXPECT_FALSE(std::filesystem::exists("test_vault"));
    EXPECT_TRUE(std::filesystem::exists("test_vault.vlt"));
    EXPECT_EQ(read_file("test_vault.vlt"), "<vault name=\"test_vault\">\n</vault>\n");
}

TEST_F(VaultTest, Open)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUyLnR4dA==\"/>\n\t<directory name=\"inner\">\n\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlLnR4dA==\"/>\n\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlMi50eHQ=\"/>\n\t\t<directory name=\"inner\">\n\t\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBmaWxlLnR4dA==\"/>\n\t\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBmaWxlMi50eHQ=\"/>\n\t\t</directory>\n\t</directory>\n</vault>\n");
    Vault::open("test_vault.vlt");

    EXPECT_TRUE(std::filesystem::exists("test_vault"));
    EXPECT_FALSE(std::filesystem::exists("test_vault.vlt"));

    EXPECT_TRUE(std::filesystem::exists("test_vault"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/inner"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/inner/file.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/inner/file2.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/file.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/file2.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/file.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/file2.txt"));

    EXPECT_EQ(read_file("test_vault/inner/inner/file.txt"), read_file("test_vault/inner/inner/file.txt"));
    EXPECT_EQ(read_file("test_vault/inner/inner/file2.txt"), read_file("test_vault/inner/inner/file2.txt"));
    EXPECT_EQ(read_file("test_vault/inner/file.txt"), read_file("test_vault/inner/file.txt"));
    EXPECT_EQ(read_file("test_vault/inner/file2.txt"), read_file("test_vault/inner/file2.txt"));
    EXPECT_EQ(read_file("test_vault/file.txt"), read_file("test_vault/file.txt"));
    EXPECT_EQ(read_file("test_vault/file2.txt"), read_file("test_vault/file2.txt"));
}

TEST_F(VaultTest, OpenEmptyVault)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");

    Vault::open("test_vault.vlt");

    EXPECT_TRUE(std::filesystem::exists("test_vault"));
    EXPECT_FALSE(std::filesystem::exists("test_vault.vlt"));
    EXPECT_TRUE(std::filesystem::is_empty("test_vault"));
}

TEST_F(VaultTest, OpenCloseNonExistent)
{
    EXPECT_THROW(Vault::open("non_existent.vlt"), std::runtime_error);
    EXPECT_THROW(Vault::close("non_existent"), std::runtime_error);
}

TEST_F(VaultTest, OpenClose)
{
    create_test_vault_directory();

    Vault::close("test_vault");

    EXPECT_TRUE(std::filesystem::exists("test_vault.vlt"));
    EXPECT_FALSE(std::filesystem::exists("test_vault"));

    Vault::open("test_vault.vlt");

    EXPECT_FALSE(std::filesystem::exists("test_vault.vlt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault"));
}

TEST_F(VaultTest, OpenCloseKeepFileStructure)
{
    create_test_vault_directory();

    Vault::close("test_vault");
    Vault::open("test_vault.vlt");

    EXPECT_TRUE(std::filesystem::exists("test_vault"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/inner"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/inner/file.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/inner/file2.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/file.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/inner/file2.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/file.txt"));
    EXPECT_TRUE(std::filesystem::exists("test_vault/file2.txt"));
}

TEST_F(VaultTest, OpenCloseKeepFileContent)
{
    create_test_vault_directory();

    Vault::close("test_vault");
    Vault::open("test_vault.vlt");

    EXPECT_EQ(read_file("test_vault/inner/inner/file.txt"), read_file("test_vault/inner/inner/file.txt"));
    EXPECT_EQ(read_file("test_vault/inner/inner/file2.txt"), read_file("test_vault/inner/inner/file2.txt"));
    EXPECT_EQ(read_file("test_vault/inner/file.txt"), read_file("test_vault/inner/file.txt"));
    EXPECT_EQ(read_file("test_vault/inner/file2.txt"), read_file("test_vault/inner/file2.txt"));
    EXPECT_EQ(read_file("test_vault/file.txt"), read_file("test_vault/file.txt"));
    EXPECT_EQ(read_file("test_vault/file2.txt"), read_file("test_vault/file2.txt"));
}

TEST_F(VaultTest, OpenCloseBinaryFile)
{
    std::filesystem::create_directory("test_vault");
    std::vector<uint8_t> binary_data = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    std::ofstream binary_file("test_vault/binary_file.bin", std::ios::binary);
    binary_file.write(reinterpret_cast<const char*>(binary_data.data()), static_cast<std::streamsize>(binary_data.size()));
    binary_file.close();

    Vault::close("test_vault");
    Vault::open("test_vault.vlt");

    std::ifstream extracted_binary_file("test_vault/binary_file.bin", std::ios::binary);
    std::vector<uint8_t> extracted_binary_data((std::istreambuf_iterator(extracted_binary_file)), std::istreambuf_iterator<char>());
    extracted_binary_file.close();

    EXPECT_EQ(binary_data, extracted_binary_data);
}

TEST_F(VaultTest, OpenCloseEmptyVault)
{
    std::filesystem::create_directory("test_vault");

    Vault::close("test_vault");
    Vault::open("test_vault.vlt");

    EXPECT_TRUE(std::filesystem::is_empty("test_vault"));
}

TEST_F(VaultTest, OpenCloseEmptyFile)
{
    std::filesystem::create_directory("test_vault");
    write_file("test_vault/empty_file.txt", "");

    Vault::close("test_vault");
    Vault::open("test_vault.vlt");

    EXPECT_EQ(read_file("test_vault/empty_file.txt"), "");
}

TEST_F(VaultTest, OpenCloseUnicodeFile)
{
    std::filesystem::create_directory("test_vault");
    const std::string unicode_content = "你好，世界! Привет, мир! Hello, world!";
    write_file("test_vault/unicode_file.txt", unicode_content);

    Vault::close("test_vault");
    Vault::open("test_vault.vlt");

    EXPECT_EQ(read_file("test_vault/unicode_file.txt"), unicode_content);
}

TEST_F(VaultTest, OpenCloseFileWithSpecialCharacters)
{
    std::filesystem::create_directory("test_vault");
    const std::string special_characters_content = "Special characters: !@#$%^&*()_+{}:\"<>?|";
    write_file("test_vault/special_characters_file.txt", special_characters_content);

    Vault::close("test_vault");
    Vault::open("test_vault.vlt");

    EXPECT_EQ(read_file("test_vault/special_characters_file.txt"), special_characters_content);
}

TEST_F(VaultTest, InvalidOpenVaultDirectory)
{
    std::filesystem::create_directory("test_vault");
    write_file("test_vault/file.txt", "Content of file.txt");

    EXPECT_THROW(Vault::open("test_vault"), std::invalid_argument);
}

TEST_F(VaultTest, InvalidCloseVaultFile)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n</vault>\n");

    EXPECT_THROW(Vault::close("test_vault.vlt"), std::invalid_argument);
}

TEST_F(VaultTest, OpenCloseWithCustomExtension)
{
    std::filesystem::create_directory("test_vault");

    Vault::close("test_vault", std::nullopt, ".vault");

    EXPECT_TRUE(std::filesystem::exists("test_vault.vault"));
    EXPECT_FALSE(std::filesystem::exists("test_vault"));

    Vault::open("test_vault.vault");

    EXPECT_FALSE(std::filesystem::exists("test_vault.vault"));
    EXPECT_TRUE(std::filesystem::exists("test_vault"));
}

TEST_F(VaultTest, OpenCloseWithoutExtension)
{
    std::filesystem::create_directory("test_vault");

    Vault::close("test_vault", std::nullopt, "");

    EXPECT_TRUE(std::filesystem::exists("test_vault"));
    EXPECT_TRUE(std::filesystem::is_regular_file("test_vault"));

    Vault::open("test_vault");

    EXPECT_TRUE(std::filesystem::exists("test_vault"));
    EXPECT_TRUE(std::filesystem::is_directory("test_vault"));
}

TEST_F(VaultTest, InvalidCloseWithSymbolicFile)
{
    try
    {
        std::filesystem::create_directory("test_vault");
        write_file("test_vault/file.txt", "Content of file.txt");
        std::filesystem::create_symlink("test_vault/file.txt", "test_vault/file.link");

        EXPECT_THROW(Vault::close("test_vault"), std::runtime_error);
    }
    catch (...) {}
}

TEST_F(VaultTest, InvalidCloseWithSymbolicDirectory)
{
    try
    {
        std::filesystem::create_directories("test_vault/inner");
        std::filesystem::create_directory_symlink("test_vault/inner", "test_vault/inner.link");

        EXPECT_THROW(Vault::close("test_vault"), std::runtime_error);
    }
    catch (...) {}
}

TEST_F(VaultTest, MissingClosingVaultTag)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="leGFtcGxlI"/>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, ImproperlyClosedVaultTag)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="leGFtcGxlI"/><vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, MalformedFileTag)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file name="fileExemple1.txt">leGFtcGxlI</file></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, MalformedDirectoryTag)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><directory name="folderExemple1"/></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, ImproperlyClosedDirectoryTag)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="leGFtcGxlI"/><directory name="folderExemple1"><file name="fileExemple2.txt" data="GUgaG9vay"/></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, TooMuchClosingDirectoryTags)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><directory name="folderExemple1"><file name="fileExemple1.txt" data="GUgaG9vay"/></directory></directory></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, IncorrectlyEncodedDataField)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="InvalidBase64Data@#%"/></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::invalid_argument);
}

TEST_F(VaultTest, MissingFileAttributes)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file data="GUgaG9vay"/></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, UnknowFileAttribute)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="GUgaG9vay" date="2021-09-01"/></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, FileDataTagNotNecessary)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file name="fileExemple1.txt"/></vault>)");

    EXPECT_NO_THROW(Vault::open("test_vault_bad.vlt"));
}

TEST_F(VaultTest, MissingDirectoryAttributes)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><directory><file name="fileExemple1.txt" data="GUgaG9vay"/></directory></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, UnknowDirectoryAttribute)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><directory date="2021-08-01" name="folderExemple1"><file name="fileExemple1.txt" data="GUgaG9vay"/></directory></vault>)");

    EXPECT_THROW(Vault::open("test_vault_bad.vlt"), std::runtime_error);
}

TEST_F(VaultTest, CloseWithSpecifiedDestination)
{
    std::filesystem::create_directory("test_vault");
    std::filesystem::create_directory("test_destination");

    Vault::close("test_vault", "test_destination");

    EXPECT_TRUE(std::filesystem::exists("test_destination/test_vault.vlt"));
}

TEST_F(VaultTest, OpenWithSpecifiedDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");
    std::filesystem::create_directory("test_destination");

    Vault::open("test_vault.vlt", "test_destination");

    EXPECT_TRUE(std::filesystem::exists("test_destination/test_vault"));
}

TEST_F(VaultTest, CloseWithEmptyDestination)
{
    std::filesystem::create_directory("test_vault");

    Vault::close("test_vault", "");

    EXPECT_TRUE(std::filesystem::exists("test_vault.vlt"));
}

TEST_F(VaultTest, OpenWithEmptyDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");

    Vault::open("test_vault.vlt", "");

    EXPECT_TRUE(std::filesystem::exists("test_vault"));
}

TEST_F(VaultTest, InvalidCloseWithNonExistentDestination)
{
    std::filesystem::create_directory("test_vault");

    EXPECT_THROW(Vault::close("test_vault", "test_destination"), std::runtime_error);
}

TEST_F(VaultTest, InvalidOpenWithNonExistentDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");

    EXPECT_THROW(Vault::open("test_vault.vlt", "test_destination"), std::runtime_error);
}
