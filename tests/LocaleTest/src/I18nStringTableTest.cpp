/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LocaleTestUtils.hpp"

using namespace locale_test;

TEST(I18nStringTableTest, AddAndGetString)
{
    I18nStringTable table = makeEnglishTable();

    EXPECT_EQ(table.getString("hello"), "Hello");
    EXPECT_EQ(table.getString("missing"), TkString::Empty);
}

TEST(I18nStringTableTest, AddAndGetPluralString)
{
    I18nStringTable table = makeEnglishTable();

    EXPECT_EQ(table.getPluralString("apples", 0), "0 apple");
    EXPECT_EQ(table.getPluralString("apples", 1), "1 apple");
    EXPECT_EQ(table.getPluralString("apples", 2), "2 apples");
    EXPECT_EQ(table.getPluralString("missing", 2), TkString::Empty);
}

TEST(I18nStringTableTest, FallbackTableIsUsed)
{
    I18nStringTable fallback = makeEnglishTable();
    I18nStringTable table = makeBritishTable();
    table.fallbackTable = &fallback;

    EXPECT_EQ(table.getString("hello"), "Hello");
    EXPECT_EQ(table.getPluralString("apples", 1), "1 apple");
}

TEST(I18nStringTableTest, SerializeAndLoadRoundTrip)
{
    I18nStringTable source = makeEnglishTable();
    source.locale = "en-US";

    TkArray<byte> bytes = serializeToBytes(source, "en");
    MemoryIStream input(bytes.get(), static_cast<u32>(bytes.count()));

    I18nStringTable loaded;
    TkString fallbackLocale;
    ASSERT_TRUE(loaded.load(input, &fallbackLocale));

    EXPECT_EQ(loaded.locale, "en-US");
    EXPECT_EQ(fallbackLocale, "en");
    EXPECT_EQ(loaded.getString("hello"), "Hello");
    EXPECT_EQ(loaded.getPluralString("apples", 1), "1 apple");
}

TEST(I18nStringTableTest, SerializeAndLoadWithEmptyFallbackLocale)
{
    I18nStringTable source = makeBritishTable();
    source.addPluralString("apples", "%d apple", 0);

    TkArray<byte> bytes = serializeToBytes(source);
    MemoryIStream input(bytes.get(), static_cast<u32>(bytes.count()));

    I18nStringTable loaded;
    TkString fallbackLocale;
    ASSERT_TRUE(loaded.load(input, &fallbackLocale));

    EXPECT_EQ(loaded.locale, "en-GB");
    EXPECT_TRUE(fallbackLocale.isEmpty());
    EXPECT_EQ(loaded.getString("colour"), "Colour");
    EXPECT_EQ(loaded.getPluralString("apples", 3), "3 apple");
}

TEST(I18nStringTableTest, LoadInvalidMagicFails)
{
    FileIStream *stream = FileIStream::open(docPath("i18n_invalid_magic.tla"));
    ASSERT_NE(stream, nullptr);

    I18nStringTable loaded;
    TkString fallbackLocale;
    EXPECT_FALSE(loaded.load(*stream, &fallbackLocale));
    EXPECT_TRUE(loaded.locale.isEmpty());
    EXPECT_EQ(loaded.entries.count(), 0);

    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}

TEST(I18nStringTableTest, LoadEmptyLocaleFails)
{
    MemoryOStream output;
    output.writeU32(0x4E383149u);
    output.writeU32(1u);
    output.write(TkString::Empty);
    output.write(TkString::Empty);
    output.write(TkHashMap<TkString, TkArray<TkString>>());

    MemoryIStream input(output.getHandle(), static_cast<u32>(output.getPosition()));
    I18nStringTable loaded;
    TkString fallbackLocale;
    EXPECT_FALSE(loaded.load(input, &fallbackLocale));
    EXPECT_TRUE(loaded.locale.isEmpty());
    EXPECT_EQ(loaded.entries.count(), 0);
}

TEST(I18nStringTableTest, UnloadClearsData)
{
    I18nStringTable table = makeEnglishTable();
    table.unload();

    EXPECT_TRUE(table.locale.isEmpty());
    EXPECT_EQ(table.entries.count(), 0);
    EXPECT_EQ(table.fallbackTable, nullptr);
}

TEST(I18nStringTableTest, LoadFromDocSampleWorks)
{
    FileIStream *stream = FileIStream::open(docPath("i18n_en_v1.tla"));
    ASSERT_NE(stream, nullptr);

    I18nStringTable loaded;
    TkString fallbackLocale;
    ASSERT_TRUE(loaded.load(*stream, &fallbackLocale));
    EXPECT_EQ(loaded.locale, "en");
    EXPECT_TRUE(fallbackLocale.isEmpty());
    EXPECT_EQ(loaded.getString("hello"), "Hello");
    EXPECT_EQ(loaded.getPluralString("apples", 2), "2 apples");

    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}

TEST(I18nStringTableTest, LoadBritishDocSampleUsesFallbackLocale)
{
    FileIStream *stream = FileIStream::open(docPath("i18n_en_gb_v1.tla"));
    ASSERT_NE(stream, nullptr);

    I18nStringTable loaded;
    TkString fallbackLocale;
    ASSERT_TRUE(loaded.load(*stream, &fallbackLocale));

    EXPECT_EQ(loaded.locale, "en-GB");
    EXPECT_EQ(fallbackLocale, "en");
    EXPECT_EQ(loaded.getString("hello"), "Hallo");
    EXPECT_EQ(loaded.getString("missing"), TkString::Empty);

    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}
