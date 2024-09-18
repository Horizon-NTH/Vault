#pragma once

#include <string>
#include <array>
#include <vector>

class EncryptionManager
{
public:
	using Salt = std::array<std::uint8_t, 16>;
	using Key = std::array<std::uint8_t, 32>;
	using Nonce = std::array<std::uint8_t, 24>;

	EncryptionManager() = delete;

	static [[nodiscard]] std::pair<std::vector<std::uint8_t>, Nonce> encrypt(std::vector<std::uint8_t>&& data, const std::string& password, const Salt&);
	static [[nodiscard]] std::pair<std::vector<std::uint8_t>, Nonce> decrypt(std::vector<std::uint8_t>&& data, const std::string& password, const Salt&, const Nonce&);
	static [[nodiscard]] Salt generate_new_salt();

private:
	static [[nodiscard]] Key derive_key(const std::string& password, const Salt&);
};
