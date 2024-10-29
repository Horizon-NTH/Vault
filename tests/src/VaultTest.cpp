#include "Vault.h"
#include <fstream>
#include <botan/allocator.h>
#include <botan/exceptn.h>
#include <gtest/gtest.h>

class VaultTest : public testing::Test
{
protected:
    std::filesystem::path m_temp_dir;

    void write_file(const std::string& name, const std::string& content) const
    {
        std::ofstream file((m_temp_dir / name).string());
        file << content;
        file.close();
    }

    [[nodiscard]] std::string read_file(const std::string& name) const
    {
        const std::ifstream file((m_temp_dir / name).string());
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    [[nodiscard]] bool exists(const std::string& name) const
    {
        return std::filesystem::exists(m_temp_dir / name);
    }

    void create_test_vault_directory() const
    {
        create_directory(m_temp_dir / "test_vault");
        create_directory(m_temp_dir / "test_vault/inner");
        create_directory(m_temp_dir / "test_vault/inner/inner");

        write_file("test_vault/inner/inner/file.txt", "Content of file.txt");
        write_file("test_vault/inner/inner/file2.txt", "Content of file2.txt");
        write_file("test_vault/inner/file.txt", "Content of inner/file.txt");
        write_file("test_vault/inner/file2.txt", "Content of inner/file2.txt");
        write_file("test_vault/file.txt", "Content of test_vault/file.txt");
        write_file("test_vault/file2.txt", "Content of test_vault/file2.txt");
    }

    static std::string get_test_vault_xml()
    {
        return "<vault name=\"test_vault\">\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\" />\n\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUyLnR4dA==\" />\n\t<directory name=\"inner\">\n\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlLnR4dA==\" />\n\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlMi50eHQ=\" />\n\t\t<directory name=\"inner\">\n\t\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBmaWxlLnR4dA==\" />\n\t\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBmaWxlMi50eHQ=\" />\n\t\t</directory>\n\t</directory>\n</vault>\n";
    }

    void assert_test_vault_existence() const
    {
        EXPECT_TRUE(exists("test_vault"));
        EXPECT_FALSE(exists("test_vault.vlt"));

        EXPECT_TRUE(exists("test_vault"));
        EXPECT_TRUE(exists("test_vault/inner"));
        EXPECT_TRUE(exists("test_vault/inner/inner"));
        EXPECT_TRUE(exists("test_vault/inner/inner/file.txt"));
        EXPECT_TRUE(exists("test_vault/inner/inner/file2.txt"));
        EXPECT_TRUE(exists("test_vault/inner/file.txt"));
        EXPECT_TRUE(exists("test_vault/inner/file2.txt"));
        EXPECT_TRUE(exists("test_vault/file.txt"));
        EXPECT_TRUE(exists("test_vault/file2.txt"));

        EXPECT_EQ(read_file("test_vault/inner/inner/file.txt"), "Content of file.txt");
        EXPECT_EQ(read_file("test_vault/inner/inner/file2.txt"), "Content of file2.txt");
        EXPECT_EQ(read_file("test_vault/inner/file.txt"), "Content of inner/file.txt");
        EXPECT_EQ(read_file("test_vault/inner/file2.txt"), "Content of inner/file2.txt");
        EXPECT_EQ(read_file("test_vault/file.txt"), "Content of test_vault/file.txt");
        EXPECT_EQ(read_file("test_vault/file2.txt"), "Content of test_vault/file2.txt");
    }

    void cleanup_test_environment() const
    {
        if (std::filesystem::exists(m_temp_dir))
        {
            remove_all(m_temp_dir);
        }
    }

    void SetUp() override
    {
        m_temp_dir = std::filesystem::temp_directory_path() / "vault_test_directory";
        cleanup_test_environment();
        create_directory(m_temp_dir);
    }

    void TearDown() override
    {
        cleanup_test_environment();
    }
};

TEST_F(VaultTest, Close)
{
    create_test_vault_directory();

    Vault vault(m_temp_dir / "test_vault");
    vault.close();

    EXPECT_FALSE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault.vlt"));
    EXPECT_TRUE(read_file("test_vault.vlt").starts_with("<vault"));
}

TEST_F(VaultTest, CloseEmptyVault)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);

    Vault vault(vaultPath);
    vault.close();

    EXPECT_FALSE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault.vlt"));
    const auto vaultContent = read_file("test_vault.vlt");
    EXPECT_TRUE(vaultContent.starts_with("<vault name=\"test_vault\""));
    EXPECT_TRUE(vaultContent.ends_with(" />\n"));
}

TEST_F(VaultTest, CloseWithCustomExtension)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);
    Vault vault(vaultPath);
    vault.close(std::nullopt, ".vault");

    EXPECT_FALSE(exists("test_vault"));
    EXPECT_FALSE(exists("test_vault.vlt"));
    EXPECT_TRUE(exists("test_vault.vault"));
}

