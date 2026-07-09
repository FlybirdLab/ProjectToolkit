/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/platforms/System.hpp>

namespace toolkit
{

	// High-resolution performance counter based on Platform::getTimeSeconds.
	class TkTimer
	{
	public:
		TkTimer()
		{
			m_start = m_end = System::getTimeSeconds();
		}

		// Starts the counter.
		void start()
		{
			m_start = System::getTimeSeconds();
		}

		// Stops the counter.
		void stop()
		{
			m_end = System::getTimeSeconds();
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
	struct TIsPODType<TkTimer>
	{
		enum
		{
			Value = true
		};
	};
}