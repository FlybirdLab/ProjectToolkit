// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "TimeSpan.hpp"
#include "String.hpp"

namespace toolkit {

	TTimeSpan TTimeSpan::fromDays(double days)
	{
		TK_ASSERT((days >= minValue().getTotalDays()) && (days <= maxValue().getTotalDays()));
		return TTimeSpan(static_cast<i64>(days * TicksPerDay));
	}


	TTimeSpan TTimeSpan::fromHours(double hours)
	{
		TK_ASSERT((hours >= minValue().getTotalHours()) && (hours <= maxValue().getTotalHours()));
		return TTimeSpan(static_cast<i64>(hours * TicksPerHour));
	}


	TTimeSpan TTimeSpan::fromMilliseconds(double milliseconds)
	{
		TK_ASSERT((milliseconds >= minValue().getTotalMilliseconds()) && (milliseconds <= maxValue().getTotalMilliseconds()));
		return TTimeSpan(static_cast<i64>(milliseconds * TicksPerMillisecond));
	}


	TTimeSpan TTimeSpan::fromMinutes(double minutes)
	{
		TK_ASSERT((minutes >= minValue().getTotalMinutes()) && (minutes <= maxValue().getTotalMinutes()));
		return TTimeSpan(static_cast<i64>(minutes * TicksPerMinute));
	}


	TTimeSpan TTimeSpan::fromSeconds(double seconds)
	{
		TK_ASSERT((seconds >= minValue().getTotalSeconds()) && (seconds <= maxValue().getTotalSeconds()));
		return TTimeSpan(static_cast<i64>(seconds * TicksPerSecond));
	}


	TTimeSpan TTimeSpan::maxValue()
	{
		return TTimeSpan(9223372036854775807);
	}


	TTimeSpan TTimeSpan::minValue()
	{
		return TTimeSpan(-9223372036854775807 - 1);
	}


	TTimeSpan TTimeSpan::zero()
	{
		return TTimeSpan(0);
	}


	void TTimeSpan::set(i32 days, i32 hours, i32 minutes, i32 seconds, i32 milliseconds)
	{
		const i64 totalMs = 1000 * (60 * 60 * 24 * (i64)days + 60 * 60 * (i64)hours + 60 * (i64)minutes + (i64)seconds) + (i64)milliseconds;
		TK_ASSERT((totalMs >= minValue().getTotalMilliseconds()) && (totalMs <= maxValue().getTotalMilliseconds()));
		ticks = totalMs * TicksPerMillisecond;
	}


	TkString TTimeSpan::toString() const
	{
		return TkString::format("%s", *this);
	}


	TkString TTimeSpan::toString(const char option) const
	{
		switch (option)
		{
		case 'a':
			return TkString::format("%02d:%02d:%02d.%03d", getHours(), getMinutes(), getSeconds(), getMilliseconds());
		default:
			return TkString::format("%02d:%02d:%02d.%07d", getHours(), getMinutes(), getSeconds(), ticks % 10000000);
		}
	}

}