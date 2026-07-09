/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LocaleTestUtils.hpp"

#include "toolkit/base/Function.hpp"

using namespace locale_test;

namespace
{
    class I18nManagerTest : public LocaleTestBase
    {
    };
}

TEST_F(I18nManagerTest, InitAndDispose)
{
    EXPECT_TRUE(I18nManager::init());
    EXPECT_TRUE(I18nManager::getCurrentCulture().getName().hasChars() || I18nManager::getCurrentCulture().getLCID() == 127);
    I18nManager::update();
    I18nManager::dispose();
    EXPECT_EQ(I18nManager::s_localeTables.count(), 0);
}

TEST_F(I18nManagerTest, AddInvalidTableFails)
{
    ASSERT_TRUE(I18nManager::init());
    I18nStringTable invalid;
    EXPECT_FALSE(I18nManager::addTable(invalid));
    EXPECT_EQ(I18nManager::s_localeTables.count(), 0);
}

TEST_F(I18nManagerTest, AddTablesAndQueryStrings)
{
    ASSERT_TRUE(I18nManager::init());
    ASSERT_TRUE(I18nManager::addTable(makeEnglishTable()));
    ASSERT_TRUE(I18nManager::addTable(makeBritishTable(), "en"));

    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en-GB")));
    EXPECT_EQ(I18nManager::getCurrentLanguage().getName(), "en-GB");
    EXPECT_EQ(I18nManager::getCurrentCulture().getName(), "en-GB");
    EXPECT_EQ(I18nManager::getString("hello"), "Hello");
    EXPECT_EQ(I18nManager::getString("colour"), "Colour");
    EXPECT_EQ(I18nManager::getString("missing", "fallback"), "fallback");
    EXPECT_EQ(I18nManager::getPluralString("apples", 2), "2 apples");
}

TEST_F(I18nManagerTest, SetCurrentCultureAndLanguageIndividually)
{
    ASSERT_TRUE(I18nManager::init());
    ASSERT_TRUE(I18nManager::addTable(makeEnglishTable()));

    I18nManager::setCurrentCulture(CultureInfo(TkStringView("en")));
    EXPECT_EQ(I18nManager::getCurrentCulture().getName(), "en");
    I18nManager::setCurrentLanguage(CultureInfo(TkStringView("en")));
    EXPECT_EQ(I18nManager::getCurrentLanguage().getName(), "en");
}

TEST_F(I18nManagerTest, LoadFromDocFileAndStream)
{
    ASSERT_TRUE(I18nManager::init());
    I18nManager::clearTables();

    ASSERT_TRUE(I18nManager::loadTableFromFile(docPath("i18n_en_v1.tla")));
    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en")));
    EXPECT_EQ(I18nManager::getString("hello"), "Hello");

    I18nManager::clearTables();

    FileIStream *stream = FileIStream::open(docPath("i18n_en_v1.tla"));
    ASSERT_NE(stream, nullptr);
    ASSERT_TRUE(I18nManager::loadTableFromStream(*stream));
    TkMemory::deletePtr(TkMemory::getDefault(), stream);

    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en")));
    EXPECT_EQ(I18nManager::getPluralString("apples", 2), "2 apples");
}

TEST_F(I18nManagerTest, LoadBritishDocSampleAndResolveFallback)
{
    ASSERT_TRUE(I18nManager::init());
    I18nManager::clearTables();

    ASSERT_TRUE(I18nManager::loadTableFromFile(docPath("i18n_en_gb_v1.tla")));
    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en-GB")));

    EXPECT_EQ(I18nManager::getString("hello"), "Hallo");
    EXPECT_EQ(I18nManager::getString("missing", "fallback"), "fallback");
}

TEST_F(I18nManagerTest, LoadMissingFileFails)
{
    ASSERT_TRUE(I18nManager::init());
    I18nManager::clearTables();

    EXPECT_FALSE(I18nManager::loadTableFromFile(docPath("missing_locale_file.tla")));
    EXPECT_EQ(I18nManager::s_localeTables.count(), 0);
}

