/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LocaleTestUtils.hpp"

using namespace locale_test;

namespace
{
    class I18nStringTest : public LocaleTestBase
    {
    };
}

TEST_F(I18nStringTest, ConstructorsAndAssignments)
{
    I18nString empty;
    EXPECT_TRUE(empty.id.isEmpty());
    EXPECT_TRUE(empty.value.isEmpty());

    I18nString text(TkStringView("fallback"));
    EXPECT_TRUE(text.id.isEmpty());
    EXPECT_EQ(text.value, "fallback");

    I18nString moved(TkString("move-me"));
    EXPECT_EQ(moved.value, "move-me");

    I18nString copy = moved;
    EXPECT_EQ(copy, moved);

    I18nString assigned;
    assigned = TkStringView("assigned");
    EXPECT_TRUE(assigned.id.isEmpty());
    EXPECT_EQ(assigned.value, "assigned");

    assigned.id = "key";
    assigned.value = "value";
    I18nString assignedCopy;
    assignedCopy = assigned;
    EXPECT_EQ(assignedCopy.id, "key");
    EXPECT_EQ(assignedCopy.value, "value");

    I18nString moveAssigned;
    moveAssigned = std::move(assignedCopy);
    EXPECT_EQ(moveAssigned.id, "key");
    EXPECT_EQ(moveAssigned.value, "value");
}

TEST_F(I18nStringTest, ComparisonOperators)
{
    I18nString a;
    a.id = "hello";
    a.value = "Hello";

    I18nString b = a;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a == TkStringView("Hello"));
    EXPECT_FALSE(a == TkStringView("World"));
}

TEST_F(I18nStringTest, ToStringAndPluralDelegateToManager)
{
    ASSERT_TRUE(I18nManager::init());
    I18nManager::clearTables();
    ASSERT_TRUE(I18nManager::addTable(makeEnglishTable()));
    I18nManager::setCurrentLanguageCulture(CultureInfo(TkStringView("en")));

    I18nString stringValue;
    stringValue.id = "hello";
    stringValue.value = "fallback";
    EXPECT_EQ(stringValue.toString(), "Hello");

    I18nString pluralValue;
    pluralValue.id = "apples";
    pluralValue.value = "%d apple";
    EXPECT_EQ(pluralValue.toStringPlural(2), "2 apples");
}

TEST_F(I18nStringTest, HashAndFormatUseResolvedString)
{
    I18nString stringValue;
    stringValue.id = "hello";
    stringValue.value = "fallback";

    const u32 hash = HashFunc<I18nString>::get(stringValue);
    EXPECT_NE(hash, 0u);
    EXPECT_STREQ(TkString::format("%s", stringValue).getText(), "fallback");
}
