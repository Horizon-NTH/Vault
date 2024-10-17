#include "CompressionManager.h"

#include <gtest/gtest.h>

TEST(CompressionManager, CompressValidData)
{
    const CompressionManager::Data data = {'H', 'e', 'l', 'l', 'o'};
    const CompressionManager::Data compressedData = CompressionManager::compress(data);
    ASSERT_FALSE(compressedData.empty());
}

TEST(CompressionManager, UncompressValidData)
{
    const CompressionManager::Data data = {'H', 'e', 'l', 'l', 'o'};
    const CompressionManager::Data compressedData = CompressionManager::compress(data);
    const CompressionManager::Data decompressedData = CompressionManager::uncompress(compressedData, data.size());
    ASSERT_EQ(decompressedData, data);
}

TEST(CompressionManager, CompressEmptyData)
{
    const CompressionManager::Data data;
    const CompressionManager::Data compressedData = CompressionManager::compress(data);
    ASSERT_TRUE(data.empty());
}

TEST(CompressionManager, InvalidUncompressEmptyData)
{
    const CompressionManager::Data data;
    ASSERT_THROW({CompressionManager::uncompress(data, data.size());}, std::runtime_error);
}

TEST(CompressionManager, UncompressInvalidData)
{
    const CompressionManager::Data invalidData = {'I', 'n', 'v', 'a', 'l', 'i', 'd'};
    ASSERT_THROW(CompressionManager::uncompress(invalidData, invalidData.size()), std::runtime_error);
}

TEST(CompressionManager, UncompressWithOriginalSizeMismatch)
{
    const CompressionManager::Data data = {'H', 'e', 'l', 'l', 'o'};
    const CompressionManager::Data compressedData = CompressionManager::compress(data);
    ASSERT_THROW(CompressionManager::uncompress(compressedData, data.size() - 1), std::runtime_error);
    ASSERT_THROW(CompressionManager::uncompress(compressedData, data.size() + 1), std::runtime_error);
}
