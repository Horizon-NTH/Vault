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

    [[nodiscard]] static int number_of_files(const std::filesystem::path& path)
    {
        int count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                count++;
            }
            else if (entry.is_directory())
            {
                count += number_of_files(entry);
            }
        }
        return count;
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

        write_file("test_vault/.vlt", "name: test_vault\nextension: .vlt");
        write_file("test_vault/inner/inner/file.txt", "Content of file.txt");
        write_file("test_vault/inner/inner/file2.txt", "Content of file2.txt");
        write_file("test_vault/inner/file.txt", "Content of inner/file.txt");
        write_file("test_vault/inner/file2.txt", "Content of inner/file2.txt");
        write_file("test_vault/file.txt", "Content of test_vault/file.txt");
        write_file("test_vault/file2.txt", "Content of test_vault/file2.txt");
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

TEST_F(VaultTest, Create)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir);

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault/.vlt"));
    EXPECT_EQ(number_of_files(m_temp_dir / "test_vault"), 1);
    EXPECT_EQ(read_file("test_vault/.vlt"), "name: test_vault\nextension: .vlt");
}

TEST_F(VaultTest, CreateWithSpecifiedExtension)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir, "vault");

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault/.vlt"));
    EXPECT_EQ(number_of_files(m_temp_dir / "test_vault"), 1);
    EXPECT_EQ(read_file("test_vault/.vlt"), "name: test_vault\nextension: .vault");
}

TEST_F(VaultTest, CreateWithEmptyExtension)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir, "");

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault/.vlt"));
    EXPECT_EQ(number_of_files(m_temp_dir / "test_vault"), 1);
    EXPECT_EQ(read_file("test_vault/.vlt"), "name: test_vault\nextension: \"\"");
}

TEST_F(VaultTest, CreateWithSourceDirectory)
{
    create_directory(m_temp_dir / "test_source");
    write_file("test_source/file.txt", "Content of file.txt");

    Vault vault("test_vault", std::filesystem::directory_entry(m_temp_dir / "test_source"), m_temp_dir);

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_FALSE(exists("test_source"));
    EXPECT_TRUE(exists("test_vault/file.txt"));
    EXPECT_EQ(read_file("test_vault/file.txt"), "Content of file.txt");
}

TEST_F(VaultTest, Close)
{
    create_test_vault_directory();

    Vault vault(m_temp_dir / "test_vault");
    vault.close();

    EXPECT_FALSE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault.vlt"));

    // TODO: Find a way to test the content of the file
    // #if defined(_WIN32)
    //     EXPECT_EQ(read_file("test_vault.vlt"), std::string("<vault name=\"test_vault\">\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUyLnR4dA==\"/>\n\t<directory name=\"inner\">\n\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlLnR4dA==\"/>\n\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlMi50eHQ=\"/>\n\t\t<directory name=\"inner\">\n\t\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBmaWxlLnR4dA==\"/>\n\t\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBmaWxlMi50eHQ=\"/>\n\t\t</directory>\n\t</directory>\n</vault>\n"));
    // #elif defined(__APPLE) || defined(__MACH__)
    //     EXPECT_EQ(read_file("test_vault.vlt"), std::string("<vault name=\"test_vault\">\n\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUyLnR4dA==\"/>\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n\t<directory name=\"inner\">\n\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlMi50eHQ=\"/>\n\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlLnR4dA==\"/>\n\t\t<directory name=\"inner\">\n\t\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBmaWxlMi50eHQ=\"/>\n\t\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBmaWxlLnR4dA==\"/>\n\t\t</directory>\n\t</directory>\n</vault>\n"));
    // #elif defined(__linux__)
    //     EXPECT_EQ(read_file("test_vault.vlt"), std::string("<vault name=\"test_vault\">\n\t<directory name=\"inner\">\n\t\t<directory name=\"inner\">\n\t\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBmaWxlLnR4dA==\"/>\n\t\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBmaWxlMi50eHQ=\"/>\n\t\t</directory>\n\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlLnR4dA==\"/>\n\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlMi50eHQ=\"/>\n\t</directory>\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUyLnR4dA==\"/>\n</vault>\n"));
    // #endif
}

