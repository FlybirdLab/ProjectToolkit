/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "I18nStringTable.hpp"

#include "toolkit/base/Math.hpp"
#include "toolkit/io/Stream.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{
    namespace
    {
        // "I18N"
        static constexpr u32 kI18nTableMagic =
            static_cast<u32>('I') |
            (static_cast<u32>('1') << 8) |
            (static_cast<u32>('8') << 16) |
            (static_cast<u32>('N') << 24);
        static constexpr u32 kI18nTableVersion = 1;

        TkString formatPluralText(const TkString &text, i32 n)
        {
            if (!text.hasChars())
                return TkString::Empty;

            if (text.contains("%"))
                return TkString::format(text.getText(), n);

            return text;
        }
    }

    void I18nStringTable::addString(const TkStringView &id, const TkStringView &value)
    {
        auto &values = entries[id];
        values.resize(1);
        values[0] = value;
    }

    void I18nStringTable::addPluralString(const TkStringView &id, const TkStringView &value, i32 n)
    {
        TK_CHECK(n >= 0 && n < 1024);
        auto &values = entries[id];
        values.resize(TkMath::max(values.count(), n + 1));
        values[n] = value;
    }

    TkString I18nStringTable::getString(const TkString &id) const
    {
        auto it = entries.find(id);
        if (it != entries.End() && it->Value.count() > 0)
        {
            const TkString &result = it->Value[0];
            if (result.hasChars())
                return result;
        }

        if (fallbackTable)
            return fallbackTable->getString(id);
        return TkString::Empty;
    }

    TkString I18nStringTable::getPluralString(const TkString &id, i32 n) const
    {
        auto it = entries.find(id);
        if (it != entries.End())
        {
            const auto &messages = it->Value;
            if (messages.count() > 0)
            {
                i32 index = n <= 0 ? 0 : n - 1;
                if (index >= messages.count())
                    index = messages.count() - 1;

                const TkString &result = messages[index];
                if (result.hasChars())
                    return formatPluralText(result, n);
            }
        }

        if (fallbackTable)
            return fallbackTable->getPluralString(id, n);
        return TkString::Empty;
    }

    bool I18nStringTable::load(ITkStream &stream, TkString *fallbackLocaleOut)
    {
        unload();

        u32 magic = 0;
        u32 version = 0;
        stream.readU32(&magic);
        stream.readU32(&version);
        if (stream.hasError() || magic != kI18nTableMagic || version != kI18nTableVersion)
            return false;

        TkString fallbackLocale;
        stream.read(locale);
        stream.read(fallbackLocale);
        stream.read(entries);

        if (stream.hasError() || !locale.hasChars())
        {
            unload();
            return false;
        }

        if (fallbackLocaleOut)
            *fallbackLocaleOut = moveTemp(fallbackLocale);

        fallbackTable = nullptr;
        return true;
    }

    void I18nStringTable::unload()
    {
        locale.clear();
        fallbackTable = nullptr;
        entries.clear();
    }

    void I18nStringTable::serialize(OTkStream &stream, const TkString &fallbackLocale) const
    {
        stream.writeU32(kI18nTableMagic);
        stream.writeU32(kI18nTableVersion);
        stream.write(locale);
        stream.write(fallbackLocale);
        stream.write(entries);
    }
}
