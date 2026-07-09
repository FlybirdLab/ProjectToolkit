// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>

#if PLATFORM_SIMD_SSE2
#ifdef _WIN32
#include <xmmintrin.h>
#endif
#elif PLATFORM_SIMD_NEON
#ifdef __arm64
#include <arm_neon.h>
#endif
#else
#include <math.h>
#endif

namespace cobalt
{

#if PLATFORM_SIMD_SSE2

	// Vector of four floating point values stored in vector register.
	typedef __m128 SimdVector4;

	class TkSIMD final
	{
	public:
		TK_FORCE_INLINE static SimdVector4 load(float xyzw)
		{
			return _mm_set1_ps(xyzw);
		}

		TK_FORCE_INLINE static SimdVector4 load(float x, float y, float z, float w)
		{
			return _mm_set_ps(w, z, y, x);
		}

		TK_FORCE_INLINE static SimdVector4 load(const void* src)
		{
			return _mm_load_ps((const float*)(src));
		}

		TK_FORCE_INLINE static SimdVector4 splat(float value)
		{
			return _mm_set_ps1(value);
		}

		TK_FORCE_INLINE static void store(void* dst, SimdVector4 src)
		{
			_mm_store_ps((float*)dst, src);
		}

		TK_FORCE_INLINE static int moveMask(SimdVector4 a)
		{
			return _mm_movemask_ps(a);
		}