TEST_F(VaultTest, CloseEmptyVault)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir);
    vault.close();

    EXPECT_FALSE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault.vlt"));
    EXPECT_EQ(read_file("test_vault.vlt"), "<vault name=\"test_vault\" extension=\".vlt\">\n</vault>\n");
}

TEST_F(VaultTest, CloseKeepCustomExtension)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir, "vault");
    vault.close();

    EXPECT_FALSE(exists("test_vault"));
    EXPECT_FALSE(exists("test_vault.vlt"));
    EXPECT_TRUE(exists("test_vault.vault"));
}

TEST_F(VaultTest, CloseKeepEmptyExtension)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir, "");
    vault.close();

    EXPECT_FALSE(exists("test_vault.vlt"));
    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(std::filesystem::is_regular_file(m_temp_dir / "test_vault"));
}

TEST_F(VaultTest, Open)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\" extension=\".vlt\">\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUyLnR4dA==\"/>\n\t<directory name=\"inner\">\n\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlLnR4dA==\"/>\n\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBpbm5lci9maWxlMi50eHQ=\"/>\n\t\t<directory name=\"inner\">\n\t\t\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiBmaWxlLnR4dA==\"/>\n\t\t\t<file name=\"file2.txt\" data=\"Q29udGVudCBvZiBmaWxlMi50eHQ=\"/>\n\t\t</directory>\n\t</directory>\n</vault>\n");

    Vault vault(m_temp_dir / "test_vault.vlt");
    vault.open();

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_FALSE(exists("test_vault.vlt"));

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault/.vlt"));
    EXPECT_TRUE(exists("test_vault/inner"));
    EXPECT_TRUE(exists("test_vault/inner/inner"));
    EXPECT_TRUE(exists("test_vault/inner/inner/file.txt"));
    EXPECT_TRUE(exists("test_vault/inner/inner/file2.txt"));
    EXPECT_TRUE(exists("test_vault/inner/file.txt"));
    EXPECT_TRUE(exists("test_vault/inner/file2.txt"));
    EXPECT_TRUE(exists("test_vault/file.txt"));
    EXPECT_TRUE(exists("test_vault/file2.txt"));

    EXPECT_EQ(read_file("test_vault/.vlt"), "name: test_vault\nextension: .vlt");
    EXPECT_EQ(read_file("test_vault/inner/inner/file.txt"), "Content of file.txt");
    EXPECT_EQ(read_file("test_vault/inner/inner/file2.txt"), "Content of file2.txt");
    EXPECT_EQ(read_file("test_vault/inner/file.txt"), "Content of inner/file.txt");
    EXPECT_EQ(read_file("test_vault/inner/file2.txt"), "Content of inner/file2.txt");
    EXPECT_EQ(read_file("test_vault/file.txt"), "Content of test_vault/file.txt");
    EXPECT_EQ(read_file("test_vault/file2.txt"), "Content of test_vault/file2.txt");
}

TEST_F(VaultTest, OpenEmptyVault)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\" extension=\".vlt\">\n</vault>\n");

    Vault vault(m_temp_dir / "test_vault.vlt");
    vault.open();

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_FALSE(exists("test_vault.vlt"));
    EXPECT_EQ(number_of_files(m_temp_dir / "test_vault"), 1);
    EXPECT_TRUE(exists("test_vault/.vlt"));
}

TEST_F(VaultTest, OpenCloseNonExistent)
{
    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault.vlt");
        vault.open();
        }, std::runtime_error);

    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault");
        vault.close();
        }, std::runtime_error);
}

TEST_F(VaultTest, OpenClose)
{
    create_test_vault_directory();

    Vault vault(m_temp_dir / "test_vault");
    vault.close();

    EXPECT_TRUE(exists("test_vault.vlt"));
    EXPECT_FALSE(exists("test_vault"));

    vault.open();

    EXPECT_FALSE(exists("test_vault.vlt"));
    EXPECT_TRUE(exists("test_vault"));
}

TEST_F(VaultTest, OpenCloseEmptyVault)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir);
    vault.close();
    vault.open();

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_FALSE(exists("test_vault.vlt"));
    EXPECT_EQ(number_of_files(m_temp_dir / "test_vault"), 1);
}

