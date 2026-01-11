// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <math.h>

namespace toolkit
{
	// Common math functions collection
	class TkMath final
	{
	public:
		// Common const values
		inline static constexpr float PI              = 3.14159265358979323846f;
		inline static constexpr float TWO_PI          = 6.28318530717958647692f;
		inline static constexpr float PI_INV          = 0.31830988618379067154f;
		inline static constexpr float PI_OVER_2        = 1.57079632679489661923f;
		inline static constexpr float PI_OVER_4        = 0.78539816339744830962f;
		inline static constexpr float PI_HALF          = PI_OVER_2;
		inline static constexpr float GOLDEN_RATIO     = 1.61803398874989484820f;

		// Tolerances
		inline static constexpr float  ZeroTolerance        = 1e-6f;
		inline static constexpr f64 ZeroToleranceDouble  = 1e-16;

		// Angle conversions
		inline static constexpr float RadiansToDegrees = 180.0f / PI;
		inline static constexpr float DegreesToRadians = PI / 180.0f;

	public:
		// Computes the sine and cosine of a scalar float.
		static void sinCos(f32 angle, float& sine, float& cosine);
		// Computes the base 2 logarithm for an integer value that is greater than 0. The result is rounded down to the nearest integer.
		static u32 floorLog2(u32 value);

	public:
		static TK_FORCE_INLINE f32 trunc(f32 value)
		{
			return truncf(value);
		}

		static TK_FORCE_INLINE f32 round(f32 value)
		{
			return roundf(value);
		}

		static TK_FORCE_INLINE f32 floor(f32 value)
		{
			return floorf(value);
		}

		static TK_FORCE_INLINE f32 ceil(f32 value)
		{
			return ceilf(value);
		}

		static TK_FORCE_INLINE f32 sin(f32 value)
		{
			return sinf(value);
		}

		static TK_FORCE_INLINE f32 asin(f32 value)
		{
			return asinf(value < -1.f ? -1.f : value < 1.f ? value : 1.f);
		}

		static TK_FORCE_INLINE f32 sinh(f32 value)
		{
			return sinhf(value);
		}

		static TK_FORCE_INLINE f32 cos(f32 value)
		{
			return cosf(value);
		}

		static TK_FORCE_INLINE f32 acos(f32 value)
		{
			return acosf(value < -1.f ? -1.f : value < 1.f ? value : 1.f);
		}

		static TK_FORCE_INLINE f32 tan(f32 value)
		{
			return tanf(value);
		}

		static TK_FORCE_INLINE f32 atan(f32 value)
		{
			return atanf(value);
		}

		static TK_FORCE_INLINE f32 atan2(f32 y, f32 x)
		{
			return atan2f(y, x);
		}

		static TK_FORCE_INLINE f32 invSqrt(f32 value)
		{
			return 1.0f / sqrtf(value);
		}

		static TK_FORCE_INLINE f32 log(const f32 value)
		{
			return logf(value);
		}

		static TK_FORCE_INLINE f32 log2(const f32 value)
		{
			return log2f(value);
		}

		static TK_FORCE_INLINE f32 log10(const f32 value)
		{
			return log10f(value);
		}

		static TK_FORCE_INLINE f32 pow(const f32 base, const f32 exponent)
		{
			return powf(base, exponent);
		}

		static TK_FORCE_INLINE f32 sqrt(const f32 value)
		{
			return sqrtf(value);
		}

		static TK_FORCE_INLINE f32 exp(const f32 value)
		{
			return expf(value);
		}

		static TK_FORCE_INLINE f32 exp2(const f32 value)
		{
			return exp2f(value);
		}

		static TK_FORCE_INLINE f32 abs(const f32 value)
		{
			return fabsf(value);
		}

		static TK_FORCE_INLINE i32 abs(const i32 value)
		{
			return value < 0 ? -value : value;
		}

		static TK_FORCE_INLINE i64 abs(const i64 value)
		{
			return value < 0 ? -value : value;
		}

		static TK_FORCE_INLINE i32 mod(const i32 a, const i32 b)
		{
			return (i32)
				fmodf((float)a, (float)b);
		}

		static TK_FORCE_INLINE f32 mod(const f32 a, const f32 b)
		{
			return fmodf(a, b);
		}

		static TK_FORCE_INLINE f32 modF(const f32 a, float* b)
		{
			return modff(a, b);
		}

		static TK_FORCE_INLINE f32 frac(f32 value)
		{
			return value - floor(value);
		}

