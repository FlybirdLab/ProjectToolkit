// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.
//
// Function TkMath::roundUpToPowerOf2 algrithm from
//          http://graphics.stanford.edu/~seander/bithacks.html#roundUpPowerOf2
// Function TkMath::nearEqual algrithm from
//          Bruce Dawson: http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition
//          http://code.google.com/p/googletest/source/browse/trunk/include/gtest/internal/gtest-internal.h
// Function TkMath::floorLog2 algrithm from
//          http://codinggorilla.domemtech.com/?p=81
//          http://en.wikipedia.org/wiki/Binary_logarithm

#include "Math.hpp"

namespace toolkit {

	void TkMath::sinCos(f32 angle, f32& sine, f32& cosine)
	{
		sine = TkMath::sin(angle);
		cosine = TkMath::cos(angle);
	}

	u32 TkMath::floorLog2(u32 value)
	{
		u32 pos = 0;
		if (value >= 1 << 16)
		{
			value >>= 16;
			pos += 16;
		}
		if (value >= 1 << 8)
		{
			value >>= 8;
			pos += 8;
		}
		if (value >= 1 << 4)
		{
			value >>= 4;
			pos += 4;
		}
		if (value >= 1 << 2)
		{
			value >>= 2;
			pos += 2;
		}
		if (value >= 1 << 1)
		{
			pos += 1;
		}
		return value == 0 ? 0 : pos;
	}

	i32 TkMath::roundUpToPowerOf2(i32 value)
	{
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		return value + 1;
	}

	u32 TkMath::roundUpToPowerOf2(u32 value)
	{
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		return value + 1;
	}

	i64 TkMath::roundUpToPowerOf2(i64 value)
	{
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		value |= value >> 32;
		return value + 1;
	}

	u64 TkMath::roundUpToPowerOf2(u64 value)
	{
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		value |= value >> 32;
		return value + 1;
	}

	bool TkMath::nearEqual(f32 a, f32 b)
	{
		// Check if the numbers are really close - needed when comparing numbers near zero
		if (abs(a - b) < ZeroTolerance)
			return true;

		const i32 aInt = *(i32*)&a;
		const i32 bInt = *(i32*)&b;

		// Different signs means they do not match
		if (aInt < 0 != bInt < 0)
			return false;

		// Find the difference in ULPs
		const int ulp = abs(aInt - bInt);

		// Choose of maxUlp = 4
		const int maxUlp = 4;
		return ulp <= maxUlp;
	}

	f64 TkMathD::smoothStep(f64 a, f64 b, f64 x)
	{
		if (x < a)
			return 0.;
		if (x >= b)
			return 1.;
		const f64 fraction = (x - a) / (b - a);
		return fraction * fraction * (3. - 2. * fraction);
	}

	f64 TkMathD::findDeltaAngle(f64 a1, f64 a2)
	{
		f64 delta = a2 - a1;
		if (delta > TkMath::PI)
			delta = delta - TkMath::TWO_PI;
		else if (delta < -TkMath::PI)
			delta = delta + TkMath::TWO_PI;
		return delta;
	}

	f64 TkMathD::clampAxis(f64 angle)
	{
		angle = mod(angle, 360.);
		if (angle < 0.)
			angle += 360.;
		return angle;
	}

	f64 TkMathD::normalizeAxis(f64 angle)
	{
		angle = clampAxis(angle);
		if (angle > 180.)
			angle -= 360.;
		return angle;
	}

	void TkMathD::sinCos(f64 angle, f64& sine, f64& cosine)
	{
		sine = TkMathD::sin(angle);
		cosine = TkMathD::cos(angle);
	}
}