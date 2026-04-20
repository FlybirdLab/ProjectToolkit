/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LocaleTestUtils.hpp"

using namespace locale_test;

TEST(CultureInfoTest, ConstructFromLcidAndName)
{
    CultureInfo byId(127);
    EXPECT_EQ(byId.getLCID(), 127);
    EXPECT_EQ(byId.getParentLCID(), 0);
    EXPECT_EQ(byId.getEnglishName(), "Invariant Culture");

    CultureInfo enByName(TkStringView("en"));
    EXPECT_TRUE(enByName.getName().hasChars());
    EXPECT_TRUE(enByName.getNativeName().hasChars());
    EXPECT_EQ(enByName.toString(), enByName.getName());
    EXPECT_EQ(enByName, CultureInfo(TkStringView("en")));
}

TEST(CultureInfoTest, ParentCultureAndFallbackName)
{
    CultureInfo gb(TkStringView("en-GB"));
    EXPECT_TRUE(gb.getName().hasChars());
    EXPECT_EQ(gb.getParentLCID(), CultureInfo(TkStringView("en")).getLCID());
    EXPECT_EQ(gb.toString(), "en-GB");
}

TEST(CultureInfoTest, UnknownCultureFallsBackToInvariant)
{
    CultureInfo unknown(TkStringView("unknown-locale"));
    EXPECT_EQ(unknown.getLCID(), 127);
    EXPECT_EQ(unknown.getEnglishName(), "Invariant Culture");
    EXPECT_TRUE(unknown.getName().isEmpty());
}