		// Returns signed fractional part of a float.
		static TK_FORCE_INLINE f32 fractional(f32 value)
		{
			return value - trunc(value);
		}

		static i32 truncToInt(f32 value)
		{
			return (i32)
				value;
		}

		static i32 floorToInt(f32 value)
		{
			return truncToInt(floorf(value));
		}

		static TK_FORCE_INLINE i32 roundToInt(f32 value)
		{
			return floorToInt(value + 0.5f);
		}

		static TK_FORCE_INLINE i32 ceilToInt(f32 value)
		{
			return truncToInt(ceilf(value));
		}

	public:
		static i32 roundUpToPowerOf2(i32 value);
		static u32 roundUpToPowerOf2(u32 value);
		static i64 roundUpToPowerOf2(i64 value);
		static u64 roundUpToPowerOf2(u64 value);

	public:
		template<class T>
		static TK_FORCE_INLINE T divideAndRoundUp(T dividend, T divisor)
		{
			return (dividend + divisor - 1) /
				divisor;
		}

		template<class T>
		static TK_FORCE_INLINE T divideAndRoundDown(T dividend, T divisor)
		{
			return dividend / divisor;
		}

		template<class T>
		static bool isInRange(const T value, const T min, const T max)
		{
			return value >= min && value <= max;
		}

		template<class T>
		static bool isOutOfRange(const T value, const T min, const T max)
		{
			return value < min || value > max;
		}

		static bool isPowerOfTwo(u32 value)
		{
			//FIX: calculate piority problem fix
			return value != 0 && ((value & (value - 1)) == 0);
		}

		template<class T>
		static T clamp(const T value, const T min, const T max)
		{
			return value < min ? min : value < max ? value : max;
		}

		template<class T>
		static T saturate(const T value)
		{
			return value < 0 ? 0 : value < 1 ? value : 1;
		}

		template<class T>
		static f32 averageArithmetic(const T a, const T b)
		{
			return (a + b) * 0.5f;
		}

	public:
		template<class T>
		static T max(const T a, const T b)
		{
			return a > b ? a : b;
		}

		template<class T>
		static T min(const T a, const T b)
		{
			return a < b ? a : b;
		}

		template<class T>
		static T max(const T a, const T b, const T c)
		{
			return max(max(a, b), c);
		}

		template<class T>
		static T max(const T a, const T b, const T c, const T d)
		{
			return max(max(max(a, b), c), d);
		}

		template<class T>
		static T min(const T a, const T b, const T c)
		{
			return min(min(a, b), c);
		}

		template<class T>
		static T min(const T a, const T b, const T c, const T d)
		{
			return min(min(min(a, b), c), d);
		}

		template<class T>
		static T moveTowards(const T current, const T target, const T maxDelta)
		{
			if (abs(target - current) <= maxDelta)
				return target;
			return current + Sign(target - current) * maxDelta;
		}

		template<class T>
		static T moveTowardsAngle(const T current, const T target, const T maxDelta)
		{
			T delta = DeltaAngle(current, target);
			if ((-maxDelta < delta) && (delta < maxDelta))
				return target;
			T deltaTarget = current + delta;
			return moveTowards(current, deltaTarget, maxDelta);
		}

		template<class T>
		static T deltaAngle(const T current, const T target)
		{
			T t = repeat(target - current, (T)360);
			if (t > (T)180)
				t -= (T)360;
			return t;
		}

		template<class T>
		static T repeat(const T t, const T length)
		{
			return t - floor(t / length) * length;
		}

		template<class T>
		static T square(const T a)
		{
			return a * a;
		}

		template<class T, class U>
		static T lerp(const T& a, const T& b, const U& alpha)
		{
			return (T)(a + alpha * (b - a));
		}

		template<class T>
		static T lerpStable(const T& a, const T& b, f64 alpha)
		{
			return (T)(a * (1.0 - alpha) + b * alpha);
		}

		template<class T>
		static T lerpStable(const T& a, const T& b, f32 alpha)
		{
			return (T)(a * (1.0f - alpha) + b * alpha);
		}

		template<class T, class U>
		static T inverseLerp(const T& a, const T& b, const U& value)
		{
			if (a == b)
				return (T)0;
			return saturate((value - a) / (b - a));
		}

	public:
		static f32 smoothStep(f32 amount)
		{
			return amount <= 0 ? 0 : amount >= 1 ? 1 : amount * amount * (3 - 2 * amount);
		}

		static f32 smootherStep(f32 amount)
		{
			return amount <= 0 ? 0 : amount >= 1 ? 1 : amount * amount * amount * (amount * (amount * 6 - 15) + 10);
		}