TEST_F(I18nManagerTest, ClearTablesResetsLookups)
{
    ASSERT_TRUE(I18nManager::init());
    ASSERT_TRUE(I18nManager::addTable(makeEnglishTable()));
    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en")));
    EXPECT_EQ(I18nManager::getString("hello"), "Hello");

    I18nManager::clearTables();
    EXPECT_EQ(I18nManager::getString("hello", "fallback"), "fallback");
    EXPECT_EQ(I18nManager::s_localeTables.count(), 0);
}

TEST_F(I18nManagerTest, FallbackCycleDoesNotLoop)
{
    ASSERT_TRUE(I18nManager::init());
    ASSERT_TRUE(I18nManager::addTable(makeCycleSourceTable(), "en"));
    ASSERT_TRUE(I18nManager::addTable(makeCycleTargetTable(), "en-US"));

    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en-US")));
    EXPECT_EQ(I18nManager::getString("cycle-key"), "cycle-value-from-b");
}

TEST_F(I18nManagerTest, LoadInvalidFileFails)
{
    ASSERT_TRUE(I18nManager::init());
    I18nManager::clearTables();
    EXPECT_FALSE(I18nManager::loadTableFromFile(docPath("i18n_invalid_magic.tla")));
    EXPECT_EQ(I18nManager::s_localeTables.count(), 0);
}

TEST_F(I18nManagerTest, StringTableIntegrationWithFallbackOnlyEntry)
{
    ASSERT_TRUE(I18nManager::init());
    ASSERT_TRUE(I18nManager::addTable(makeFallbackTable()));
    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en")));
    EXPECT_EQ(I18nManager::getString("fallback-only"), "Fallback text");
}

TEST_F(I18nManagerTest, FallbackLanguageIsUsedWhenCurrentLanguageIsMissing)
{
    ASSERT_TRUE(I18nManager::init());
    I18nManager::clearTables();

    ASSERT_TRUE(I18nManager::addTable(makeEnglishTable()));
    I18nManager::setCurrentLanguage(CultureInfo(TkStringView("zz-ZZ")));
    I18nManager::setCurrentCulture(CultureInfo(TkStringView("zz-ZZ")));

    EXPECT_EQ(I18nManager::getString("hello"), "Hello");
    EXPECT_EQ(I18nManager::getPluralString("apples", 2), "2 apples");
}

TEST_F(I18nManagerTest, SetCurrentLanguageAndCultureSeparatelyRefreshesLookups)
{
    ASSERT_TRUE(I18nManager::init());
    I18nManager::clearTables();

    I18nStringTable en = makeEnglishTable();
    I18nStringTable gb = makeBritishTable();

    ASSERT_TRUE(I18nManager::addTable(en));
    ASSERT_TRUE(I18nManager::addTable(gb, "en"));

    I18nManager::setCurrentLanguage(CultureInfo(TkStringView("en")));
    I18nManager::setCurrentCulture(CultureInfo(TkStringView("en-GB")));

    EXPECT_EQ(I18nManager::getCurrentLanguage().getName(), "en");
    EXPECT_EQ(I18nManager::getCurrentCulture().getName(), "en-GB");
    EXPECT_EQ(I18nManager::getString("hello"), "Hello");
    EXPECT_EQ(I18nManager::getString("colour", "fallback"), "fallback");
}

TEST_F(I18nManagerTest, LocalizationChangedSignalIsCallable)
{
    ASSERT_TRUE(I18nManager::init());
    bool called = false;
    struct Handler
    {
        bool *called = nullptr;
        void onChanged()
        {
            *called = true;
        }
    } handler{&called};

    I18nManager::onLocalizationChanged.bind<Handler, &Handler::onChanged>(&handler);

    ASSERT_TRUE(I18nManager::addTable(makeEnglishTable()));
    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en")));

    EXPECT_TRUE(called);
    I18nManager::onLocalizationChanged.unbind<Handler, &Handler::onChanged>(&handler);
}

TEST_F(I18nManagerTest, ClearTablesKeepsManagerUsable)
{
    ASSERT_TRUE(I18nManager::init());
    ASSERT_TRUE(I18nManager::addTable(makeEnglishTable()));
    I18nManager::clearTables();

    ASSERT_TRUE(I18nManager::addTable(makeEnglishTable()));
    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en")));
    EXPECT_EQ(I18nManager::getString("hello"), "Hello");
}
