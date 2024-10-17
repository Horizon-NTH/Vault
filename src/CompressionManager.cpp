#include "CompressionManager.h"

#include <stdexcept>
#include <zlib.h>

CompressionManager::Data CompressionManager::compress(const Data& data)
{
	const auto srcLen = static_cast<uLong>(data.size());
	uLong destLen = compressBound(srcLen);
	Data compressedData(destLen);

	if (::compress(compressedData.data(), &destLen, data.data(), srcLen))
		throw std::runtime_error("Compression failed");

	compressedData.resize(destLen);
	return compressedData;
}

CompressionManager::Data CompressionManager::uncompress(const Data& data, const size_t originalSize)
{
	Data decompressedData(originalSize);

	auto uncompressedSize = static_cast<uLongf>(originalSize);
	if (::uncompress(decompressedData.data(), &uncompressedSize, data.data(), static_cast<uLong>(data.size())))
		throw std::runtime_error("Decompression failed");
	if (uncompressedSize != originalSize)
		throw std::runtime_error("Decompressed data size mismatch");

	return decompressedData;
}