TEST_F(VaultTest, CloseKeepEmptyExtension)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);
    Vault vault(vaultPath);
    vault.close(std::nullopt, "");

    EXPECT_FALSE(exists("test_vault.vlt"));
    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(std::filesystem::is_regular_file(m_temp_dir / "test_vault"));
}

TEST_F(VaultTest, Open)
{
    write_file("test_vault.vlt", get_test_vault_xml());

    Vault vault(m_temp_dir / "test_vault.vlt");
    vault.open();

    assert_test_vault_existence();
}

TEST_F(VaultTest, OpenEmptyVault)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");

    Vault vault(m_temp_dir / "test_vault.vlt");
    vault.open();

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_FALSE(exists("test_vault.vlt"));
}

TEST_F(VaultTest, OpenCloseNonExistent)
{
    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault.vlt");
        }, std::runtime_error);

    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault");
        }, std::runtime_error);
}

TEST_F(VaultTest, OpenClose)
{
    create_test_vault_directory();

    Vault vault(m_temp_dir / "test_vault");
    vault.close();

    EXPECT_TRUE(exists("test_vault.vlt"));
    EXPECT_FALSE(exists("test_vault"));
    EXPECT_TRUE(read_file("test_vault.vlt").starts_with("<vault"));

    vault.open();

    assert_test_vault_existence();
}

TEST_F(VaultTest, OpenCloseEmptyVault)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);

    Vault vault(vaultPath);
    vault.close();
    vault.open();

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_FALSE(exists("test_vault.vlt"));
    EXPECT_TRUE(std::filesystem::is_empty(vaultPath));
}

TEST_F(VaultTest, OpenCloseEmptyFile)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);
    write_file("test_vault/empty_file.txt", "");

    Vault vault(vaultPath);
    vault.close();
    vault.open();

    EXPECT_EQ(read_file("test_vault/empty_file.txt"), "");
}

TEST_F(VaultTest, OpenCloseBinaryFile)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);
    std::vector<uint8_t> binary_data = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    std::ofstream binary_file(m_temp_dir / "test_vault/binary_file.bin", std::ios::binary);
    binary_file.write(reinterpret_cast<const char*>(binary_data.data()), static_cast<std::streamsize>(binary_data.size()));
    binary_file.close();

    Vault vault(vaultPath);
    vault.close();
    vault.open();

    std::ifstream extracted_binary_file(m_temp_dir / "test_vault/binary_file.bin", std::ios::binary);
    std::vector<uint8_t> extracted_binary_data((std::istreambuf_iterator(extracted_binary_file)), std::istreambuf_iterator<char>());
    extracted_binary_file.close();

    EXPECT_EQ(binary_data, extracted_binary_data);
}

TEST_F(VaultTest, OpenCloseUnicodeFile)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);
    const std::string unicode_content = "你好，世界! Привет, мир! Hello, world!";
    write_file("test_vault/unicode_file.txt", unicode_content);

    Vault vault(vaultPath);
    vault.close();
    vault.open();

    EXPECT_EQ(read_file("test_vault/unicode_file.txt"), unicode_content);
}

TEST_F(VaultTest, OpenCloseFileWithSpecialCharacters)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);
    const std::string special_characters_content = "Special characters: !@#$%^&*()_+{}:\"<>?|";
    write_file("test_vault/special_characters_file.txt", special_characters_content);

    Vault vault(vaultPath);
    vault.close();
    vault.open();

    EXPECT_EQ(read_file("test_vault/special_characters_file.txt"), special_characters_content);
}

TEST_F(VaultTest, InvalidOpenVaultDirectory)
{
    create_directory(m_temp_dir / "test_vault");
    write_file("test_vault/file.txt", "Content of file.txt");

    Vault vault(m_temp_dir / "test_vault");

    EXPECT_THROW({vault.open();}, std::invalid_argument) << "The vault is already open";
}

TEST_F(VaultTest, InvalidCloseVaultFile)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n</vault>\n");

    Vault vault(m_temp_dir / "test_vault.vlt");

    EXPECT_THROW({vault.close();}, std::invalid_argument) << "The vault is already closed";
}

TEST_F(VaultTest, IncorrectlyEncodedDataField)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad"><file name="fileExemple1.txt" data="InvalidBase64Data@#%"/></vault>)");

    Vault vault(m_temp_dir / "test_vault_bad.vlt");

    EXPECT_THROW({vault.open();}, Botan::Invalid_Argument) << "The data field is not correctly encoded in Base64";
}

TEST_F(VaultTest, InvalidCloseWithSymbolicFile)
{
    try
    {
        create_directory(m_temp_dir / "test_vault");
        write_file("test_vault/file.txt", "Content of file.txt");
        create_symlink(m_temp_dir / "test_vault/file.txt", m_temp_dir / "test_vault/file.link");

        Vault vault(m_temp_dir / "test_vault");

        EXPECT_THROW({vault.close();}, std::runtime_error) << "Symbolic links are not supported";
    }
    catch (...) {}
}

