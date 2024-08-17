#pragma once
#include <array>
#include <string>
#include <vector>

class Base64
{
public:
	Base64() = delete;
	Base64(const Base64&) = delete;

	static std::string encode(const std::vector<uint8_t>& data);
	static std::vector<uint8_t> decode(const std::string& data);

private:
	static constexpr std::string_view base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	static constexpr std::array<uint8_t, 256> base64_reverse =
			[]
				{
					std::array<uint8_t, 256> result{};
					for (uint8_t i = 0; i < static_cast<uint8_t>(base64_chars.size()); ++i)
						result[base64_chars[i]] = i;
					return result;
				}();
};
