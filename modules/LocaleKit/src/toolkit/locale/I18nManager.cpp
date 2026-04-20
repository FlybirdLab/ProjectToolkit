/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "I18nManager.hpp"

#include "CultureInfo.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/io/FileStream.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/base/Function.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{
    TkAction<> I18nManager::onLocalizationChanged;
    TkArray<I18nStringTable> I18nManager::s_localeTables;
    TkString I18nManager::s_fallbackLang = "en";
    bool I18nManager::s_applyCppLocale = true;

    CultureInfo g_currentCulture(0);
    CultureInfo g_currentLanguage(0);
    TkArray<I18nStringTable *> g_activeTables;
    TkArray<I18nStringTable *> g_fallbackTables;
    TkArray<TkString> g_tableFallbackLocales;
    bool g_initialized = false;

    namespace
    {
        const TkArray<I18nStringTable *> *findTablesForLocale(const TkHashMap<TkString, TkArray<I18nStringTable *>> &mapping, const TkString &locale)
        {
            if (locale.isEmpty())
                return nullptr;
            auto it = mapping.find(locale);
            return it != mapping.End() ? &it->Value : nullptr;
        }

        TkString findFirstLocaleKeyByValue(const TkHashMap<TkString, TkArray<I18nStringTable *>> &mapping, const TkArray<I18nStringTable *> *target)
        {
            if (target == nullptr)
                return TkString::Empty;
            for (auto it = mapping.Begin(); it.isNotEnd(); ++it)
            {
                if (&it->Value == target)
                    return it->Key;
            }
            return TkString::Empty;
        }

        TkString formatPluralText(const TkString &text, i32 n);

        TkString tryGetFromMessages(const TkArray<TkString> *messages, i32 index)
        {
            if (messages == nullptr || index < 0 || index >= messages->count())
                return TkString::Empty;
            const TkString &value = (*messages)[index];
            return value.hasChars() ? value : TkString::Empty;
        }

        TkString tryGetPluralFromMessages(const TkArray<TkString> *messages, i32 n)
        {
            if (messages == nullptr || messages->count() == 0)
                return TkString::Empty;

            i32 index = n <= 0 ? 0 : n - 1;
            if (index >= messages->count())
                index = messages->count() - 1;

            const TkString &value = (*messages)[index];
            return value.hasChars() ? formatPluralText(value, n) : TkString::Empty;
        }

        TkString tryGetFromTableEntries(const I18nStringTable *table, const TkString &id, i32 index)
        {
            if (table == nullptr)
                return TkString::Empty;
            auto it = table->entries.find(id);
            if (it == table->entries.End())
                return TkString::Empty;
            return tryGetFromMessages(&it->Value, index);
        }

        TkString tryGetFromTablesDirect(const TkArray<I18nStringTable *> &tables, const TkString &id, i32 index)
        {
            for (i32 i = 0; i < tables.count(); ++i)
            {
                I18nStringTable *table = tables[i];
                if (table == nullptr)
                    continue;

                TkString result = tryGetFromTableEntries(table, id, index);
                if (result.hasChars())
                    return result;
            }
            return TkString::Empty;
        }

        TkString tryGetFromTablesFallbackRefs(const TkArray<I18nStringTable *> &tables, const TkString &id, i32 index)
        {
            for (i32 i = 0; i < tables.count(); ++i)
            {
                I18nStringTable *table = tables[i];
                if (table == nullptr)
                    continue;

                I18nStringTable *fallback = table->fallbackTable;
                TkString result = tryGetFromTableEntries(fallback, id, index);
                if (result.hasChars())
                    return result;
            }
            return TkString::Empty;
        }

        TkString tryGetPluralFromTableEntries(const I18nStringTable *table, const TkString &id, i32 n)
        {
            if (table == nullptr)
                return TkString::Empty;
            auto it = table->entries.find(id);
            if (it == table->entries.End())
                return TkString::Empty;
            return tryGetPluralFromMessages(&it->Value, n);
        }

        TkString tryGetPluralFromTablesDirect(const TkArray<I18nStringTable *> &tables, const TkString &id, i32 n)
        {
            for (i32 i = 0; i < tables.count(); ++i)
            {
                I18nStringTable *table = tables[i];
                if (table == nullptr)
                    continue;

                TkString result = tryGetPluralFromTableEntries(table, id, n);
                if (result.hasChars())
                    return result;
            }
            return TkString::Empty;
        }

        TkString tryGetPluralFromTablesFallbackRefs(const TkArray<I18nStringTable *> &tables, const TkString &id, i32 n)
        {
            for (i32 i = 0; i < tables.count(); ++i)
            {
                I18nStringTable *table = tables[i];
                if (table == nullptr)
                    continue;

                I18nStringTable *fallback = table->fallbackTable;
                TkString result = tryGetPluralFromTableEntries(fallback, id, n);
                if (result.hasChars())
                    return result;
            }
            return TkString::Empty;
        }

        TkString formatPluralText(const TkString &text, i32 n)
        {
            if (!text.hasChars())
                return TkString::Empty;

            if (text.contains("%"))
                return TkString::format(text.getText(), n);

            return text;
        }
    }

    void applyCppLocale(const CultureInfo &culture)
    {
        TkString localeName = culture.getName();
        if (localeName.hasChars())
        {
            localeName.replace("-", "_");
            localeName += ".UTF-8";
        }
        // Use toolkit system layer to apply process locale preference.
        if (!System::setEnvVar(TkStringView("LC_ALL"), localeName))
            LogWarn("Failed to set locale env var LC_ALL to '%s'", localeName.getText());
        if (!System::setEnvVar(TkStringView("LANG"), localeName))
            LogWarn("Failed to set locale env var LANG to '%s'", localeName.getText());
    }

    bool I18nManager::init()
    {
        if (g_initialized)
            return true;

        const TkString userLocale = System::getLocale();
        g_currentCulture = CultureInfo(TkStringView(userLocale));
        g_currentLanguage = g_currentCulture;
        refreshLocalization();
        g_initialized = true;
        return true;
    }

    void I18nManager::update()
    {
    }

    void I18nManager::dispose()
    {
        g_activeTables.clear();
        g_fallbackTables.clear();
        g_tableFallbackLocales.clear();
        g_currentCulture = CultureInfo(0);
        g_currentLanguage = CultureInfo(0);
        g_initialized = false;
    }

    void I18nManager::clearTables()
    {
        s_localeTables.clear();
        g_tableFallbackLocales.clear();
        g_activeTables.clear();
        g_fallbackTables.clear();
        if (g_initialized)
            onLocalizationChanged();
    }

    bool I18nManager::addTable(const I18nStringTable &table, const TkString &fallbackLocale)
    {
        if (!table.locale.hasChars())
            return false;
        s_localeTables.add(table);
        g_tableFallbackLocales.add(fallbackLocale);
        if (g_initialized)
            refreshLocalization();
        return true;
    }

    bool I18nManager::addTable(I18nStringTable &&table, const TkString &fallbackLocale)
    {
        if (!table.locale.hasChars())
            return false;
        s_localeTables.add(moveTemp(table));
        g_tableFallbackLocales.add(fallbackLocale);
        if (g_initialized)
            refreshLocalization();
        return true;
    }

    bool I18nManager::loadTableFromFile(const TkStringView &path)
    {
        FileIStream *stream = FileIStream::open(path);
        if (stream == nullptr)
            return false;

        const bool result = loadTableFromStream(*stream);
        TkMemory::deletePtr(TkMemory::getDefault(), stream);
        return result;
    }

    bool I18nManager::loadTableFromStream(ITkStream &stream)
    {
        I18nStringTable table;
        TkString fallbackLocale;
        if (!table.load(stream, &fallbackLocale))
            return false;
        return addTable(moveTemp(table), fallbackLocale);
    }

    void I18nManager::refreshLocalization()
    {
        g_activeTables.clear();
        g_fallbackTables.clear();

        const TkString en = "en";
        TkHashMap<TkString, TkArray<I18nStringTable *>> localeMapping;
        for (i32 i = 0; i < s_localeTables.count(); ++i)
        {
            I18nStringTable &table = s_localeTables[i];
            table.fallbackTable = nullptr;
            if (!table.locale.hasChars())
                continue;
            localeMapping[table.locale].add(&table);
        }

        const i32 maxFallbackDepth = s_localeTables.count();
        auto wouldCreateFallbackCycle = [maxFallbackDepth](I18nStringTable *source, I18nStringTable *candidate) -> bool
        {
            if (source == nullptr || candidate == nullptr)
                return false;
            if (source == candidate)
                return true;

            I18nStringTable *cursor = candidate;
            for (i32 depth = 0; depth < maxFallbackDepth; ++depth)
            {
                cursor = cursor->fallbackTable;
                if (cursor == nullptr)
                    return false;
                if (cursor == source)
                    return true;
            }
            return true;
        };

        for (i32 i = 0; i < s_localeTables.count(); ++i)
        {
            if (i >= g_tableFallbackLocales.count())
                break;
            const TkString &fallbackLocale = g_tableFallbackLocales[i];
            if (!fallbackLocale.hasChars())
                continue;
            auto it = localeMapping.find(fallbackLocale);
            if (it != localeMapping.End() && it->Value.count() > 0)
            {
                I18nStringTable *candidate = it->Value[0];
                I18nStringTable *source = &s_localeTables[i];
                if (!wouldCreateFallbackCycle(source, candidate))
                {
                    source->fallbackTable = candidate;
                }
                else
                {
            LogWarn("Skip fallback link cycle: '%s' -> '%s'", source->locale.getText(), candidate->locale.getText());
                }
            }
        }

        const TkArray<I18nStringTable *> *selected = findTablesForLocale(localeMapping, g_currentLanguage.getName());
        if (selected == nullptr)
        {
            const CultureInfo parentLanguage(g_currentLanguage.getParentLCID());
            if (parentLanguage.getName().hasChars())
                selected = findTablesForLocale(localeMapping, parentLanguage.getName());
            if (selected == nullptr)
                selected = findTablesForLocale(localeMapping, s_fallbackLang);
            if (selected == nullptr)
                selected = findTablesForLocale(localeMapping, en);
        }

        if (selected != nullptr)
        {
            const TkString selectedLocale = findFirstLocaleKeyByValue(localeMapping, selected);
            LogInfo("Using localization for %s", selectedLocale.getText());
            for (i32 i = 0; i < selected->count(); ++i)
                g_activeTables.add((*selected)[i]);

            if (selectedLocale != s_fallbackLang && selectedLocale != en)
            {
                const TkArray<I18nStringTable *> *fallback = findTablesForLocale(localeMapping, s_fallbackLang);
                if (fallback == nullptr)
                    fallback = findTablesForLocale(localeMapping, en);
                if (fallback != nullptr)
                {
                    for (i32 i = 0; i < fallback->count(); ++i)
                        g_fallbackTables.add((*fallback)[i]);
                }
            }
        }

        if (s_applyCppLocale)
            applyCppLocale(g_currentCulture);
        onLocalizationChanged();
    }

    const CultureInfo &I18nManager::getCurrentCulture()
    {
        return g_currentCulture;
    }

    void I18nManager::setCurrentCulture(const CultureInfo &value)
    {
        if (g_currentCulture == value)
            return;

            LogInfo("Changing current culture to: %s (%d)", value.getName().getText(), value.getLCID());
        g_currentCulture = value;
        refreshLocalization();
    }

    const CultureInfo &I18nManager::getCurrentLanguage()
    {
        return g_currentLanguage;
    }

    void I18nManager::setCurrentLanguage(const CultureInfo &value)
    {
        if (g_currentLanguage == value)
            return;

            LogInfo("Changing current language to: %s (%d)", value.getName().getText(), value.getLCID());
        g_currentLanguage = value;
        refreshLocalization();
    }

    void I18nManager::setCurrentLanguageCulture(const CultureInfo &value)
    {
        if (g_currentCulture == value && g_currentLanguage == value)
            return;

            LogInfo("Changing current language and culture to: %s (%d)", value.getName().getText(), value.getLCID());
        g_currentCulture = value;
        g_currentLanguage = value;
        refreshLocalization();
    }

    TkString I18nManager::getString(const TkString &id, const TkString &fallback)
    {
        if (!g_initialized)
            init();

        if (id.isEmpty())
            return fallback;

        TkString result = tryGetFromTablesDirect(g_activeTables, id, 0);
        if (!result.hasChars())
            result = tryGetFromTablesFallbackRefs(g_activeTables, id, 0);
        if (!result.hasChars())
            result = tryGetFromTablesDirect(g_fallbackTables, id, 0);
        return result.hasChars() ? result : fallback;
    }

    TkString I18nManager::getPluralString(const TkString &id, i32 n, const TkString &fallback)
    {
        if (!g_initialized)
            init();

        if (n < 1)
            return formatPluralText(fallback, n);

        TkString result = tryGetPluralFromTablesDirect(g_activeTables, id, n);
        if (!result.hasChars())
            result = tryGetPluralFromTablesFallbackRefs(g_activeTables, id, n);
        if (!result.hasChars())
            result = tryGetPluralFromTablesDirect(g_fallbackTables, id, n);
        if (!result.hasChars())
            result = fallback;
        return formatPluralText(result, n);
    }
}