TEST_F(VaultTest, InvalidCloseWithSymbolicDirectory)
{
    try
    {
        create_directories(m_temp_dir / "test_vault/inner");
        create_directory_symlink(m_temp_dir / "test_vault/inner", m_temp_dir / "test_vault/inner.link");

        Vault vault(m_temp_dir / "test_vault");

        EXPECT_THROW({vault.close();}, std::runtime_error) << "Symbolic links are not supported";
    }
    catch (...) {}
}

TEST_F(VaultTest, CloseWithSpecifiedDestination)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);
    create_directory(m_temp_dir / "test_destination");

    Vault vault(vaultPath);
    vault.close(m_temp_dir / "test_destination");

    EXPECT_TRUE(exists("test_destination/test_vault.vlt"));
}

TEST_F(VaultTest, OpenWithSpecifiedDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");
    create_directory(m_temp_dir / "test_destination");

    Vault vault(m_temp_dir / "test_vault.vlt");
    vault.open(m_temp_dir / "test_destination");

    EXPECT_TRUE(exists("test_destination/test_vault"));
}

TEST_F(VaultTest, CloseWithEmptyDestination)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);

    Vault vault(vaultPath);
    vault.close(m_temp_dir / "");

    EXPECT_TRUE(exists("test_vault.vlt"));
}

TEST_F(VaultTest, OpenWithEmptyDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");

    Vault vault(m_temp_dir / "test_vault.vlt");
    vault.open(m_temp_dir / "");

    EXPECT_TRUE(exists("test_vault"));
}

TEST_F(VaultTest, InvalidCloseWithNonExistentDestination)
{
    create_directory(m_temp_dir / "test_vault");

    Vault vault(m_temp_dir / "test_vault");

    EXPECT_THROW({vault.close(m_temp_dir / "test_destination");}, std::runtime_error);
}

TEST_F(VaultTest, InvalidCloseWithDestinationInsideTheClosedVault)
{
    create_directory(m_temp_dir / "test_vault");

    Vault vault(m_temp_dir / "test_vault");

    EXPECT_THROW({vault.close(m_temp_dir / "test_vault");}, std::invalid_argument);
}

TEST_F(VaultTest, InvalidOpenWithNonExistentDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");

    Vault vault(m_temp_dir / "test_vault.vlt");

    EXPECT_THROW({vault.open(m_temp_dir / "test_destination");}, std::runtime_error);
}

TEST_F(VaultTest, CloseWithCompression)
{
    create_test_vault_directory();

    Vault vault(m_temp_dir / "test_vault");
    vault.close(std::nullopt, std::nullopt, true);

    EXPECT_FALSE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault.vlt"));
    EXPECT_TRUE(read_file("test_vault.vlt").starts_with("<compressed"));
}

TEST_F(VaultTest, OpenWithCompression)
{
    create_test_vault_directory();

    Vault vault(m_temp_dir / "test_vault");
    vault.close(std::nullopt, std::nullopt, true);
    vault.open();

    assert_test_vault_existence();
}

#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
TEST_F(VaultTest, CloseOpenKeepLastWriteTime)
{
    create_directory(m_temp_dir / "test_vault");
    write_file("test_vault/file.txt", "Content of file.txt");
    const auto dir_last_write_time = last_write_time(m_temp_dir / "test_vault");
    const auto file_last_write_time = last_write_time(m_temp_dir / "test_vault/file.txt");

    Vault vault(m_temp_dir / "test_vault");
    vault.close();
    vault.open();

    EXPECT_EQ(last_write_time(m_temp_dir / "test_vault"), dir_last_write_time);
    EXPECT_EQ(last_write_time(m_temp_dir / "test_vault/file.txt"), file_last_write_time);
}
#endif

#if !defined(_WIN32)
TEST_F(VaultTest, CloseOpenKeepPermissions)
{
    const auto vaultPath = m_temp_dir / "test_vault";
    create_directory(vaultPath);
    write_file("test_vault/file.txt", "Content of file.txt");

    std::filesystem::permissions(vaultPath, std::filesystem::perms::none | std::filesystem::perms::owner_all);
    std::filesystem::permissions(vaultPath / "file.txt", std::filesystem::perms::owner_all | std::filesystem::perms::others_exec | std::filesystem::perms::group_read);

    Vault vault(vaultPath);
    vault.close();
    vault.open();

    EXPECT_EQ(std::filesystem::status(vaultPath).permissions(), (std::filesystem::perms::none | std::filesystem::perms::owner_all));
    EXPECT_EQ(std::filesystem::status(vaultPath / "file.txt").permissions(), (std::filesystem::perms::owner_all | std::filesystem::perms::others_exec | std::filesystem::perms::group_read));
}
#endif