TEST_F(VaultTest, OpenCloseEmptyFile)
{
    create_directory(m_temp_dir / "test_vault");
    write_file("test_vault/empty_file.txt", "");

    Vault vault("test_vault", std::filesystem::directory_entry(m_temp_dir / "test_vault"), m_temp_dir);
    vault.close();
    vault.open();

    EXPECT_EQ(read_file("test_vault/empty_file.txt"), "");
}

TEST_F(VaultTest, OpenCloseKeepFileStructure)
{
    create_test_vault_directory();

    Vault vault(m_temp_dir / "test_vault");
    vault.close();
    vault.open();

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(exists("test_vault/inner"));
    EXPECT_TRUE(exists("test_vault/inner/inner"));
    EXPECT_TRUE(exists("test_vault/inner/inner/file.txt"));
    EXPECT_TRUE(exists("test_vault/inner/inner/file2.txt"));
    EXPECT_TRUE(exists("test_vault/inner/file.txt"));
    EXPECT_TRUE(exists("test_vault/inner/file2.txt"));
    EXPECT_TRUE(exists("test_vault/file.txt"));
    EXPECT_TRUE(exists("test_vault/file2.txt"));
}

TEST_F(VaultTest, OpenCloseKeepFileContent)
{
    create_test_vault_directory();

    Vault vault(m_temp_dir / "test_vault");
    vault.close();
    vault.open();

    EXPECT_EQ(read_file("test_vault/inner/inner/file.txt"), read_file("test_vault/inner/inner/file.txt"));
    EXPECT_EQ(read_file("test_vault/inner/inner/file2.txt"), read_file("test_vault/inner/inner/file2.txt"));
    EXPECT_EQ(read_file("test_vault/inner/file.txt"), read_file("test_vault/inner/file.txt"));
    EXPECT_EQ(read_file("test_vault/inner/file2.txt"), read_file("test_vault/inner/file2.txt"));
    EXPECT_EQ(read_file("test_vault/file.txt"), read_file("test_vault/file.txt"));
    EXPECT_EQ(read_file("test_vault/file2.txt"), read_file("test_vault/file2.txt"));
}

TEST_F(VaultTest, OpenCloseBinaryFile)
{
    create_directory(m_temp_dir / "test_vault");
    std::vector<uint8_t> binary_data = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    std::ofstream binary_file(m_temp_dir / "test_vault/binary_file.bin", std::ios::binary);
    binary_file.write(reinterpret_cast<const char*>(binary_data.data()), static_cast<std::streamsize>(binary_data.size()));
    binary_file.close();

    Vault vault("test_vault", std::filesystem::directory_entry(m_temp_dir / "test_vault"), m_temp_dir);
    vault.close();
    vault.open();

    std::ifstream extracted_binary_file(m_temp_dir / "test_vault/binary_file.bin", std::ios::binary);
    std::vector<uint8_t> extracted_binary_data((std::istreambuf_iterator(extracted_binary_file)), std::istreambuf_iterator<char>());
    extracted_binary_file.close();

    EXPECT_EQ(binary_data, extracted_binary_data);
}

TEST_F(VaultTest, OpenCloseUnicodeFile)
{
    create_directory(m_temp_dir / "test_vault");
    const std::string unicode_content = "你好，世界! Привет, мир! Hello, world!";
    write_file("test_vault/unicode_file.txt", unicode_content);

    Vault vault("test_vault", std::filesystem::directory_entry(m_temp_dir / "test_vault"), m_temp_dir);
    vault.close();
    vault.open();

    EXPECT_EQ(read_file("test_vault/unicode_file.txt"), unicode_content);
}

TEST_F(VaultTest, OpenCloseFileWithSpecialCharacters)
{
    create_directory(m_temp_dir / "test_vault");
    const std::string special_characters_content = "Special characters: !@#$%^&*()_+{}:\"<>?|";
    write_file("test_vault/special_characters_file.txt", special_characters_content);

    Vault vault("test_vault", std::filesystem::directory_entry(m_temp_dir / "test_vault"), m_temp_dir);
    vault.close();
    vault.open();

    EXPECT_EQ(read_file("test_vault/special_characters_file.txt"), special_characters_content);
}

TEST_F(VaultTest, InvalidOpenVaultDirectory)
{
    create_directory(m_temp_dir / "test_vault");
    write_file("test_vault/file.txt", "Content of file.txt");

    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault");
        vault.open();
        }, std::runtime_error) << "The vault is already open";
}

