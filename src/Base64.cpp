#include "../include/Base64.h"

#include <stdexcept>

std::string Base64::encode(const std::vector<uint8_t>& data)
{
	if (data.empty())
		return {};

	std::string result;
	result.reserve((data.size() + 2) / 3 * 4);

	int overflow = 0;
	for (auto it = data.begin(); it != data.end();)
	{
		const uint8_t first = *it++;
		const uint8_t second = it != data.end() ? static_cast<uint8_t>(*it++) : (overflow++, 0x00);
		const uint8_t third = it != data.end() ? static_cast<uint8_t>(*it++) : (overflow++, 0x00);

		result.push_back(base64_chars[first >> 2]);
		result.push_back(base64_chars[(first & 0b11) << 4 | second >> 4]);
		result.push_back(overflow == 2 ? '=' : base64_chars[(second & 0b1111) << 2 | third >> 6]);
		result.push_back(overflow == 2 || overflow == 1 ? '=' : base64_chars[third & 0b111111]);
	}
	return std::move(result);
}

std::vector<uint8_t> Base64::decode(const std::string& data)
{
	if (data.empty())
		return {};

	if (data.size() % 4 != 0)
		throw std::invalid_argument("Invalid base64 string size.");
	if (data.find_first_not_of(base64_chars) != std::string::npos)
		throw std::invalid_argument("Invalid base64 string characters.");
	if (const auto substr = data.substr(0, data.size() - 2); substr.find('=') != std::string::npos)
		throw std::invalid_argument("Invalid base64 string padding.");

	std::vector<uint8_t> result;
	result.reserve(data.size() / 4 * 3);
	auto it = data.begin();
	while (it != data.end())
	{
		const uint8_t b1 = base64_reverse[*it++];
		const uint8_t b2 = base64_reverse[*it++];
		const uint8_t b3 = base64_reverse[*it++];
		const uint8_t b4 = base64_reverse[*it++];

		result.push_back(b1 << 2 | b2 >> 4);
		if (*(it - 2) != '=')
			result.push_back((b2 & 0b1111) << 4 | b3 >> 2);
		if (*(it - 1) != '=')
			result.push_back((b3 & 0b11) << 6 | b4);
	}
	return std::move(result);
}