		inline i32 isZero(i32 a)
		{
			return a == 0;
		}

		inline bool isZero(f32 a)
		{
			return abs(a) < ZeroTolerance;
		}

		inline bool isOne(i32 a)
		{
			return a == 1;
		}

		inline bool isOne(f32 a)
		{
			return isZero(a - 1.0f);
		}

		inline f32 sign(f32 v)
		{
			return v > 0.0f ? 1.0f : v < 0.0f ? -1.0f : 0.0f;
		}

		inline bool sameSign(const f32 a, const f32 b)
		{
			return a * b >= 0.0f;
		}

		inline bool notSameSign(const f32 a, const f32 b)
		{
			return a * b < 0.0f;
		}

		static bool nearEqual(f32 a, f32 b);
		static TK_FORCE_INLINE bool notNearEqual(f32 a, f32 b)
		{
			return !nearEqual(a, b);
		}

		static bool nearEqual(f32 a, f32 b, f32 eps)
		{
			return abs(a - b) < eps;
		}

		static f32 remap(f32 value, f32 frommin, f32 frommax, f32 tomin, f32 tomax)
		{
			return (value - frommin) / (frommax - frommin) * (tomax - tomin) + tomin;
		}

		template<typename T>
		static T alignUpWithMask(T value, T mask)
		{
			return (T)(value + mask & ~mask);
		}

		template<typename T>
		static T alignDownWithMask(T value, T mask)
		{
			return (T)(value & ~mask);
		}

		template<typename T>
		static T alignUp(T value, T alignment)
		{
			T mask = alignment - 1;
			return (T)(value + mask & ~mask);
		}

		template<typename T>
		static T alignDown(T value, T alignment)
		{
			T mask = alignment - 1;
			return (T)(value & ~mask);
		}

		template<typename T>
		static bool isAligned(T value, T alignment)
		{
			// FIX: piority problem
			return (value & (alignment - 1)) == 0;
		}

		template<typename T>
		static T divideByMultiple(T value, T alignment)
		{
			return (T)((value + alignment - 1) / alignment);
		}

		static f32 clampAxis(f32 angle)
		{
			angle = mod(angle, 360.f);
			if (angle < 0.0f)
				angle += 360.0f;
			return angle;
		}

		static f32 normalizeAxis(f32 angle)
		{
			angle = clampAxis(angle);
			if (angle > 180.f)
				angle -= 360.f;
			return angle;
		}

		static f32 findDeltaAngle(f32 a1, f32 a2)
		{
			f32 delta = a2 - a1;
			if (delta > PI)
				delta = delta - TWO_PI;
			else if (delta < -PI)
				delta = delta + TWO_PI;
			return delta;
		}

	public:
		template<typename T>
		static T unwindRadians(T a)
		{
			while (a > PI)
				a -= TWO_PI;
			while (a < -PI)
				a += TWO_PI;
			return a;
		}

		template<typename T>
		static T unwindDegrees(T a)
		{
			while (a > 180)
				a -= 360;
			while (a < -180)
				a += 360;
			return a;
		}

		static f32 floatSelect(f32 comparand, f32 valueGEZero, f32 valueLTZero)
		{
			return comparand >= 0.f ? valueGEZero : valueLTZero;
		}

		static f32 smoothStep(f32 a, f32 b, f32 x)
		{
			if (x < a)
				return 0.0f;
			if (x >= b)
				return 1.0f;
			const f32 fraction = (x - a) / (b - a);
			return fraction * fraction * (3.0f - 2.0f * fraction);
		}

		template<class T, class U>
		static TK_FORCE_INLINE T cubicInterp(const T& p0, const T& t0, const T& p1, const T& t1, const U& alpha)
		{
			const f32 alpha2 = alpha * alpha;
			const f32 alpha3 = alpha2 * alpha;
			return (T)((2 * alpha3 - 3 * alpha2 + 1) * p0) + (alpha3 - 2 * alpha2 + alpha) * t0 + (alpha3 - alpha2) * t1
				+ (-2 * alpha3 + 3 * alpha2) * p1;
		}

		template<class T>
		static TK_FORCE_INLINE T interpEaseIn(const T& a, const T& b, f32 alpha, f32 exponent)
		{
			const f32 blend = pow(alpha, exponent);
			return lerp<T>(a, b, blend);
		}

