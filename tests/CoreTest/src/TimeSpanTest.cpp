/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"
#include "toolkit/types/TimeSpan.hpp"

using namespace toolkit;

TEST(TkTimeSpan, ConstructorAndGetters)
{
    TkTimeSpan t1;  
    EXPECT_EQ(t1.ticks, 0);

    TkTimeSpan t2(1); 
    EXPECT_EQ(t2.ticks, 1);

    TkTimeSpan t3(1, 2, 3); // days, hours, minutes
    EXPECT_EQ(t3.getDays(), 1);
    EXPECT_EQ(t3.getHours(), 2);
    EXPECT_EQ(t3.getMinutes(), 3);
    EXPECT_EQ(t3.getSeconds(), 0);
    EXPECT_EQ(t3.getMilliseconds(), 0);

    TkTimeSpan t4(1, 2, 3, 4, 500); // full constructor
    EXPECT_EQ(t4.getDays(), 1);
    EXPECT_EQ(t4.getHours(), 2);
    EXPECT_EQ(t4.getMinutes(), 3);
    EXPECT_EQ(t4.getSeconds(), 4);
    EXPECT_EQ(t4.getMilliseconds(), 500);
}

TEST(TkTimeSpan, FactoryMethods)
{
    auto t1 = TkTimeSpan::fromDays(1.5);
    EXPECT_DOUBLE_EQ(t1.getTotalDays(), 1.5);

    auto t2 = TkTimeSpan::fromHours(36.0);
    EXPECT_DOUBLE_EQ(t2.getTotalHours(), 36.0);

    auto t3 = TkTimeSpan::fromMinutes(90.0);
    EXPECT_DOUBLE_EQ(t3.getTotalMinutes(), 90.0);

    auto t4 = TkTimeSpan::fromSeconds(3661);
    EXPECT_FLOAT_EQ(t4.getTotalSeconds(), 3661.0f);

    auto t5 = TkTimeSpan::fromMilliseconds(12345);
    EXPECT_DOUBLE_EQ(t5.getTotalMilliseconds(), 12345.0);
}

TEST(TkTimeSpan, Operators)
{
    TkTimeSpan t1(1000000);
    TkTimeSpan t2(2000000);

    auto tAdd = t1 + t2;
    EXPECT_EQ(tAdd.ticks, 3000000);

    t1 += t2;
    EXPECT_EQ(t1.ticks, 3000000);

    auto tSub = t1 - t2;
    EXPECT_EQ(tSub.ticks, 1000000);

    t1 -= t2;
    EXPECT_EQ(t1.ticks, 1000000);

    auto tNeg = -t1;
    EXPECT_EQ(tNeg.ticks, -1000000);

    auto tMul = t1 * 2.5f;
    EXPECT_EQ(tMul.ticks, 2500000);

    t1 *= 3.0f;
    EXPECT_EQ(t1.ticks, 3000000);

    EXPECT_TRUE(t1 > t2);
    EXPECT_TRUE(t2 < t1);
    EXPECT_TRUE(t1 >= t2);
    EXPECT_TRUE(t2 <= t1);
    EXPECT_TRUE(t1 != t2);
    EXPECT_FALSE(t1 == t2);
}

TEST(TkTimeSpan, Duration)
{
    TkTimeSpan t1(-1000000);
    auto tAbs = t1.getDuration();
    EXPECT_EQ(tAbs.ticks, 1000000);

    TkTimeSpan t2(5000000);
    EXPECT_EQ(t2.getDuration().ticks, 5000000);
}

TEST(TkTimeSpan, ToString)
{
    TkTimeSpan t(1, 2, 3, 4, 567);
    
    auto s1 = t.toString();
    EXPECT_FALSE(s1.isEmpty());

    auto s2 = t.toString('a');
    EXPECT_FALSE(s2.isEmpty());

    // Check formatting contains hours/minutes/seconds/milliseconds
    EXPECT_NE(s2.find("02:03:04"), std::string::npos);
}

TEST(TkTimeSpan, MaxMinZero)
{
    auto tMax = TkTimeSpan::maxValue();
    EXPECT_EQ(tMax.ticks, 9223372036854775807);

    auto tMin = TkTimeSpan::minValue();
    EXPECT_EQ(tMin.ticks, -9223372036854775807 - 1);

    auto tZero = TkTimeSpan::zero();
    EXPECT_EQ(tZero.ticks, 0);
}

TEST(TkTimeSpan, TotalProperties)
{
    TkTimeSpan t(1, 2, 3, 4, 500);

    EXPECT_DOUBLE_EQ(t.getTotalDays(), t.ticks / (double)TkTimeSpan::TicksPerDay);
    EXPECT_DOUBLE_EQ(t.getTotalHours(), t.ticks / (double)TkTimeSpan::TicksPerHour);
    EXPECT_DOUBLE_EQ(t.getTotalMinutes(), t.ticks / (double)TkTimeSpan::TicksPerMinute);
    EXPECT_DOUBLE_EQ(t.getTotalMilliseconds(), t.ticks / (double)TkTimeSpan::TicksPerMillisecond);
    EXPECT_FLOAT_EQ(t.getTotalSeconds(), t.ticks / (float)TkTimeSpan::TicksPerSecond);
}
