#include <gtest/gtest.h>
#include "Base64.h"

TEST(Base64Test, EncodeDecodeGeneral)
{
    const std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    const std::string encoded = Base64::encode(data);
    const std::vector<uint8_t> decoded = Base64::decode(encoded);

    EXPECT_EQ(encoded, "SGVsbG8gV29ybGQh");
    EXPECT_EQ(decoded, data);
}

TEST(Base64Test, EmptyData)
{
    const std::vector<uint8_t> emptyData;
    const std::string encoded = Base64::encode(emptyData);
    const std::vector<uint8_t> decoded = Base64::decode(encoded);

    EXPECT_EQ(encoded, "");
    EXPECT_EQ(decoded, emptyData);
}

TEST(Base64Test, PaddingWithOneCharacter)
{
    const std::vector<uint8_t> data = {'M'};
    const std::string encoded = Base64::encode(data);
    const std::vector<uint8_t> decoded = Base64::decode(encoded);

    EXPECT_EQ(encoded, "TQ==");
    EXPECT_EQ(decoded, data);
}

TEST(Base64Test, PaddingWithTwoCharacters)
{
    const std::vector<uint8_t> data = {'M', 'a'};
    const std::string encoded = Base64::encode(data);
    const std::vector<uint8_t> decoded = Base64::decode(encoded);

    EXPECT_EQ(encoded, "TWE=");
    EXPECT_EQ(decoded, data);
}

TEST(Base64Test, BinaryData)
{
    const std::vector<uint8_t> binaryData = {0x00, 0xFF, 0xAC, 0x01, 0x02};
    const std::string encoded = Base64::encode(binaryData);
    const std::vector<uint8_t> decoded = Base64::decode(encoded);

    EXPECT_EQ(encoded, "AP+sAQI=");
    EXPECT_EQ(decoded, binaryData);
}

TEST(Base64Test, InvalidBase64StringSize)
{
    const std::string invalidBase64 = "SGVbG8=";
    EXPECT_THROW(Base64::decode(invalidBase64), std::invalid_argument);
}

TEST(Base64Test, InvalidBase64Characters)
{
    const std::string invalidBase64 = "SGVsbG8@";
    EXPECT_THROW(Base64::decode(invalidBase64), std::invalid_argument);
}

TEST(Base64Test, InvalidBase64Padding)
{
    const std::string invalidBase64 = "SGVsbG8==A";
    EXPECT_THROW(Base64::decode(invalidBase64), std::invalid_argument);
}

TEST(Base64Test, InvalidBase64PaddingWithGoodPadding)
{
    const std::string invalidBase64 = "S=VsbG8==";
    EXPECT_THROW(Base64::decode(invalidBase64), std::invalid_argument);
}

TEST(Base64Test, Base64WithOnlyPadding)
{
    const std::string invalidBase64 = "====";
    EXPECT_THROW(Base64::decode(invalidBase64), std::invalid_argument);
}

TEST(Base64Test, LongString)
{
    std::string longString(1000, 'A');
    const std::vector<uint8_t> data(longString.begin(), longString.end());

    const std::string encoded = Base64::encode(data);
    const std::vector<uint8_t> decoded = Base64::decode(encoded);

    EXPECT_EQ(decoded, data);
}

TEST(Base64Test, ExactDecodingWithNoPadding)
{
    const std::vector<uint8_t> data = {'M', 'a', 'n'};
    const std::string encoded = Base64::encode(data);
    const std::vector<uint8_t> decoded = Base64::decode(encoded);

    EXPECT_EQ(encoded, "TWFu");
    EXPECT_EQ(decoded, data);
}
