/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#include <toolkit/locale/CultureInfo.hpp>
#include <toolkit/locale/I18nManager.hpp>
#include <toolkit/locale/I18nString.hpp>
#include <toolkit/locale/I18nStringTable.hpp>
#include <toolkit/io/FileStream.hpp>
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/memory/Memory.hpp>

namespace locale_test
{
    class LocaleTestBase : public ::testing::Test
    {
    protected:
        TkString oldFallbackLang;
        bool oldApplyCppLocale = true;

        void SetUp() override
        {
            oldFallbackLang = I18nManager::s_fallbackLang;
            oldApplyCppLocale = I18nManager::s_applyCppLocale;
            I18nManager::dispose();
            I18nManager::clearTables();
            I18nManager::s_fallbackLang = "en";
            I18nManager::s_applyCppLocale = false;
        }

        void TearDown() override
        {
            I18nManager::dispose();
            I18nManager::clearTables();
            I18nManager::s_fallbackLang = oldFallbackLang;
            I18nManager::s_applyCppLocale = oldApplyCppLocale;
        }
    };

    inline TkString docPath(const char *fileName)
    {
        return TkString(LOCALE_TEST_DOC_DIR) / fileName;
    }

    inline I18nStringTable makeEnglishTable()
    {
        I18nStringTable table;
        table.locale = "en";
        table.addString("hello", "Hello");
        table.addPluralString("apples", "%d apple", 0);
        table.addPluralString("apples", "%d apples", 1);
        return table;
    }

    inline I18nStringTable makeBritishTable()
    {
        I18nStringTable table;
        table.locale = "en-GB";
        table.addString("colour", "Colour");
        return table;
    }

    inline I18nStringTable makeFallbackTable()
    {
        I18nStringTable table;
        table.locale = "en";
        table.addString("fallback-only", "Fallback text");
        return table;
    }

    inline I18nStringTable makeCycleSourceTable()
    {
        I18nStringTable table;
        table.locale = "en-US";
        return table;
    }

    inline I18nStringTable makeCycleTargetTable()
    {
        I18nStringTable table;
        table.locale = "en";
        table.addString("cycle-key", "cycle-value-from-b");
        return table;
    }

    inline TkArray<byte> serializeToBytes(const I18nStringTable &table, const TkString &fallbackLocale = TkString::Empty)
    {
        MemoryOStream stream;
        table.serialize(stream, fallbackLocale);

        TkArray<byte> bytes;
        bytes.resize(static_cast<i32>(stream.getPosition()), false);
        if (bytes.count() > 0)
            TkMemory::memoryCopy(bytes.get(), stream.getHandle(), stream.getPosition());
        return bytes;
    }
}
