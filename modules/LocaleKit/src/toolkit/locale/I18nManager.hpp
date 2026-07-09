/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include <toolkit/locale/common.hpp>
#include "toolkit/collection/Array.hpp"
#include "I18nStringTable.hpp"

namespace toolkit
{
    class ITkStream;

    // The language and culture localization manager.
    class TK_API I18nManager final
    {
    public:
        // Initialize localization runtime state.
        static bool init();
        // Update localization runtime state (reserved for future runtime updates).
        static void update();
        // Release localization runtime state.
        static void dispose();

        // Rebuild internal active/fallback locale tables after changing tables or locale.
        static void refreshLocalization();

    public:
        // Get the current culture (date, time, currency and values formatting locale).
        static const CultureInfo &getCurrentCulture();

        // Sets the current culture (date, time, currency and values formatting locale).
        static void setCurrentCulture(const CultureInfo &value);
        // Get the current language (text display locale).
        static const CultureInfo &getCurrentLanguage();
        // Sets the current language (text display locale).
        static void setCurrentLanguage(const CultureInfo &value);
        // Sets both the current language (text display locale) and the current culture (date, time, currency and values formatting locale) at once.
        static void setCurrentLanguageCulture(const CultureInfo &value);

        // Occur when current culture or language gets changed. Can be used to refresh UI to reflect language changes.
        static TkAction<> onLocalizationChanged;

    public:
        // Get the localized string for the current language by using string id lookup.
        static TkString getString(const TkString &id, const TkString &fallback = TkString::Empty);
        // Get the localized plural string for the current language by using string id lookup.
        static TkString getPluralString(const TkString &id, i32 n, const TkString &fallback = TkString::Empty);

    public:
        // Removes all registered locale tables and clears resolved runtime mappings.
        static void clearTables();
        // Add a locale table. fallbackLocale can be empty if table fallback is set manually.
        static bool addTable(const I18nStringTable &table, const TkString &fallbackLocale = TkString::Empty);
        // Add a locale table. fallbackLocale can be empty if table fallback is set manually.
        static bool addTable(I18nStringTable &&table, const TkString &fallbackLocale = TkString::Empty);
        // Load and register a locale table from a binary file path.
        static bool loadTableFromFile(const TkStringView &path);
        // Load and register a locale table from a binary stream.
        static bool loadTableFromStream(ITkStream &stream);

    public:
        // The list of the string localization tables used.
        static TkArray<I18nStringTable> s_localeTables;
        // The default fallback language to use if localization system fails to pick the system locale language (eg. en-GB).
        static TkString s_fallbackLang;
        // True to apply process locale-related env vars via System on localization refresh.
        static bool s_applyCppLocale;
    };

}
