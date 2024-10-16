#pragma once

#include <botan/secmem.h>

class CompressionManager
{
public:
	using Data = Botan::secure_vector<std::uint8_t>;

	CompressionManager() = delete;

	static Data compress(const Data&);
	static Data uncompress(const Data&, size_t originalSize);
};
