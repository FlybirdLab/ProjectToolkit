/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References
 *      http://www.cs.cmu.edu/~jinlianw/third_party/float16_compressor.hpp
 */

#pragma once

#include <toolkit/math/Vector2.hpp>
#include <toolkit/math/Vector3.hpp>

namespace toolkit
{

    // Half-precision 16 bit floating point number consisting of a sign bit, a 5 bit biased exponent, and a 10 bit mantissa
    typedef u16 Half;

#define USE_SSE_HALF_CONVERSION 0
#define USE_NEON_HALF_CONVENSION 0

    // Utility for packing/unpacking floating point value from single precision (32 bit) to half precision (16 bit).
    class TK_API Float16Compressor
    {
        union Bits
        {
            float f;
            i32 si;
            u32 ui;
        };

        static const int shift = 13;
        static const int shiftSign = 16;
        static const i32 infN = 0x7F800000;  // flt32 infinity
        static const i32 maxN = 0x477FE000;  // max flt16 normal as a flt32
        static const i32 minN = 0x38800000;  // min flt16 normal as a flt32
        static const i32 signN = 0x80000000; // flt32 sign bit
        static const i32 infC = infN >> shift;
        static const i32 nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
        static const i32 maxC = maxN >> shift;
        static const i32 minC = minN >> shift;
        static const i32 signC = signN >> shiftSign; // flt16 sign bit
        static const i32 mulN = 0x52000000;          // (1 << 23) / minN
        static const i32 mulC = 0x33800000;          // minN / (1 << (23 - shift))
        static const i32 subC = 0x003FF;             // max flt32 subnormal down shifted
        static const i32 norC = 0x00400;             // min flt32 normal down shifted
        static const i32 maxD = infC - maxC - 1;
        static const i32 minD = minC - subC - 1;

    public:
#if USE_SSE_HALF_CONVERSION
        TK_FORCE_INLINE static Half compress(float value)
        {
            __m128 value1 = _mm_set_ss(value);
            __m128i value2 = _mm_cvtps_ph(value1, 0);
            return static_cast<Half>(_mm_cvtsi128_si32(value2));
        }
        TK_FORCE_INLINE static float decompress(Half value)
        {
            __m128i value1 = _mm_cvtsi32_si128(static_cast<int>(value));
            __m128 value2 = _mm_cvtph_ps(value1);
            return _mm_cvtss_f32(value2);
        }
#elif USE_NEON_HALF_CONVENSION
        TK_FORCE_INLINE static Half compress(float value)
        {
            float16_t half_value = vcvt_f16_f32(value); // Convert from float to float16
            return static_cast<Half>(half_value);       // Return as Half (typically u16_t or u16_t)
        }

        TK_FORCE_INLINE static float decompress(Half value)
        {
            float16_t half_value = static_cast<float16_t>(value); // Convert Half back to float16_t
            return vcvt_f32_f16(half_value);                      // Convert from float16 to float32
        }
#else
        static Half compress(float value);
        static float decompress(Half value);
#endif
    };

    // Defines a two component vector, using half precision floating point coordinates.
    struct TK_API Half2
    {
        // zero vector
        static Half2 Zero;

        // Gets or sets the x component of the vector.
        Half x;
        // Gets or sets the y component of the vector.
        Half y;

        Half2() = default;
        TK_FORCE_INLINE Half2(Half x, Half y)
            : x(x), y(y)
        {
        }
        TK_FORCE_INLINE Half2(float x, float y)
        {
            x = Float16Compressor::compress(x);
            y = Float16Compressor::compress(y);
        }
        TK_FORCE_INLINE Half2(const FVec2 &v)
        {
            x = Float16Compressor::compress(v.x);
            y = Float16Compressor::compress(v.y);
        }

    public:
        FVec2 toFVec2() const;
    };

    // Defines a three component vector, using half precision floating point coordinates
    struct TK_API Half3
    {
        // zero vector
        static Half3 Zero;

        Half x;
        Half y;
        Half z;

        Half3() = default;
        TK_FORCE_INLINE Half3(Half x, Half y, Half z)
            : x(x), y(y), z(z)
        {
        }
        TK_FORCE_INLINE Half3(float x, float y, float z)
        {
            x = Float16Compressor::compress(x);
            y = Float16Compressor::compress(y);
            z = Float16Compressor::compress(z);
        }
        TK_FORCE_INLINE Half3(const FVec3 &v)
        {
            x = Float16Compressor::compress(v.x);
            y = Float16Compressor::compress(v.y);
            z = Float16Compressor::compress(v.z);
        }

