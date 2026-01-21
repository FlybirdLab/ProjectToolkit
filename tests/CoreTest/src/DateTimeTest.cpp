/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include "toolkit/types/DateTime.hpp"

TEST(DateTime, ConstructAndGetters)
{
    TkDateTime t(2024, 3, 15, 13, 45, 30, 123);

    EXPECT_EQ(t.getYear(), 2024);
    EXPECT_EQ(t.getMonth(), 3);
    EXPECT_EQ(t.getDay(), 15);
    EXPECT_EQ(t.getHour(), 13);
    EXPECT_EQ(t.getMinute(), 45);
    EXPECT_EQ(t.getSecond(), 30);
    EXPECT_EQ(t.getMillisecond(), 123);
}

TEST(DateTime, LeapYear)
{
    EXPECT_TRUE(TkDateTime::isLeapYear(2000));
    EXPECT_FALSE(TkDateTime::isLeapYear(1900));
    EXPECT_TRUE(TkDateTime::isLeapYear(2024));
    EXPECT_FALSE(TkDateTime::isLeapYear(2023));
}

TEST(DateTime, DaysInMonth)
{
    EXPECT_EQ(TkDateTime::daysInMonth(2024, 2), 29);
    EXPECT_EQ(TkDateTime::daysInMonth(2023, 2), 28);
    EXPECT_EQ(TkDateTime::daysInMonth(2023, 1), 31);
    EXPECT_EQ(TkDateTime::daysInMonth(2023, 4), 30);
}

TEST(DateTime, DaysInYear)
{
    EXPECT_EQ(TkDateTime::daysInYear(2024), 366);
    EXPECT_EQ(TkDateTime::daysInYear(2023), 365);
}

TEST(DateTime, DayOfYear)
{
    EXPECT_EQ(TkDateTime(2023, 1, 1).getDayOfYear(), 1);
    EXPECT_EQ(TkDateTime(2023, 12, 31).getDayOfYear(), 365);

    EXPECT_EQ(TkDateTime(2024, 12, 31).getDayOfYear(), 366);
    EXPECT_EQ(TkDateTime(2024, 3, 1).getDayOfYear(), 61);
}

TEST(DateTime, DayOfWeek)
{
    TkDateTime t(2024, 1, 1);
    EXPECT_EQ(t.getDayOfWeek(), DayOfWeek::kMonday);

    EXPECT_EQ(TkDateTime(2024, 1, 7).getDayOfWeek(), DayOfWeek::kSunday);
    EXPECT_EQ(TkDateTime(2024, 1, 8).getDayOfWeek(), DayOfWeek::kMonday);
}

TEST(DateTime, AddSubtractTimeSpan)
{
    TkDateTime t(2024, 3, 15, 23, 59, 59);
    TkTimeSpan oneSecond = TkTimeSpan::fromSeconds(1);

    auto next = t + oneSecond;
    EXPECT_EQ(next.getDay(), 16);
    EXPECT_EQ(next.getHour(), 0);
    EXPECT_EQ(next.getMinute(), 0);
    EXPECT_EQ(next.getSecond(), 0);
}

TEST(DateTime, DateDifference)
{
    TkDateTime a(2024, 3, 15);
    TkDateTime b(2024, 3, 16);

    TkTimeSpan diff = b - a;
    EXPECT_EQ(diff.getTotalDays(), 1);
}

TEST(DateTime, MinMax)
{
    auto min = TkDateTime::minValue();
    EXPECT_EQ(min.getYear(), 1);
    EXPECT_EQ(min.getMonth(), 1);
    EXPECT_EQ(min.getDay(), 1);

    auto max = TkDateTime::maxValue();
    EXPECT_GT(max.getTicks(), min.getTicks());
}

TEST(DateTime, StringFormat)
{
    TkDateTime t(2024, 3, 15, 9, 8, 7);

    EXPECT_EQ(t.toFileNameString(), "2024_03_15_09_08_07");
    EXPECT_FALSE(t.toString().isEmpty());
}

TEST(DateTime, Validate)
{
    EXPECT_TRUE(TkDateTime::validate(2024, 2, 29, 23, 59, 59, 999));
    EXPECT_FALSE(TkDateTime::validate(2023, 2, 29, 0, 0, 0, 0));
    EXPECT_FALSE(TkDateTime::validate(2023, 13, 1, 0, 0, 0, 0));
}