		TK_FORCE_INLINE static SimdVector4 add(SimdVector4 a, SimdVector4 b)
		{
			return _mm_add_ps(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 sub(SimdVector4 a, SimdVector4 b)
		{
			return _mm_sub_ps(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 mul(SimdVector4 a, SimdVector4 b)
		{
			return _mm_mul_ps(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 div(SimdVector4 a, SimdVector4 b)
		{
			return _mm_div_ps(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 rcp(SimdVector4 a)
		{
			return _mm_rcp_ps(a);
		}

		TK_FORCE_INLINE static SimdVector4 sqrt(SimdVector4 a)
		{
			return _mm_sqrt_ps(a);
		}

		TK_FORCE_INLINE static SimdVector4 rsqrt(SimdVector4 a)
		{
			return _mm_rsqrt_ps(a);
		}

		TK_FORCE_INLINE static SimdVector4 min(SimdVector4 a, SimdVector4 b)
		{
			return _mm_min_ps(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 max(SimdVector4 a, SimdVector4 b)
		{
			return _mm_max_ps(a, b);
		}
	};

#elif PLATFORM_SIMD_NEON

	typedef float32x4_t SimdVector4;

	class TkSIMD final
	{
	public:
		TK_FORCE_INLINE static SimdVector4 load(float xyzw)
		{
			return vdupq_n_f32(xyzw); // load 4 float32 to float32 vector
		}

		TK_FORCE_INLINE static SimdVector4 load(float x, float y, float z, float w)
		{
			return (SimdVector4) { x, y, z, w }; // set float4 data
		}

		TK_FORCE_INLINE static SimdVector4 load(const void* src)
		{
			return vld1q_f32((const float*)src); // load float4 from memory
		}

		TK_FORCE_INLINE static SimdVector4 splat(float value)
		{
			return vdupq_n_f32(value); // copy float4 to a vector
		}

		TK_FORCE_INLINE static void store(void* dst, SimdVector4 src)
		{
			vst1q_f32((float*)dst, src); // save float4 to memory
		}

		TK_FORCE_INLINE static int moveMask(SimdVector4 a)
		{
			uint32x4_t mask = vreinterpretq_u32_f32(a); // float f32 unsinged
			return (vgetq_lane_u32(mask, 0) >> 31) |
				((vgetq_lane_u32(mask, 1) >> 30) & 2) |
				((vgetq_lane_u32(mask, 2) >> 29) & 4) |
				((vgetq_lane_u32(mask, 3) >> 28) & 8); // like `movemask` of sse
		}

		TK_FORCE_INLINE static SimdVector4 add(SimdVector4 a, SimdVector4 b)
		{
			return vaddq_f32(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 sub(SimdVector4 a, SimdVector4 b)
		{
			return vsubq_f32(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 mul(SimdVector4 a, SimdVector4 b)
		{
			return vmulq_f32(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 div(SimdVector4 a, SimdVector4 b)
		{
			return vdivq_f32(a, b); // abrove ARMv8.3
		}

		TK_FORCE_INLINE static SimdVector4 rcp(SimdVector4 a)
		{
			return vrecpeq_f32(a);
		}

		TK_FORCE_INLINE static SimdVector4 sqrt(SimdVector4 a)
		{
			return vsqrtq_f32(a);
		}

		TK_FORCE_INLINE static SimdVector4 rsqrt(SimdVector4 a)
		{
			return vrsqrteq_f32(a);
		}

		TK_FORCE_INLINE static SimdVector4 min(SimdVector4 a, SimdVector4 b)
		{
			return vminq_f32(a, b);
		}

		TK_FORCE_INLINE static SimdVector4 max(SimdVector4 a, SimdVector4 b)
		{
			return vmaxq_f32(a, b);
		}
	};

#else
	struct SimdVector4
	{
		float X, Y, Z, W;
	};

	class TkSIMD final
	{
	public:
		TK_FORCE_INLINE static SimdVector4 load(float xyzw)
		{
			return { xyzw, xyzw, xyzw, xyzw };
		}

		TK_FORCE_INLINE static SimdVector4 load(float x, float y, float z, float w)
		{
			return { x, y, z, w };
		}

		TK_FORCE_INLINE static SimdVector4 load(const void* src)
		{
			return *(const SimdVector4*)src;
		}

		TK_FORCE_INLINE static SimdVector4 splat(float value)
		{
			return { value, value, value, value };
		}

		TK_FORCE_INLINE static void store(void* dst, SimdVector4 src)
		{
			(*(SimdVector4*)dst) = src;
		}

		TK_FORCE_INLINE static int moveMask(SimdVector4 a)
		{
			return (a.W < 0 ? (1 << 3) : 0) |
				(a.Z < 0 ? (1 << 2) : 0) |
				(a.Y < 0 ? (1 << 1) : 0) |
				(a.X < 0 ? 1 : 0);
		}

		TK_FORCE_INLINE static SimdVector4 add(SimdVector4 a, SimdVector4 b)
		{
			return
				{
					a.X + b.X,
					a.Y + b.Y,
					a.Z + b.Z,
					a.W + b.W
				};
		}

		TK_FORCE_INLINE static SimdVector4 sub(SimdVector4 a, SimdVector4 b)
		{
			return
				{
					a.X - b.X,
					a.Y - b.Y,
					a.Z - b.Z,
					a.W - b.W
				};
		}

		TK_FORCE_INLINE static SimdVector4 mul(SimdVector4 a, SimdVector4 b)
		{
			return
				{
					a.X * b.X,
					a.Y * b.Y,
					a.Z * b.Z,
					a.W * b.W
				};
		}

		TK_FORCE_INLINE static SimdVector4 div(SimdVector4 a, SimdVector4 b)
		{
			return
				{
					a.X / b.X,
					a.Y / b.Y,
					a.Z / b.Z,
					a.W / b.W
				};
		}

		TK_FORCE_INLINE static SimdVector4 rcp(SimdVector4 a)
		{
			return
				{
					1 / a.X,
					1 / a.Y,
					1 / a.Z,
					1 / a.W
				};
		}

		TK_FORCE_INLINE static SimdVector4 sqrt(SimdVector4 a)
		{
			return
				{
					(float)::sqrt(a.X),
					(float)::sqrt(a.Y),
					(float)::sqrt(a.Z),
					(float)::sqrt(a.W)
				};
		}

		TK_FORCE_INLINE static SimdVector4 rsqrt(SimdVector4 a)
		{
			return
				{
					1 / (float)::sqrt(a.X),
					1 / (float)::sqrt(a.Y),
					1 / (float)::sqrt(a.Z),
					1 / (float)::sqrt(a.W)
				};
		}

		TK_FORCE_INLINE static SimdVector4 min(SimdVector4 a, SimdVector4 b)
		{
			return
				{
					a.X < b.X ? a.X : b.X,
					a.Y < b.Y ? a.Y : b.Y,
					a.Z < b.Z ? a.Z : b.Z,
					a.W < b.W ? a.W : b.W
				};
		}

		TK_FORCE_INLINE static SimdVector4 max(SimdVector4 a, SimdVector4 b)
		{
			return
				{
					a.X > b.X ? a.X : b.X,
					a.Y > b.Y ? a.Y : b.Y,
					a.Z > b.Z ? a.Z : b.Z,
					a.W > b.W ? a.W : b.W
				};
		}
	};

#endif

}