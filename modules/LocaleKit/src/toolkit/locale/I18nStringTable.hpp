/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/locale/common.hpp>
#include "toolkit/types/String.hpp"
#include "toolkit/collection/HashMap.hpp"

namespace toolkit
{
    class ITkStream;
    class OTkStream;

    // Contains localized strings table for a given culture.
    class TK_API I18nStringTable
    {
    public:
        // The locale of the localized string table (eg. pl-PL).
        TkString locale;
        // The fallback language table to use for missing keys. Eg. table for 'en-GB' can point to 'en' as a fallback to prevent problem of missing localized strings.
        I18nStringTable* fallbackTable = nullptr;
        // The string table. Maps the message id into the localized text. For plural messages the list contains separate items for value numbers.
        TkHashMap<TkString, TkArray<TkString>> entries;

    public:
        // Add the localized string to the table.
        void addString(const TkStringView &id, const TkStringView &value);
        // Add the localized plural string to the table.
        void addPluralString(const TkStringView &id, const TkStringView &value, i32 n);

        // Get the localized string by using string id lookup. Uses fallback table if text is not included in this table.
        TkString getString(const TkString &id) const;
        // Get the localized plural string by using string id lookup. Uses fallback table if text is not included in this table.
        TkString getPluralString(const TkString &id, i32 n) const;

    public:
        // Load table contents from a data stream. fallbackLocaleOut receives fallback locale id if payload contains it.
        bool load(ITkStream& stream, TkString* fallbackLocaleOut = nullptr);
        // Clear all table data and detach fallback pointer.
        void unload();
        // Serialize table contents to a data stream. fallbackLocale is stored as locale id (not pointer).
        void serialize(OTkStream& stream, const TkString& fallbackLocale = TkString::Empty) const;
    };

}
