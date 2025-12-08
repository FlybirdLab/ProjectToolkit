// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/base/Format.hpp>

namespace toolkit
{
	// Represents the difference between two dates and times
	class TTimeSpan
	{
	public:
		// The number of timespan ticks per day.
		static constexpr i64 TicksPerDay = 864000000000;
		// The number of timespan ticks per hour.
		static constexpr i64 TicksPerHour = 36000000000;
		// The number of timespan ticks per millisecond.
		static constexpr i64 TicksPerMillisecond = 10000;
		// The number of timespan ticks per minute.
		static constexpr i64 TicksPerMinute = 600000000;
		// The number of timespan ticks per second.
		static constexpr i64 TicksPerSecond = 10000000;
		// The number of timespan ticks per week.
		static constexpr i64 TicksPerWeek = 6048000000000;

	public:
		// Time span in 100 nanoseconds resolution.
		i64 ticks;

	public:
		TTimeSpan() = default;
		TTimeSpan(i64 ticks)
			: ticks(ticks)
		{
		}
		TTimeSpan(i32 days, i32 hours, i32 minutes)
		{
			set(days, hours, minutes, 0, 0);
		}
		TTimeSpan(i32 days, i32 hours, i32 minutes, i32 seconds)
		{
			set(days, hours, minutes, seconds, 0);
		}
		TTimeSpan(i32 days, i32 hours, i32 minutes, i32 seconds, i32 milliseconds)
		{
			set(days, hours, minutes, seconds, milliseconds);
		}

	public:
		// Get string
		TString toString() const;
		// Get string
		TString toString(const char option) const;

	public:
		TK_FORCE_INLINE TTimeSpan operator+(const TTimeSpan& other) const
		{
			return TTimeSpan(ticks + other.ticks);
		}
		TK_FORCE_INLINE TTimeSpan& operator+=(const TTimeSpan& other)
		{
			ticks += other.ticks;
			return *this;
		}
		TK_FORCE_INLINE TTimeSpan operator-() const
		{
			return TTimeSpan(-ticks);
		}
		TK_FORCE_INLINE TTimeSpan operator-(const TTimeSpan& other) const
		{
			return TTimeSpan(ticks - other.ticks);
		}
		TK_FORCE_INLINE TTimeSpan& operator-=(const TTimeSpan& other)
		{
			ticks -= other.ticks;
			return *this;
		}
		TK_FORCE_INLINE TTimeSpan operator*(float scalar) const
		{
			return TTimeSpan((i64)((float)ticks * scalar));
		}
		TK_FORCE_INLINE TTimeSpan& operator*=(float scalar)
		{
			ticks = (i64)((float)ticks * scalar);
			return *this;
		}
		TK_FORCE_INLINE bool operator==(const TTimeSpan& other) const
		{
			return ticks == other.ticks;
		}
		TK_FORCE_INLINE bool operator!=(const TTimeSpan& other) const
		{
			return ticks != other.ticks;
		}
		TK_FORCE_INLINE bool operator>(const TTimeSpan& other) const
		{
			return ticks > other.ticks;
		}
		TK_FORCE_INLINE bool operator>=(const TTimeSpan& other) const
		{
			return ticks >= other.ticks;
		}
		TK_FORCE_INLINE bool operator<(const TTimeSpan& other) const
		{
			return ticks < other.ticks;
		}
		TK_FORCE_INLINE bool operator<=(const TTimeSpan& other) const
		{
			return ticks <= other.ticks;
		}

	public:
		// Gets the days component of this time span.
		TK_FORCE_INLINE i32 getDays() const
		{
			return (i32)(ticks / TicksPerDay);
		}

		// Returns a time span with the absolute value of this time span.
		TK_FORCE_INLINE TTimeSpan getDuration() const
		{
			return TTimeSpan(ticks >= 0 ? ticks : -ticks);
		}

		// Gets the hours component of this time span.
		TK_FORCE_INLINE i32 getHours() const
		{
			return (i32)(ticks / TicksPerHour % 24);
		}

		// Gets the milliseconds component of this time span.
		TK_FORCE_INLINE i32 getMilliseconds() const
		{
			return (i32)(ticks / TicksPerMillisecond % 1000);
		}

		// Gets the minutes component of this time span.
		TK_FORCE_INLINE i32 getMinutes() const
		{
			return (i32)(ticks / TicksPerMinute % 60);
		}

		// Gets the seconds component of this time span.
		TK_FORCE_INLINE i32 getSeconds() const
		{
			return (i32)(ticks / TicksPerSecond % 60);
		}

		// Gets the total number of days represented by this time span.
		TK_FORCE_INLINE double getTotalDays() const
		{
			return (double)ticks / TicksPerDay;
		}

		// Gets the total number of hours represented by this time span.
		TK_FORCE_INLINE double getTotalHours() const
		{
			return (double)ticks / TicksPerHour;
		}

		// Gets the total number of milliseconds represented by this time span.
		TK_FORCE_INLINE double getTotalMilliseconds() const
		{
			return (double)ticks / TicksPerMillisecond;
		}

		// Gets the total number of minutes represented by this time span.
		TK_FORCE_INLINE double getTotalMinutes() const
		{
			return (double)ticks / TicksPerMinute;
		}

		// Gets the total number of seconds represented by this time span
		TK_FORCE_INLINE float getTotalSeconds() const
		{
			return static_cast<float>(ticks) / TicksPerSecond;
		}

	public:
		// Creates a time span that represents the specified number of days.
		static TTimeSpan fromDays(double days);
		// Creates a time span that represents the specified number of hours.
		static TTimeSpan fromHours(double hours);
		// Creates a time span that represents the specified number of milliseconds.
		static TTimeSpan fromMilliseconds(double milliseconds);
		// Creates a time span that represents the specified number of minutes.
		static TTimeSpan fromMinutes(double minutes);
		// Creates a time span that represents the specified number of seconds.
		static TTimeSpan fromSeconds(double seconds);

	public:
		// Returns the maximum time span value.
		static TTimeSpan maxValue();
		// Returns the minimum time span value.
		static TTimeSpan minValue();
		// Returns the zero time span value.
		static TTimeSpan zero();

	private:
		void set(i32 days, i32 hours, i32 minutes, i32 seconds, i32 milliseconds);
	};

	inline TTimeSpan operator*(float scalar, const TTimeSpan& timespan)
	{
		return timespan.operator*(scalar);
	}

	template<>
	struct TIsPODType<TTimeSpan>
	{
		enum
		{
			Value = true
		};
	};

}

FORMAT_CUSTOM(toolkit::TTimeSpan, "%02d:%02d:%02d.%07d",
	t.getHours(),
	t.getMinutes(),
	t.getSeconds(),
	(int)(t.ticks % 10000000))