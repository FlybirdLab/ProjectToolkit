/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/utils/Convertor.hpp"
#include "toolkit/collection/Array.hpp"

TEST(TkConvertor, RoundDecimal)
{
    EXPECT_DOUBLE_EQ(TkConvertor::roundTo1DecimalPlace(1.24), 1.2);
    EXPECT_DOUBLE_EQ(TkConvertor::roundTo1DecimalPlace(1.25), 1.3);

    EXPECT_DOUBLE_EQ(TkConvertor::roundTo2DecimalPlaces(3.14159), 3.14);
    EXPECT_DOUBLE_EQ(TkConvertor::roundTo3DecimalPlaces(3.14159), 3.142);

    EXPECT_DOUBLE_EQ(TkConvertor::roundTo2DecimalPlaces(-1.235), -1.24);
}

TEST(TkConvertor, CountBits)
{
    EXPECT_EQ(TkConvertor::countBits(0u), 0);
    EXPECT_EQ(TkConvertor::countBits(1u), 1);
    EXPECT_EQ(TkConvertor::countBits(0xFFFFFFFFu), 32);
    EXPECT_EQ(TkConvertor::countBits(0xAAAAAAAAu), 16);
    EXPECT_EQ(TkConvertor::countBits(0x55555555u), 16);
}

TEST(TkConvertor, BytesToText)
{
    EXPECT_STREQ(TkConvertor::bytesToText(0).getText(), "0 B");
    EXPECT_STREQ(TkConvertor::bytesToText(1).getText(), "1 B");
    EXPECT_STREQ(TkConvertor::bytesToText(1023).getText(), "1023 B");
    EXPECT_STREQ(TkConvertor::bytesToText(1024).getText(), "1 KB");
    EXPECT_STREQ(TkConvertor::bytesToText(1536).getText(), "1.5 KB");
    EXPECT_STREQ(TkConvertor::bytesToText(1024 * 1024).getText(), "1 MB");
    EXPECT_STREQ(TkConvertor::bytesToText(5ull * 1024ull * 1024ull * 1024ull).getText(), "5 GB");
}

TEST(TkConvertor, HertzToText)
{
    EXPECT_STREQ(TkConvertor::hertzToText(0).getText(), "0 Hz");
    EXPECT_STREQ(TkConvertor::hertzToText(999).getText(), "999 Hz");
    EXPECT_STREQ(TkConvertor::hertzToText(1000).getText(), "1 KHz");
    EXPECT_STREQ(TkConvertor::hertzToText(1500).getText(), "1.5 KHz");
    EXPECT_STREQ(TkConvertor::hertzToText(2500000).getText(), "2.5 MHz");
}

TEST(TkConvertor, UnsafeMemoryCopy)
{
    byte src[16];
    byte dst[16];

    for (int i = 0; i < 16; ++i)
        src[i] = (byte)i;

    TkConvertor::unsafeMemoryCopy(dst, src, 16);

    EXPECT_EQ(std::memcmp(src, dst, 16), 0);
}

static const char* kUtf8Text = "Hello 世界\\ 🌍";
static const wchar *kUtf16Text = L"Hello 世界\\ 🌍";

TEST(TkConvertor, Utf8Utf16RoundTrip)
{
    i32 len16 = 0;
    wchar buffer16[64] = {};

    TkConvertor::utf8ToUtf16(kUtf8Text, buffer16, (i32)strlen(kUtf8Text), len16);
    EXPECT_GT(len16, 0);

    i32 len8 = 0;
    char buffer8[64] = {};
    TkConvertor::utf16ToUtf8(buffer16, buffer8, len16, len8);

    EXPECT_STREQ(buffer8, kUtf8Text);
}

TEST(TkConvertor, Utf8ToUtf16Heap)
{
    i32 len16 = 0;
    wchar *out = TkConvertor::utf8ToUtf16(kUtf8Text, (i32)strlen(kUtf8Text), len16);

    ASSERT_NE(out, nullptr);
    EXPECT_GT(len16, 0);
    EXPECT_EQ(out[len16], 0);

    TkMemory::free(TkMemory::getDefault(), out);
}

TEST(TkConvertor, Utf16ToUtf8Heap)
{
    i32 len8 = 0;
    char *out = TkConvertor::utf16ToUtf8(kUtf16Text, (i32)wcslen(kUtf16Text), len8);

    ASSERT_NE(out, nullptr);
    EXPECT_STREQ(out, kUtf8Text);

    TkMemory::free(TkMemory::getDefault(), out);
}

