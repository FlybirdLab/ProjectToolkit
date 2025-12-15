// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/TypeTraits.hpp>

namespace toolkit
{

	// High-resolution performance counter based on Platform::getTimeSeconds.
	class TTimer
	{
	public:
		TTimer()
		{
			m_start = m_end = platform::getTimeSeconds();
		}

		// Starts the counter.
		void start()
		{
			m_start = platform::getTimeSeconds();
		}

		// Stops the counter.
		void stop()
		{
			m_end = platform::getTimeSeconds();
		}

		// Gets the milliseconds time.
		TK_FORCE_INLINE i32 getMilliseconds() const
		{
			return (i32)((m_end - m_start) * 1000.0);
		}

		// Gets the total number of milliseconds.
		TK_FORCE_INLINE double getTotalMilliseconds() const
		{
			return (float)((m_end - m_start) * 1000.0);
		}

		// Gets the total number of seconds.
		TK_FORCE_INLINE float getTotalSeconds() const
		{
			return (float)(m_end - m_start);
		}

	private:
		double m_start, m_end;
	};

	template<>
	struct TIsPODType<TTimer>
	{
		enum
		{
			Value = true
		};
	};
}