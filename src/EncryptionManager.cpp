#include <thread>
#include <botan/argon2.h>
#include <botan/auto_rng.h>
#include <botan/stream_cipher.h>

#include "EncryptionManager.h"

std::pair<std::vector<std::uint8_t>, EncryptionManager::Nonce> EncryptionManager::encrypt(std::vector<std::uint8_t>&& data, const std::string& password, const Salt& salt)
{
	Botan::AutoSeeded_RNG rng;
	Nonce nonce{};
	rng.randomize(nonce);
	Key key = derive_key(password, salt);
	const auto cipher = Botan::StreamCipher::create("ChaCha20");
	if (!cipher)
		throw std::runtime_error("ChaCha20 cipher not available!");
	cipher->set_key(key);
	cipher->set_iv(nonce);
	auto dataToEncrypt = data;
	cipher->encrypt(dataToEncrypt);
	return std::make_pair(dataToEncrypt, nonce);
}

std::pair<std::vector<std::uint8_t>, EncryptionManager::Nonce> EncryptionManager::decrypt(std::vector<std::uint8_t>&& data, const std::string& password, const Salt& salt, const Nonce& nonce)
{
	const auto cipher = Botan::StreamCipher::create("ChaCha20");
	Key key = derive_key(password, salt);
	if (!cipher)
		throw std::runtime_error("ChaCha20 cipher not available!");
	cipher->set_key(key);
	cipher->set_iv(nonce);
	auto dataToEncrypt = data;
	cipher->decrypt(dataToEncrypt);
	return std::make_pair(dataToEncrypt, nonce);
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
	Key key{};
	const Botan::Argon2 argon2(2, 64, 3, std::thread::hardware_concurrency() >= 4 ? 4 : 1);
	argon2.derive_key(key.data(), key.size(), password.data(), password.size(), salt.data(), salt.size());
	return key;
}