		template<class T>
		static TK_FORCE_INLINE T interpEaseOut(const T& a, const T& b, f32 alpha, f32 exponent)
		{
			const f32 blend = 1.f - pow(1.f - alpha, exponent);
			return lerp<T>(a, b, blend);
		}

		template<class T>
		static TK_FORCE_INLINE T interpEaseInOut(const T& a, const T& b, f32 alpha, f32 exponent)
		{
			return lerp<T>(a, b, alpha < 0.5f ? interpEaseIn(0.f, 1.f, alpha * 2.f, exponent) * 0.5f :
			                     interpEaseOut(0.f, 1.f, alpha * 2.f - 1.f, exponent) * 0.5f + 0.5f);
		}

		template<class T>
		static TK_FORCE_INLINE T interpSinIn(const T& a, const T& b, f32 alpha)
		{
			const f32 blend = -1.f * cos(alpha * PI_HALF) + 1.f;
			return lerp<T>(a, b, blend);
		}

		template<class T>
		static TK_FORCE_INLINE T interpSinOut(const T& a, const T& b, f32 alpha)
		{
			const f32 blend = sin(alpha * PI_HALF);
			return lerp<T>(a, b, blend);
		}

		template<class T>
		static TK_FORCE_INLINE T interpSinInOut(const T& a, const T& b, f32 alpha)
		{
			return lerp<T>(a, b,
				alpha < 0.5f ? interpSinIn(0.f, 1.f, alpha * 2.f) * 0.5f :
				interpSinOut(0.f, 1.f, alpha * 2.f - 1.f) * 0.5f
					+ 0.5f);
		}

		template<class T>
		static TK_FORCE_INLINE T interpExpoIn(const T& a, const T& b, f32 alpha)
		{
			const f32 blend = alpha == 0.f ? 0.f : pow(2.f, 10.f * (alpha - 1.f));
			return lerp<T>(a, b, blend);
		}

		template<class T>
		static TK_FORCE_INLINE T interpExpoOut(const T& a, const T& b, f32 alpha)
		{
			const f32 blend = alpha == 1.f ? 1.f : -pow(2.f, -10.f * alpha) + 1.f;
			return lerp<T>(a, b, blend);
		}

		template<class T>
		static TK_FORCE_INLINE T interpExpoInOut(const T& a, const T& b, f32 alpha)
		{
			return lerp<T>(a, b,
				alpha < 0.5f ? interpExpoIn(0.f, 1.f, alpha * 2.f) * 0.5f :
				interpExpoOut(0.f, 1.f, alpha * 2.f - 1.f) * 0.5f
					+ 0.5f);
		}

		template<class T>
		static TK_FORCE_INLINE T interpCircularIn(const T& a, const T& b, f32 alpha)
		{
			const f32 blend = -1.f * (sqrt(1.f - alpha * alpha) - 1.f);
			return lerp<T>(a, b, blend);
		}

		template<class T>
		static TK_FORCE_INLINE T interpCircularOut(const T& a, const T& b, f32 alpha)
		{
			alpha -= 1.f;
			const f32 blend = sqrt(1.f - alpha * alpha);
			return lerp<T>(a, b, blend);
		}

		template<class T>
		static TK_FORCE_INLINE T interpCircularInOut(const T& a, const T& b, f32 alpha)
		{
			return lerp<T>(a, b, alpha < 0.5f ? interpCircularIn(0.f, 1.f, alpha * 2.f) * 0.5f :
			                     interpCircularOut(0.f, 1.f, alpha * 2.f - 1.f) * 0.5f + 0.5f);
		}
	};


	// Double version of math util functions
	class TkMathD final
	{
	public:
		// Computes the sine and cosine of a scalar f64.
		void sinCos(f64 angle, f64& sine, f64& cosine);

	public:
		static TK_FORCE_INLINE f64 trunc(f64 value)
		{
			return ::trunc(value);
		}

		static TK_FORCE_INLINE f64 round(f64 value)
		{
			return ::round(value);
		}

		static TK_FORCE_INLINE f64 floor(f64 value)
		{
			return ::floor(value);
		}

		static TK_FORCE_INLINE f64 ceil(f64 value)
		{
			return ::ceil(value);
		}

		static TK_FORCE_INLINE f64 sin(f64 value)
		{
			return ::sin(value);
		}

		static TK_FORCE_INLINE f64 asin(f64 value)
		{
			return ::asin(value < -1. ? -1. : value < 1. ? value : 1.);
		}

		static TK_FORCE_INLINE f64 sinh(f64 value)
		{
			return ::sinh(value);
		}

