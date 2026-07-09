/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target: DateTimeTest.cpp
 */

#pragma once

#include <toolkit/base/Enum.hpp>
#include <toolkit/types/TimeSpan.hpp>

namespace toolkit
{

    // The days of the week
    TK_DECLARE_ENUM_EX_7(DayOfWeek, i32, 0, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday);
    // The months of the year
    TK_DECLARE_ENUM_EX_12(MonthOfYear, i32, 1, January, February, March, April, May, June, July, August, September, October, November, December);

    // Represents date and time
    class TkDateTime final
    {
    public:
        TkDateTime() = default;
        TkDateTime(i64 ticks)
            : ticks(ticks)
        {
        }
        TkDateTime(i32 year, i32 month, i32 day, i32 hour = 0, i32 minute = 0, i32 second = 0, i32 millisecond = 0);

    public:
        // Gets the string.
        TkString toString() const;
        // Gets the string that is valid for filename.
        TkString toFileNameString() const;

    public:
        TkDateTime operator+(const TkTimeSpan &other) const;
        TkDateTime &operator+=(const TkTimeSpan &other);
        TkTimeSpan operator-(const TkDateTime &other) const;
        TkDateTime operator-(const TkTimeSpan &other) const;
        TkDateTime &operator-=(const TkTimeSpan &other);

        TK_FORCE_INLINE bool operator==(const TkDateTime &other) const
        {
            return ticks == other.ticks;
        }
        TK_FORCE_INLINE bool operator!=(const TkDateTime &other) const
        {
            return ticks != other.ticks;
        }
        TK_FORCE_INLINE bool operator>(const TkDateTime &other) const
        {
            return ticks > other.ticks;
        }
        TK_FORCE_INLINE bool operator>=(const TkDateTime &other) const
        {
            return ticks >= other.ticks;
        }
        TK_FORCE_INLINE bool operator<(const TkDateTime &other) const
        {
            return ticks < other.ticks;
        }
        TK_FORCE_INLINE bool operator<=(const TkDateTime &other) const
        {
            return ticks <= other.ticks;
        }

    public:
        TkDateTime getDate() const;
        void getDate(i32 &year, i32 &month, i32 &day) const;
        // Gets this date's day part (1 to 31).
        i32 getDay() const;
        // Calculates this date's day of the week (Sunday - Saturday).
        DayOfWeek getDayOfWeek() const;
        // Gets this date's day of the year.
        i32 getDayOfYear() const;
        // Gets this date's hour part in 24-hour clock format (0 to 23).
        i32 getHour() const;
        // Gets this date's hour part in 12-hour clock format (1 to 12).
        i32 getHour12() const;
        // Gets the Julian Day for this date.
        double getJulianDay() const;
        // Gets the Modified Julian day.
        double getModifiedJulianDay() const;
        // Gets this date's millisecond part (0 to 999).
        i32 getMillisecond() const;
        // Gets this date's minute part (0 to 59).
        i32 getMinute() const;
        // Gets this date's the month part (1 to 12).
        i32 getMonth() const;
        // Gets the date's month of the year (January to December).
        MonthOfYear getMonthOfYear() const;
        // Gets this date's second part.
        i32 getSecond() const;
        // Gets this date's representation as number of ticks since midnight, January 1, 0001.
        i64 getTicks() const
        {
            return ticks;
        }
        // Gets the time elapsed since midnight of this date.
        TkTimeSpan getTimeOfDay() const;
        // Gets this date's year part.
        i32 getYear() const;

    public:
        // Gets the number of days in the year and month.
        static i32 daysInMonth(i32 year, i32 month);
        // Gets the number of days in the given year.
        static i32 daysInYear(i32 year);
        // Determines whether the specified year is a leap year.
        static bool isLeapYear(i32 year);
        // Returns the maximum date value.
        static TkDateTime maxValue();
        // Returns the minimum date value.
        static TkDateTime minValue()
        {
            return TkDateTime(1, 1, 1, 0, 0, 0, 0);
        }
        // Gets the local date and time on this computer.
        static TkDateTime now();
        // Gets the UTC date and time on this computer.
        static TkDateTime nowUTC();
        // Validates the given components of a date and time value.
        static bool validate(i32 year, i32 month, i32 day, i32 hour, i32 minute, i32 second, i32 millisecond);

    public:
        // ticks in 100 nanoseconds resolution since January 1, 0001 A.D.
        i64 ticks;
    };

    template <>
    struct TIsPODType<TkDateTime>
    {
        enum
        {
            Value = true
        };
    };

}

namespace toolkit
{
    template <>
    inline psize formatCustom<>(FormatBuffer &buf, const TkDateTime &t)
    {
        i32 year, month, day;
        t.getDate(year, month, day);
        return Formatter::formatRaw(buf, "%04d_%02d_%02d_%02d_%02d_%02d", year, month, day, t.getHour(), t.getMinute(), t.getSecond());
    }
};