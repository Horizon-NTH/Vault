#include <thread>
#include <botan/argon2.h>
#include <botan/auto_rng.h>
#include <botan/aead.h>

#include "EncryptionManager.h"

#include <iostream>

std::pair<EncryptionManager::Data, EncryptionManager::Nonce> EncryptionManager::encrypt(Data data, const Password& password, const Salt& salt)
{
	if (data.empty())
		return {data, {}};

	const auto encryptor = Botan::AEAD_Mode::create_or_throw("ChaCha20Poly1305", Botan::Cipher_Dir::Encryption);
	Botan::AutoSeeded_RNG rng;
	Nonce nonce(24);
	rng.randomize(nonce);
	Key key = derive_key(password, salt);
	encryptor->set_key(key);
	encryptor->set_associated_data(nullptr, 0);
	encryptor->start(nonce);
	encryptor->finish(data);
	return std::make_pair(data, nonce);
}

EncryptionManager::Data EncryptionManager::decrypt(Data data, const Password& password, const Salt& salt, const Nonce& nonce)
{
	if (data.empty())
		return data;
	if (nonce.size() != 24)
		throw std::invalid_argument("Nonce must be 24 bytes long");

	const auto decryptor = Botan::AEAD_Mode::create_or_throw("ChaCha20Poly1305", Botan::Cipher_Dir::Decryption);
	Key key = derive_key(password, salt);
	decryptor->set_key(key);
	decryptor->set_associated_data(nullptr, 0);
	decryptor->start(nonce);
	try { decryptor->finish(data); }
	catch (const Botan::Integrity_Failure&) { throw std::runtime_error("Decryption failed: Incorrect password or data has been tampered with."); }
	return data;
}

EncryptionManager::Salt EncryptionManager::generate_new_salt()
{
	Botan::AutoSeeded_RNG rng;
	Salt salt(16);
	rng.randomize(salt);
	return salt;
}

EncryptionManager::Key EncryptionManager::derive_key(const Password& password, const Salt& salt)
{
	if (salt.size() != 16)
		throw std::invalid_argument("Salt must be 16 bytes long");
	Key key(32);
	const Botan::Argon2 argon2(2, 64, 3, std::thread::hardware_concurrency() >= 4 ? 4 : 1);
	argon2.derive_key(key.data(), key.size(), password.data(), password.size(), salt.data(), salt.size());
	return key;
}
