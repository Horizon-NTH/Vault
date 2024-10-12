#include "EncryptionManager.h"

#include <gtest/gtest.h>
#include <botan/auto_rng.h>

EncryptionManager::Data generate_random_data(const size_t size)
{
    Botan::AutoSeeded_RNG rng;
    EncryptionManager::Data data(size);
    rng.randomize(data.data(), size);
    return data;
}

class EncryptionManagerTest : public testing::Test
{
protected:
    EncryptionManager::Password password;
    EncryptionManager::Salt salt{};

    void SetUp() override
    {
        password = "Complex#passW0R12";
        salt = EncryptionManager::generate_new_salt();
    }
};

TEST_F(EncryptionManagerTest, EncryptionDecryption)
{
    const auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(data, password, salt);

    EXPECT_NE(encrypted_data, data);

    const auto decrypted_data = EncryptionManager::decrypt(std::move(encrypted_data), password, salt, nonce);

    EXPECT_EQ(decrypted_data, data);
}

TEST_F(EncryptionManagerTest, DifferentNonces)
{
    const auto data = generate_random_data(1024);

    auto [encrypted_data1, nonce1] = EncryptionManager::encrypt(data, password, salt);
    auto [encrypted_data2, nonce2] = EncryptionManager::encrypt(data, password, salt);

    EXPECT_NE(encrypted_data1, encrypted_data2) << "The encryption of a same data with same password and salt should produce different encrypted data";
    EXPECT_NE(nonce1, nonce2) << "Encryption should produce a different nonce each time.";
}

TEST_F(EncryptionManagerTest, WrongPasswordDecryption)
{
    const auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(data, password, salt);

    const EncryptionManager::Password wrong_password = "wrong-password";
    EXPECT_THROW({
        auto _ = EncryptionManager::decrypt(std::move(encrypted_data), wrong_password, salt, nonce);
        }, std::runtime_error) << "Decrypting with the wrong password should throw an exception";
}

TEST_F(EncryptionManagerTest, WrongSaltDecryption)
{
    const auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(data, password, salt);

    EXPECT_THROW({
        auto _ = EncryptionManager::decrypt(std::move(encrypted_data), password, EncryptionManager::generate_new_salt(), nonce);
        }, std::runtime_error) << "Decrypting with the wrong salt should throw an exception";
}

TEST_F(EncryptionManagerTest, ModifiedCiphertextFailsDecryption)
{
    const auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(data, password, salt);

    encrypted_data[0] ^= 0xFF;  // Flips the first byte

    EXPECT_THROW({
        auto _ = EncryptionManager::decrypt(std::move(encrypted_data), password, salt, nonce);
        }, std::runtime_error) << "Decrypting modified ciphertext should throw an exception.";
}

TEST_F(EncryptionManagerTest, ModifiedNonceFailsDecryption)
{
    const auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(data, password, salt);

    EncryptionManager::Nonce modified_nonce = nonce;
    modified_nonce[0] ^= 0xFF;

    EXPECT_THROW({
        auto _ = EncryptionManager::decrypt(std::move(encrypted_data), password, salt, modified_nonce);
        }, std::runtime_error) << "Decrypting with a modified nonce should throw an exception.";
}

TEST_F(EncryptionManagerTest, EmptyData)
{
    const EncryptionManager::Data empty_data;
    EXPECT_NO_THROW({
        const auto encrypted = EncryptionManager::encrypt(empty_data, password, salt);
        const auto uncrypted = EncryptionManager::decrypt(encrypted.first, password, salt, encrypted.second);
        }) << "Encrypting empty data should not throw exception";
}

TEST_F(EncryptionManagerTest, CiphertextTooShortFailsDecryption)
{
    const auto data = generate_random_data(1024);
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(data, password, salt);

    encrypted_data.resize(encrypted_data.size() / 2);

    EXPECT_THROW({
        auto _ = EncryptionManager::decrypt(std::move(encrypted_data), password, salt, nonce);
        }, std::runtime_error) << "Decrypting an incomplete ciphertext should throw an exception.";
}

TEST_F(EncryptionManagerTest, NonRandomDataEncryptionDecryption)
{
    const EncryptionManager::Data known_data = {'T', 'e', 's', 't', ' ', 'd', 'a', 't', 'a'};
    auto [encrypted_data, nonce] = EncryptionManager::encrypt(known_data, password, salt);

    const auto decrypted_data = EncryptionManager::decrypt(std::move(encrypted_data), password, salt, nonce);

    EXPECT_EQ(decrypted_data, known_data) << "Decrypted known data should match original.";
}
