/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "DateTime.hpp"

#include <toolkit/base/Math.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/platforms/System.hpp>

namespace toolkit {
    
    const i32 CachedDaysPerMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    const i32 CachedDaysToMonth[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

    TkDateTime::TkDateTime(i32 year, i32 month, i32 day, i32 hour, i32 minute, i32 second, i32 millisecond)
    {
        TK_ASSERT(validate(year, month, day, hour, minute, second, millisecond));
        i32 daysSum = 0;
        if (month > 2 && isLeapYear(year))
            daysSum++;
        year--;
        month--;
        daysSum += year * 365 + year / 4 - year / 100 + year / 400 + CachedDaysToMonth[month] + day - 1;
        ticks = daysSum * TkTimeSpan::TicksPerDay
            + hour * TkTimeSpan::TicksPerHour
            + minute * TkTimeSpan::TicksPerMinute
            + second * TkTimeSpan::TicksPerSecond
            + millisecond * TkTimeSpan::TicksPerMillisecond;
    }


    TkDateTime TkDateTime::getDate() const
    {
        return TkDateTime(ticks - ticks % TkTimeSpan::TicksPerDay);
    }


    void TkDateTime::getDate(i32& year, i32& month, i32& day) const
    {
        // Based on:
        // Fliegel, H. F. and van Flandern, T. C.,
        // Communications of the ACM, Vol. 11, No. 10 (October 1968).
        i32 l = TkMath::floorToInt((float)(getDate().getJulianDay() + 0.5)) + 68569;
        const i32 n = 4 * l / 146097;
        l = l - (146097 * n + 3) / 4;
        i32 i = 4000 * (l + 1) / 1461001;
        l = l - 1461 * i / 4 + 31;
        i32 j = 80 * l / 2447;
        const i32 k = l - 2447 * j / 80;
        l = j / 11;
        j = j + 2 - 12 * l;
        i = 100 * (n - 49) + i + l;
        year = i;
        month = j;
        day = k;
    }


    i32 TkDateTime::getDay() const
    {
        i32 year, month, day;
        getDate(year, month, day);
        return day;
    }


    DayOfWeek TkDateTime::getDayOfWeek() const
    {
        return static_cast<DayOfWeek>((ticks / TkTimeSpan::TicksPerDay) % 7);
    }


    i32 TkDateTime::getDayOfYear() const
    {
        i32 year, month, day;
        getDate(year, month, day);
        for (i32 i = 1; i < month; i++)
            day += daysInMonth(year, i);
        return day;
    }


    i32 TkDateTime::getHour() const
    {
        return static_cast<i32>(ticks / TkTimeSpan::TicksPerHour % 24);
    }


    i32 TkDateTime::getHour12() const
    {
        const i32 hour = getHour();
        if (hour < 1)
            return 12;
        if (hour > 12)
            return hour - 12;
        return hour;
    }


    double TkDateTime::getJulianDay() const
    {
        return 1721425.5 + static_cast<double>(ticks) / TkTimeSpan::TicksPerDay;
    }


    double TkDateTime::getModifiedJulianDay() const
    {
        return getJulianDay() - 2400000.5;
    }


    i32 TkDateTime::getMillisecond() const
    {
        return static_cast<i32>(ticks / TkTimeSpan::TicksPerMillisecond % 1000);
    }

    i32 TkDateTime::getMinute() const
    {
        return static_cast<i32>(ticks / TkTimeSpan::TicksPerMinute % 60);
    }

    i32 TkDateTime::getMonth() const
    {
        i32 year, month, day;
        getDate(year, month, day);
        return month;
    }

    MonthOfYear TkDateTime::getMonthOfYear() const
    {
        return static_cast<MonthOfYear>(getMonth());
    }

    i32 TkDateTime::getSecond() const
    {
        return static_cast<i32>(ticks / TkTimeSpan::TicksPerSecond % 60);
    }

    TkTimeSpan TkDateTime::getTimeOfDay() const
    {
        return TkTimeSpan(ticks % TkTimeSpan::TicksPerDay);
    }

    i32 TkDateTime::getYear() const
    {
        i32 year, month, day;
        getDate(year, month, day);
        return year;
    }

    i32 TkDateTime::daysInMonth(i32 year, i32 month)
    {
        TK_ASSERT((month >= 1) && (month <= 12));
        if (month == 2 && isLeapYear(year))
            return 29;
        return CachedDaysPerMonth[month];
    }

    i32 TkDateTime::daysInYear(i32 year)
    {
        return isLeapYear(year) ? 366 : 365;
    }

    bool TkDateTime::isLeapYear(i32 year)
    {
        if ((year % 4) == 0)
        {
            return (((year % 100) != 0) || ((year % 400) == 0));
        }
        return false;
    }

    TkDateTime TkDateTime::maxValue()
    {
        return TkDateTime(3652059 * TkTimeSpan::TicksPerDay - 1);
    }

    TkDateTime TkDateTime::now()
    {
        System::Time time {};
        System::getSystemTime(time);
        return TkDateTime(time.year, time.month, time.day, time.hour, time.minute, time.second, time.millisecond);
    }

    TkDateTime TkDateTime::nowUTC()
    {
        System::Time time {};
        System::getUTCTime(time);
        return TkDateTime(time.year, time.month, time.day, time.hour, time.minute, time.second, time.millisecond);
    }

    bool TkDateTime::validate(i32 year, i32 month, i32 day, i32 hour, i32 minute, i32 second, i32 millisecond)
    {
        return year >= 1 && year <= 999999 && month >= 1 && month <= 12 && day >= 1 && day <= daysInMonth(year, month) && hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59 && millisecond >= 0 && millisecond <= 999;
    }


    TkString TkDateTime::toString() const
    {
        return TkString::format("{}", *this);
    }

    TkString TkDateTime::toFileNameString() const
    {
        i32 year, month, day;
        getDate(year, month, day);
        return TkString::format("%s_%02d_%02d_%02d_%02d_%02d", year, month, day, getHour(), getMinute(), getSecond());
    }

    TkDateTime TkDateTime::operator+(const TkTimeSpan& other) const
    {
        return TkDateTime(ticks + other.ticks);
    }

    TkDateTime& TkDateTime::operator+=(const TkTimeSpan& other)
    {
        ticks += other.ticks;
        return *this;
    }

    TkTimeSpan TkDateTime::operator-(const TkDateTime& other) const
    {
        return TkTimeSpan(ticks - other.ticks);
    }

    TkDateTime TkDateTime::operator-(const TkTimeSpan& other) const
    {
        return TkDateTime(ticks - other.ticks);
    }

    TkDateTime& TkDateTime::operator-=(const TkTimeSpan& other)
    {
        ticks -= other.ticks;
        return *this;
    }
}