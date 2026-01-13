/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "TimeSpan.hpp"
#include "String.hpp"

namespace toolkit {

	TkTimeSpan TkTimeSpan::fromDays(double days)
	{
		TK_ASSERT((days >= minValue().getTotalDays()) && (days <= maxValue().getTotalDays()));
		return TkTimeSpan(static_cast<i64>(days * TicksPerDay));
	}


	TkTimeSpan TkTimeSpan::fromHours(double hours)
	{
		TK_ASSERT((hours >= minValue().getTotalHours()) && (hours <= maxValue().getTotalHours()));
		return TkTimeSpan(static_cast<i64>(hours * TicksPerHour));
	}


	TkTimeSpan TkTimeSpan::fromMilliseconds(double milliseconds)
	{
		TK_ASSERT((milliseconds >= minValue().getTotalMilliseconds()) && (milliseconds <= maxValue().getTotalMilliseconds()));
		return TkTimeSpan(static_cast<i64>(milliseconds * TicksPerMillisecond));
	}


	TkTimeSpan TkTimeSpan::fromMinutes(double minutes)
	{
		TK_ASSERT((minutes >= minValue().getTotalMinutes()) && (minutes <= maxValue().getTotalMinutes()));
		return TkTimeSpan(static_cast<i64>(minutes * TicksPerMinute));
	}


	TkTimeSpan TkTimeSpan::fromSeconds(double seconds)
	{
		TK_ASSERT((seconds >= minValue().getTotalSeconds()) && (seconds <= maxValue().getTotalSeconds()));
		return TkTimeSpan(static_cast<i64>(seconds * TicksPerSecond));
	}


	TkTimeSpan TkTimeSpan::maxValue()
	{
		return TkTimeSpan(9223372036854775807);
	}


	TkTimeSpan TkTimeSpan::minValue()
	{
		return TkTimeSpan(-9223372036854775807 - 1);
	}


	TkTimeSpan TkTimeSpan::zero()
	{
		return TkTimeSpan(0);
	}


	void TkTimeSpan::set(i32 days, i32 hours, i32 minutes, i32 seconds, i32 milliseconds)
	{
		const i64 totalMs = 1000 * (60 * 60 * 24 * (i64)days + 60 * 60 * (i64)hours + 60 * (i64)minutes + (i64)seconds) + (i64)milliseconds;
		TK_ASSERT((totalMs >= minValue().getTotalMilliseconds()) && (totalMs <= maxValue().getTotalMilliseconds()));
		ticks = totalMs * TicksPerMillisecond;
	}


	TkString TkTimeSpan::toString() const
	{
		return TkString::format("%s", *this);
	}


	TkString TkTimeSpan::toString(const char option) const
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