TEST(StringAs, ANSI_FromUtf16)
{
    StringAsANSI<32> s(kUtf16Text);
    EXPECT_STREQ(s.get(), kUtf8Text);
}

TEST(StringAs, UTF8_FromUtf16)
{
    StringAsUTF8<32> s(kUtf16Text);
    EXPECT_STREQ(s.get(), kUtf8Text);
}

TEST(StringAs, UTF16_FromUtf8)
{
    StringAsUTF16<32> s(kUtf8Text);
    EXPECT_STREQ(s.get(), kUtf16Text);
}

TEST(StringAs, InlineVsHeap)
{
    const wchar *shortText = L"short";
    const wchar *longText =
        L"This is a very very very very very very long text";

    StringAsUTF8<64> inlineStr(shortText);
    StringAsUTF8<16> heapStr(longText);

    EXPECT_NE(inlineStr.get(), nullptr);
    EXPECT_NE(heapStr.get(), nullptr);
}

TEST(StringAs, InlineUtf16ToUtf8CapacityUsesOutputBytes)
{
    const wchar *text = L"世界";
    StringAsUTF8<8> inlineStr(text);

    EXPECT_STREQ(inlineStr.get(), "\xE4\xB8\x96\xE7\x95\x8C");
}

TEST(StringAs, InlineAnsiFromUtf16UsesSourceLength)
{
    const wchar text[] = {L'O', L'K', 0, L'X', 0};
    StringAsANSI<16> s(text, 2);

    EXPECT_STREQ(s.get(), "OK");
}

TEST(StringAsTerminated, AlreadyTerminated)
{
    const char text[] = "hello";
    StringAsTerminated<char> s(text, 5);

    EXPECT_STREQ((const char *)s, "hello");
}

TEST(StringAsTerminated, NotTerminated)
{
    const char raw[] = {'h', 'i'};
    StringAsTerminated<char> s(raw, 2);

    EXPECT_STREQ((const char *)s, "hi");
}

TEST(StringAsTerminated, EmptyInput)
{
    StringAsTerminated<char> s(nullptr, 4);

    EXPECT_STREQ((const char *)s, "");
}

TEST(TkConvertor, Base64RoundTrip)
{
    TkConvertor c;

    const byte data[] = {1, 2, 3, 4, 5, 6, 7};
    TkArray<char> encoded;
    TkArray<byte> decoded;

    c.base64Encode(data, 7, encoded);
    c.base64Decode(encoded.get(), encoded.count(), decoded);

    ASSERT_EQ(decoded.count(), 7);
    EXPECT_EQ(std::memcmp(data, decoded.get(), 7), 0);
}

TEST(TkConvertor, Base64KnownValues)
{
    TkConvertor c;
    TkArray<char> encoded;

    const byte one[] = {'M'};
    c.base64Encode(one, 1, encoded);
    ASSERT_EQ(encoded.count(), 4);
    EXPECT_EQ(std::memcmp(encoded.get(), "TQ==", 4), 0);

    const byte two[] = {'M', 'a'};
    c.base64Encode(two, 2, encoded);
    ASSERT_EQ(encoded.count(), 4);
    EXPECT_EQ(std::memcmp(encoded.get(), "TWE=", 4), 0);

    const byte three[] = {'M', 'a', 'n'};
    c.base64Encode(three, 3, encoded);
    ASSERT_EQ(encoded.count(), 4);
    EXPECT_EQ(std::memcmp(encoded.get(), "TWFu", 4), 0);
}

TEST(TkConvertor, Base64Empty)
{
    TkConvertor c;
    TkArray<char> encoded;
    TkArray<byte> decoded;

    c.base64Encode(nullptr, 0, encoded);
    c.base64Decode(nullptr, 0, decoded);

    EXPECT_EQ(encoded.count(), 0);
    EXPECT_EQ(decoded.count(), 0);
}

TEST(TkConvertor, EncryptDecrypt)
{
    TkConvertor c;

    byte data[] = {10, 20, 30, 40, 50};
    byte original[5];
    memcpy(original, data, 5);

    c.encryptBytes(data, 5);
    c.decryptBytes(data, 5);

    EXPECT_EQ(std::memcmp(data, original, 5), 0);
}