        FVec3 toFVec3() const;
    };

    // Defines a four component vector, using half precision floating point coordinates
    struct TK_API Half4
    {
        // zero vector
        static Half4 Zero;

        Half x;
        Half y;
        Half z;
        Half w;

        Half4() = default;
        Half4(Half x, Half y, Half z, Half w)
            : x(x), y(y), z(z), w(w)
        {
        }
        Half4(const float x, const float y, const float z)
        {
            this->x = Float16Compressor::compress(x);
            this->y = Float16Compressor::compress(y);
            this->z = Float16Compressor::compress(z);
            this->w = 0;
        }
        Half4(const float x, const float y, const float z, const float w)
        {
            this->x = Float16Compressor::compress(x);
            this->y = Float16Compressor::compress(y);
            this->z = Float16Compressor::compress(z);
            this->w = Float16Compressor::compress(w);
        }

        explicit Half4(const FVec4 &v);
        explicit Half4(const Color &c);
        explicit Half4(const Rectangle &rect);

    public:
        FVec2 toFVec2() const;
        FVec3 toFVec3() const;
        FVec4 toFVec4() const;
    };

    // Half-precision 16 bit floating point number consisting of a sign bit, a 5 bit biased exponent, and a 10 bit mantissa.
    using Float16 = Half;

    // Packed vector, layout: R:10 bytes, G:10 bytes, B:10 bytes, A:2 bytes, all values are stored as floats in range [0;1].
    struct TK_API FloatR10G10B10A2
    {
        union
        {
            struct
            {
                u32 x : 10; // 0/1023 to 1023/1023
                u32 y : 10; // 0/1023 to 1023/1023
                u32 z : 10; // 0/1023 to 1023/1023
                u32 w : 2;  //     0/3 to       3/3
            };

            u32 value;
        };

        FloatR10G10B10A2() = default;

        explicit FloatR10G10B10A2(u32 packed);
        FloatR10G10B10A2(float x, float y, float z, float w);
        FloatR10G10B10A2(const FVec3 &v, float alpha = 0);
        FloatR10G10B10A2(const FVec4 &v);
        explicit FloatR10G10B10A2(const float *values);

        operator u32() const
        {
            return value;
        }

        operator FVec3() const;
        operator FVec4() const;

        FloatR10G10B10A2 &operator=(const FloatR10G10B10A2 &other)
        {
            value = other.value;
            return *this;
        }

        FloatR10G10B10A2 &operator=(u32 packed)
        {
            value = packed;
            return *this;
        }

        FVec3 toFVec3() const;
        FVec4 toFVec4() const;
    };

    // [Deprecated on 14.01.2022, expires on 14.01.2024]
    using Float1010102 = FloatR10G10B10A2;

    // The 3D vector is packed into 32 bits with 11/11/10 bits per floating-point component.
    struct TK_API FloatR11G11B10
    {
        union
        {
            struct
            {
                u32 xm : 6; // x-mantissa
                u32 xe : 5; // x-exponent
                u32 ym : 6; // y-mantissa
                u32 ye : 5; // y-exponent
                u32 zm : 5; // z-mantissa
                u32 ze : 5; // z-exponent
            };

            u32 value;
        };

        FloatR11G11B10() = default;

        explicit FloatR11G11B10(u32 packed)
            : value(packed)
        {
        }

        FloatR11G11B10(float x, float y, float z);
        FloatR11G11B10(const FVec3 &v);
        FloatR11G11B10(const FVec4 &v);
        FloatR11G11B10(const Color &v);
        explicit FloatR11G11B10(const float *values);

        operator u32() const
        {
            return value;
        }

        operator FVec3() const;

        FloatR11G11B10 &operator=(const FloatR11G11B10 &other)
        {
            value = other.value;
            return *this;
        }

        FloatR11G11B10 &operator=(u32 packed)
        {
            value = packed;
            return *this;
        }

    public:
        FVec3 toFVec3() const;
    };

    struct TK_API RG16UNorm
    {
        u16 x, y;

        RG16UNorm(float x, float y)
        {
            this->x = (u16)(x * kMaxU16);
            this->y = (u16)(y * kMaxU16);
        }

        FVec2 toFVec2() const;
    };

    struct TK_API RGBA16UNorm
    {
        u16 x, y, z, w;

        RGBA16UNorm(float x, float y, float z, float w)
        {
            this->x = (u16)(x * kMaxU16);
            this->y = (u16)(y * kMaxU16);
            this->z = (u16)(z * kMaxU16);
            this->w = (u16)(w * kMaxU16);
        }

        FVec4 toFVec4() const;
    };
}