		static TK_FORCE_INLINE f64 cos(f64 value)
		{
			return ::cos(value);
		}

		static TK_FORCE_INLINE f64 acos(f64 value)
		{
			return ::acos(value < -1. ? -1. : value < 1. ? value : 1.);
		}

		static TK_FORCE_INLINE f64 tan(f64 value)
		{
			return ::tan(value);
		}

		static TK_FORCE_INLINE f64 atan(f64 value)
		{
			return ::atan(value);
		}

		static TK_FORCE_INLINE f64 atan2(f64 y, f64 x)
		{
			return ::atan2(y, x);
		}

		static TK_FORCE_INLINE f64 invSqrt(f64 value)
		{
			return 1.0 / ::sqrt(value);
		}

		static TK_FORCE_INLINE f64 log(const f64 value)
		{
			return ::log(value);
		}

		static TK_FORCE_INLINE f64 log2(const f64 value)
		{
			return ::log2(value);
		}

		static TK_FORCE_INLINE f64 log10(const f64 value)
		{
			return ::log10(value);
		}

		static TK_FORCE_INLINE f64 pow(const f64 base, const f64 exponent)
		{
			return ::pow(base, exponent);
		}

		static TK_FORCE_INLINE f64 sqrt(const f64 value)
		{
			return ::sqrt(value);
		}

		static TK_FORCE_INLINE f64 exp(const f64 value)
		{
			return ::exp(value);
		}

		static TK_FORCE_INLINE f64 exp2(const f64 value)
		{
			return ::exp2(value);
		}

		static TK_FORCE_INLINE f64 abs(const f64 value)
		{
			return ::fabs(value);
		}

		static TK_FORCE_INLINE f64 mod(const f64 a, const f64 b)
		{
			return ::fmod(a, b);
		}

		static TK_FORCE_INLINE f64 modF(f64 a, f64* b)
		{
			return ::modf(a, b);
		}

		static TK_FORCE_INLINE f64 frac(f64 value)
		{
			return value - floor(value);
		}

		// Returns signed fractional part of a f64.
		static TK_FORCE_INLINE f64 fractional(f64 value)
		{
			return value - trunc(value);
		}

		static i64 truncToInt(f64 value)
		{
			return (i64)value;
		}

		static i64 floorToInt(f64 value)
		{
			return truncToInt(floor(value));
		}

		static TK_FORCE_INLINE i64 roundToInt(f64 value)
		{
			return floorToInt(value + 0.5);
		}

		static TK_FORCE_INLINE i64 ceilToInt(f64 value)
		{
			return truncToInt(ceil(value));
		}

	public:
		static f64 smoothStep(f64 amount)
		{
			return amount <= 0. ? 0. : amount >= 1. ? 1. : amount * amount * (3. - 2. * amount);
		}

		static f64 smootherStep(f64 amount)
		{
			return amount <= 0. ? 0. : amount >= 1. ? 1. : amount * amount * amount * (amount * (amount * 6. - 15.) + 10.);
		}

		inline bool isZero(f64 a)
		{
			return abs(a) < TkMath::ZeroTolerance;
		}

		inline bool isOne(f64 a)
		{
			return isZero(a - 1.);
		}

		inline f64 sign(f64 v)
		{
			return v > 0. ? 1. : v < 0. ? -1. : 0.;
		}

		inline bool sameSign(const f64 a, const f64 b)
		{
			return a * b >= 0.;
		}

		inline bool notSameSign(const f64 a, const f64 b)
		{
			return a * b < 0.;
		}

		static bool notNearEqual(f64 a, f64 b)
		{
			return abs(a - b) >= TkMath::ZeroTolerance;
		}

		static bool nearEqual(f64 a, f64 b)
		{
			return abs(a - b) < TkMath::ZeroToleranceDouble;
		}

		static bool nearEqual(f64 a, f64 b, f64 eps)
		{
			return abs(a - b) < eps;
		}

		static f64 remap(f64 value, f64 fromMin, f64 fromMax, f64 toMin, f64 toMax)
		{
			return (value - fromMin) / (fromMax - fromMin) * (toMax - toMin) + toMin;
		}

	public:
		static f64 clampAxis(f64 angle);
		static f64 normalizeAxis(f64 angle);
		static f64 findDeltaAngle(f64 a1, f64 a2);
		static f64 smoothStep(f64 a, f64 b, f64 x);

		static f64 doubleSelect(f64 comparand, f64 valueGEZero, f64 valueLTZero)
		{
			return comparand >= 0. ? valueGEZero : valueLTZero;
		}
	};

}