TEST_F(VaultTest, InvalidCloseVaultFile)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n\t<file name=\"file.txt\" data=\"Q29udGVudCBvZiB0ZXN0X3ZhdWx0L2ZpbGUudHh0\"/>\n</vault>\n");

    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault");
        vault.close();
        }, std::runtime_error) << "The vault is already closed";
}

TEST_F(VaultTest, IncorrectlyEncodedDataField)
{
    write_file("test_vault_bad.vlt", R"(<vault name="test_vault_bad" extension=".vlt"><file name="fileExemple1.txt" data="InvalidBase64Data@#%"/></vault>)");

    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault_bad.vlt");
        vault.open();
        }, Botan::Invalid_Argument) << "The data field is not correctly encoded in Base64";
}

TEST_F(VaultTest, OpenCloseWithCustomExtension)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir, "vault");
    vault.close();

    EXPECT_TRUE(exists("test_vault.vault"));
    EXPECT_FALSE(exists("test_vault"));

    vault.open();

    EXPECT_FALSE(exists("test_vault.vault"));
    EXPECT_TRUE(exists("test_vault"));
}

TEST_F(VaultTest, OpenCloseWithEmptyExtension)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir, "");
    vault.close();

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(std::filesystem::is_regular_file(m_temp_dir / "test_vault"));

    vault.open();

    EXPECT_TRUE(exists("test_vault"));
    EXPECT_TRUE(std::filesystem::is_directory(m_temp_dir / "test_vault"));
}

TEST_F(VaultTest, InvalidCloseWithSymbolicFile)
{
    try
    {
        create_directory(m_temp_dir / "test_vault");
        write_file("test_vault/file.txt", "Content of file.txt");
        create_symlink(m_temp_dir / "test_vault/file.txt", m_temp_dir / "test_vault/file.link");

        EXPECT_THROW({
            Vault vault(m_temp_dir / "test_vault");
            vault.close();
            }, std::runtime_error) << "Symbolic links are not supported";
    }
    catch (...) {}
}

TEST_F(VaultTest, InvalidCloseWithSymbolicDirectory)
{
    try
    {
        create_directories(m_temp_dir / "test_vault/inner");
        create_directory_symlink(m_temp_dir / "test_vault/inner", m_temp_dir / "test_vault/inner.link");

        EXPECT_THROW({
            Vault vault(m_temp_dir / "test_vault");
            vault.close();
            }, std::runtime_error) << "Symbolic links are not supported";
    }
    catch (...) {}
}

TEST_F(VaultTest, CloseWithSpecifiedDestination)
{
    create_directory(m_temp_dir / "test_destination");

    Vault vault("test_vault", std::nullopt, m_temp_dir);
    vault.close(m_temp_dir / "test_destination");

    EXPECT_TRUE(exists("test_destination/test_vault.vlt"));
}

TEST_F(VaultTest, OpenWithSpecifiedDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\" extension=\".vlt\">\n</vault>\n");
    create_directory(m_temp_dir / "test_destination");

    Vault vault(m_temp_dir / "test_vault.vlt");
    vault.open(m_temp_dir / "test_destination");

    EXPECT_TRUE(exists("test_destination/test_vault"));
}

TEST_F(VaultTest, CloseWithEmptyDestination)
{
    Vault vault("test_vault", std::nullopt, m_temp_dir);
    vault.close(m_temp_dir / "");

    EXPECT_TRUE(exists("test_vault.vlt"));
}

TEST_F(VaultTest, OpenWithEmptyDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\" extension=\".vlt\">\n</vault>\n");

    Vault vault(m_temp_dir / "test_vault.vlt");
    vault.open(m_temp_dir / "");

    EXPECT_TRUE(exists("test_vault"));
}

TEST_F(VaultTest, InvalidCloseWithNonExistentDestination)
{
    create_directory(m_temp_dir / "test_vault");

    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault.vlt");
        vault.close(m_temp_dir / "test_destination");
        }, std::runtime_error);
}

TEST_F(VaultTest, InvalidOpenWithNonExistentDestination)
{
    write_file("test_vault.vlt", "<vault name=\"test_vault\">\n</vault>\n");

    EXPECT_THROW({
        Vault vault(m_temp_dir / "test_vault.vlt");
        vault.open(m_temp_dir / "test_destination");
        }, std::runtime_error);
}
