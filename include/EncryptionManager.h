#pragma once

#include <string>
#include <array>
#include <vector>
#include <botan/secmem.h>

class EncryptionManager
{
public:
	using Salt = std::array<std::uint8_t, 16>;
	using Nonce = std::array<std::uint8_t, 24>;
	using Key = Botan::secure_vector<std::uint8_t>;

	EncryptionManager() = delete;

	[[nodiscard]] static std::pair<std::vector<std::uint8_t>, Nonce> encrypt(std::vector<std::uint8_t> data, const std::string& password, const Salt&);
	[[nodiscard]] static std::vector<std::uint8_t> decrypt(std::vector<std::uint8_t> data, const std::string& password, const Salt&, const Nonce&);
	[[nodiscard]] static Salt generate_new_salt();

private:
	[[nodiscard]] static Key derive_key(const std::string& password, const Salt&);
};
