#pragma once

#include <string>
#include <array>
#include <vector>
#include <botan/secmem.h>

class EncryptionManager
{
public:
	using Salt = Botan::secure_vector<std::uint8_t>;
	using Nonce = Botan::secure_vector<std::uint8_t>;
	using Key = Botan::secure_vector<std::uint8_t>;
	using Data = Botan::secure_vector<std::uint8_t>;
	using Password = std::basic_string<char, std::char_traits<char>, Botan::secure_allocator<char>>;

	EncryptionManager() = delete;

	[[nodiscard]] static std::pair<Data, Nonce> encrypt(Data, const Password&, const Salt&);
	[[nodiscard]] static Data decrypt(Data, const Password&, const Salt&, const Nonce&);
	[[nodiscard]] static Salt generate_new_salt();

private:
	[[nodiscard]] static Key derive_key(const Password&, const Salt&);
};
