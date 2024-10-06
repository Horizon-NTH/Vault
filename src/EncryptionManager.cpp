#include <thread>
#include <botan/argon2.h>
#include <botan/auto_rng.h>
#include <botan/aead.h>

#include "EncryptionManager.h"

std::pair<std::vector<std::uint8_t>, EncryptionManager::Nonce> EncryptionManager::encrypt(std::vector<std::uint8_t> data, const std::string& password, const Salt& salt)
{
	const auto encryptor = Botan::AEAD_Mode::create_or_throw("ChaCha20Poly1305", Botan::Cipher_Dir::Encryption);
	Botan::AutoSeeded_RNG rng;
	Nonce nonce{};
	rng.randomize(nonce);
	Key key = derive_key(password, salt);
	encryptor->set_key(key);
	encryptor->set_associated_data(nullptr, 0);
	encryptor->start(nonce);
	encryptor->finish(data);
	return std::make_pair(data, nonce);
}

std::vector<std::uint8_t> EncryptionManager::decrypt(std::vector<std::uint8_t> data, const std::string& password, const Salt& salt, const Nonce& nonce)
{
	const auto decryptor = Botan::AEAD_Mode::create_or_throw("ChaCha20Poly1305", Botan::Cipher_Dir::Decryption);
	Key key = derive_key(password, salt);
	decryptor->set_key(key);
	decryptor->set_associated_data(nullptr, 0);
	decryptor->start(nonce);
	decryptor->finish(data);
	return data;
}

EncryptionManager::Salt EncryptionManager::generate_new_salt()
{
	Botan::AutoSeeded_RNG rng;
	Salt salt{};
	rng.randomize(salt);
	return salt;
}

EncryptionManager::Key EncryptionManager::derive_key(const std::string& password, const Salt& salt)
{
	Key key(32);
	const Botan::Argon2 argon2(2, 64, 3, std::thread::hardware_concurrency() >= 4 ? 4 : 1);
	argon2.derive_key(key.data(), key.size(), password.data(), password.size(), salt.data(), salt.size());
	return key;
}
