#include "Application.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockVaultManager final : public VaultManager
{
public:
    MOCK_METHOD(void, open_vault, (const std::filesystem::path& vault, const std::optional<std::filesystem::path>& destination), (override));
    MOCK_METHOD(void, close_vault, (const std::filesystem::path& vault, const std::optional<std::filesystem::path>& destination, const std::optional<std::string>& extension), (override));
};

class ApplicationTest : public testing::Test
{
protected:
    std::unique_ptr<Application> m_app;
    MockVaultManager* m_vaultManagerPtr = nullptr;
    std::unique_ptr<MockVaultManager> m_vaultManager;
    std::filesystem::path m_temp_dir;

    void init(const std::span<const char*>& args)
    {
        m_app = std::make_unique<Application>(args, std::move(m_vaultManager));
    }

    [[nodiscard]] std::filesystem::path create_file(const std::string& name) const
    {
        const auto path = m_temp_dir / name;
        std::ofstream file(path);
        file << "test";
        return path;
    }

    [[nodiscard]] std::filesystem::path create_directory(const std::string& name) const
    {
        std::filesystem::create_directory(m_temp_dir / name);
        return m_temp_dir / name;
    }

    void SetUp() override
    {
        std::cout.setstate(std::ios_base::failbit);
        std::cerr.setstate(std::ios_base::failbit);
        if (exists(m_temp_dir))
            remove_all(m_temp_dir);
        m_vaultManager = std::make_unique<MockVaultManager>();
        m_vaultManagerPtr = m_vaultManager.get();
        m_temp_dir = std::filesystem::temp_directory_path() / "vault_app_test";
        std::filesystem::create_directory(m_temp_dir);
    }

    void TearDown() override
    {
        std::cout.clear();
        std::cerr.clear();
        if (exists(m_temp_dir))
            remove_all(m_temp_dir);
    }
};

TEST_F(ApplicationTest, NoArgs)
{
    const char* args[] = {"vault"};

    init(args);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteOpenWithValidArgs)
{
    const auto vault = create_file("vault.vlt").string();
    const char* args[] = {"vault", "open", "--vault", vault.c_str()};
    init(args);

    EXPECT_CALL(*m_vaultManagerPtr, open_vault(testing::Eq(vault), testing::Eq(std::nullopt))).Times(1);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteCloseWithValidArgs)
{
    const auto vault = create_directory("vault").string();
    const char* args[] = {"vault", "close", "--vault", vault.c_str()};
    init(args);

    EXPECT_CALL(*m_vaultManagerPtr, close_vault(testing::Eq(vault), testing::Eq(std::nullopt), testing::Eq(std::nullopt))).Times(1);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithNoProvidedArgs)
{
    const char* args[] = {"vault", "open", "--vault"};
    init(args);

    EXPECT_NE(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithVersionFlag)
{
    const char* args[] = {"vault", "--version"};
    init(args);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithHelpSubcommand)
{
    const char* args[] = {"vault", "help"};
    init(args);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithoutName)
{
    const char* args[] = {"vault", "create"};
    init(args);

    EXPECT_NE(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithNonExistentVault)
{
    const char* args[] = {"vault", "open", "--vault", "nonexistent.vault"};
    init(args);

    EXPECT_NE(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithOpenAndDestination)
{
    const auto vault = create_file("vault.vlt").string();
    const auto destination = create_directory("destination").string();
    const char* args[] = {"vault", "open", "--vault", vault.c_str(), "--destination", destination.c_str()};

    init(args);

    EXPECT_CALL(*m_vaultManagerPtr, open_vault(testing::Eq(vault), testing::Eq(destination))).Times(1);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithCloseAndDestination)
{
    const auto vault = create_directory("vault").string();
    const auto destination = create_directory("destination").string();
    const char* args[] = {"vault", "close", "--vault", vault.c_str(), "--destination", destination.c_str()};

    EXPECT_CALL(*m_vaultManager, close_vault(testing::Eq(vault), testing::Eq(destination), testing::Eq(std::nullopt))).Times(1);

    init(args);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithOpenWithDifferentFlagTypes)
{
    const auto vault = create_file("vault.vlt").string();
    const auto destination = create_directory("destination").string();
    const auto concatenated = "--destination=" + destination;
    const char* args[] = {"vault", "open", "-v", vault.c_str(), destination.c_str()};

    EXPECT_CALL(*m_vaultManager, open_vault(testing::Eq(vault), testing::Eq(destination))).Times(1);

    init(args);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}

TEST_F(ApplicationTest, ExecuteWithCloseWithDifferentFlagTypes)
{
    const auto vault = create_directory("vault").string();
    const auto destination = create_directory("destination").string();
    const char* args[] = {"vault", "close", "--destination", destination.c_str(), vault.c_str()};

    EXPECT_CALL(*m_vaultManager, close_vault(testing::Eq(vault), testing::Eq(destination), testing::Eq(std::nullopt))).Times(1);

    init(args);

    EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
}
