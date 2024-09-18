#include "EncryptionManager.h"

#include <gtest/gtest.h>
#include <botan/auto_rng.h>

std::vector<std::uint8_t> generate_random_data(const size_t size)
{
    Botan::AutoSeeded_RNG rng;
    std::vector<std::uint8_t> data(size);
    rng.randomize(data.data(), size);
    return data;
}

class EncryptionManagerTest : public testing::Test
{
protected:
    std::string password;
    EncryptionManager::Salt salt{};

    void SetUp() override
    {
        password = "Complex#passW0R12";
        salt = EncryptionManager::generate_new_salt();
    }
};

TEST_F(EncryptionManagerTest, EncryptionDecryption)
{
    auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(std::move(data), password, salt);

    EXPECT_NE(encrypted_data, data);

    auto [decrypted_data, nonce_decrypt] = EncryptionManager::decrypt(std::move(encrypted_data), password, salt, nonce);

    EXPECT_EQ(decrypted_data, data);
}

TEST_F(EncryptionManagerTest, DifferentNonces)
{
    auto data = generate_random_data(1024);

    auto [encrypted_data1, nonce1] = EncryptionManager::encrypt(std::move(data), password, salt);
    auto [encrypted_data2, nonce2] = EncryptionManager::encrypt(std::move(data), password, salt);

    EXPECT_NE(encrypted_data1, encrypted_data2) << "The encryption of a same data with same password and salt should produce different encrypted data";
    EXPECT_NE(nonce1, nonce2) << "Encryption should produce a different nonce each time.";
}

TEST_F(EncryptionManagerTest, WrongPasswordDecryption)
{
    auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(std::move(data), password, salt);

    const std::string wrong_password = "wrong-password";
    auto [decrypted_data, nonce_decrypt] = EncryptionManager::decrypt(std::move(encrypted_data), wrong_password, salt, nonce);

    EXPECT_NE(decrypted_data, data);
}

TEST_F(EncryptionManagerTest, WrongSaltDecryption)
{
    auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(std::move(data), password, salt);

    auto [decrypted_data, nonce_decrypt] = EncryptionManager::decrypt(std::move(encrypted_data), password, EncryptionManager::generate_new_salt(), nonce);

    EXPECT_NE(decrypted_data, data);
}

TEST_F(EncryptionManagerTest, WrongNonceDecryption)
{
    auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(std::move(data), password, salt);

    auto [decrypted_data, nonce_decrypt] = EncryptionManager::decrypt(std::move(encrypted_data), password, salt, {});

    EXPECT_NE(decrypted_data, data);
}
