/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Half.hpp"

#include "Color.hpp"
#include "toolkit/math/geometry/Rect.hpp"
#include "toolkit/math/Vector4.hpp"

namespace toolkit
{

    static_assert(sizeof(Half) == 2, "Invalid Half type size.");
    static_assert(sizeof(Half2) == 4, "Invalid Half2 type size.");
    static_assert(sizeof(Half3) == 6, "Invalid Half3 type size.");
    static_assert(sizeof(Half4) == 8, "Invalid Half4 type size.");

    Half2 Half2::Zero(0.0f, 0.0f);
    Half3 Half3::Zero(0.0f, 0.0f, 0.0f);
    Half4 Half4::Zero(0.0f, 0.0f, 0.0f, 0.0f);

#if !USE_SSE_HALF_CONVERSION

    Half Float16Compressor::compress(float value)
    {
        Bits v, s;
        v.f = value;
        u32 sign = v.si & signN;
        v.si ^= sign;
        sign >>= shiftSign; // logical shift
        s.si = mulN;
        s.si = static_cast<i32>(s.f * v.f); // correct subnormals
        v.si ^= (s.si ^ v.si) & -(minN > v.si);
        v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
        v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
        v.ui >>= shift; // logical shift
        v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
        v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
        return v.ui | sign;
    }

    float Float16Compressor::decompress(Half value)
    {
        Bits v;
        v.ui = value;
        i32 sign = v.si & signC;
        v.si ^= sign;
        sign <<= shiftSign;
        v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
        v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
        Bits s;
        s.si = mulC;
        s.f *= v.si;
        const i32 mask = -(norC > v.si);
        v.si <<= shift;
        v.si ^= (s.si ^ v.si) & mask;
        v.si |= sign;
        return v.f;
    }

#endif

    FVec2 Half2::toFVec2() const
    {
        return FVec2(
            Float16Compressor::decompress(x),
            Float16Compressor::decompress(y));
    }

    FVec3 Half3::toFVec3() const
    {
        return FVec3(
            Float16Compressor::decompress(x),
            Float16Compressor::decompress(y),
            Float16Compressor::decompress(z));
    }

    Half4::Half4(const FVec4 &v)
    {
        z = Float16Compressor::compress(v.x);
        y = Float16Compressor::compress(v.y);
        z = Float16Compressor::compress(v.z);
        w = Float16Compressor::compress(v.w);
    }

    Half4::Half4(const Color &c)
    {
        z = Float16Compressor::compress(c.r);
        y = Float16Compressor::compress(c.g);
        z = Float16Compressor::compress(c.b);
        w = Float16Compressor::compress(c.a);
    }

    Half4::Half4(const Rectangle &rect)
    {
        z = Float16Compressor::compress(rect.location.x);
        y = Float16Compressor::compress(rect.location.y);
        z = Float16Compressor::compress(rect.size.x);
        w = Float16Compressor::compress(rect.size.y);
    }

    FVec2 Half4::toFVec2() const
    {
        return FVec2(
            Float16Compressor::decompress(x),
            Float16Compressor::decompress(y));
    }

    FVec3 Half4::toFVec3() const
    {
        return FVec3(
            Float16Compressor::decompress(x),
            Float16Compressor::decompress(y),
            Float16Compressor::decompress(z));
    }

    FVec4 Half4::toFVec4() const
    {
        return FVec4(
            Float16Compressor::decompress(x),
            Float16Compressor::decompress(y),
            Float16Compressor::decompress(z),
            Float16Compressor::decompress(w));
    }

    FloatR10G10B10A2::FloatR10G10B10A2(u32 packed)
        : value(packed)
    {
    }

    FloatR10G10B10A2::FloatR10G10B10A2(float x, float y, float z, float w)
    {
        x = TkMath::saturate(x);
        y = TkMath::saturate(y);
        z = TkMath::saturate(z);
        w = TkMath::saturate(w);

        x = TkMath::round(x * 1023.0f);
        y = TkMath::round(y * 1023.0f);
        z = TkMath::round(z * 1023.0f);
        w = TkMath::round(w * 3.0f);

        value = ((u32)w << 30) | (((u32)z & 0x3FF) << 20) | (((u32)y & 0x3FF) << 10) | (((u32)x & 0x3FF));
    }

    FloatR10G10B10A2::FloatR10G10B10A2(const FVec3 &v, float alpha)
        : FloatR10G10B10A2(v.x, v.y, v.z, alpha)
    {
    }

    FloatR10G10B10A2::FloatR10G10B10A2(const FVec4 &v)
        : FloatR10G10B10A2(v.x, v.y, v.z, v.w)
    {
    }

    FloatR10G10B10A2::FloatR10G10B10A2(const float *values)
        : FloatR10G10B10A2(values[0], values[1], values[2], values[3])
    {
    }

    FloatR10G10B10A2::operator FVec3() const
    {
        return toFVec3();
    }

    FloatR10G10B10A2::operator FVec4() const
    {
        return toFVec4();
    }

    FVec3 FloatR10G10B10A2::toFVec3() const
    {
        FVec3 vectorOut;
        u32 tmp;
        tmp = value & 0x3FF;
        vectorOut.x = (float)tmp / 1023.f;
        tmp = (value >> 10) & 0x3FF;
        vectorOut.y = (float)tmp / 1023.f;
        tmp = (value >> 20) & 0x3FF;
        vectorOut.z = (float)tmp / 1023.f;
        return vectorOut;
    }

    FVec4 FloatR10G10B10A2::toFVec4() const
    {
        FVec4 vectorOut;
        u32 tmp;
        tmp = value & 0x3FF;
        vectorOut.x = (float)tmp / 1023.f;
        tmp = (value >> 10) & 0x3FF;
        vectorOut.y = (float)tmp / 1023.f;
        tmp = (value >> 20) & 0x3FF;
        vectorOut.z = (float)tmp / 1023.f;
        vectorOut.w = (float)(value >> 30) / 3.f;
        return vectorOut;
    }

    FloatR11G11B10::FloatR11G11B10(float x, float y, float z)
    {
        // Reference: https://github.com/microsoft/DirectXMath

        u32 ivalue[4];
        ivalue[0] = *(u32 *)&x;
        ivalue[1] = *(u32 *)&y;
        ivalue[2] = *(u32 *)&z;
        ivalue[3] = 0;

        // TODO: optimize this?

        u32 result[3];

        // X & Y Channels (5-bit exponent, 6-bit mantissa)
        for (u32 j = 0; j < 2; j++)
        {
            u32 sign = ivalue[j] & 0x80000000;
            u32 i = ivalue[j] & 0x7FFFFFFF;

            if ((i & 0x7F800000) == 0x7F800000)
            {
                // INF or NAN
                result[j] = 0x7c0;
                if ((i & 0x7FFFFF) != 0)
                {
                    result[j] = 0x7c0 | (((i >> 17) | (i >> 11) | (i >> 6) | (i)) & 0x3f);
                }
                else if (sign)
                {
                    // -INF is clamped to 0 since 3PK is positive only
                    result[j] = 0;
                }
            }
            else if (sign)
            {
                // 3PK is positive only, so clamp to zero
                result[j] = 0;
            }
            else if (i > 0x477E0000U)
            {
                // The number is too large to be represented as a float11, set to max
                result[j] = 0x7BF;
            }
            else
            {
                if (i < 0x38800000U)
                {
                    // The number is too small to be represented as a normalized float11
                    // Convert it to a denormalized value.
                    u32 shift = 113U - (i >> 23U);
                    i = (0x800000U | (i & 0x7FFFFFU)) >> shift;
                }
                else
                {
                    // Rebias the exponent to represent the value as a normalized float11
                    i += 0xC8000000U;
                }
                result[j] = ((i + 0xFFFFU + ((i >> 17U) & 1U)) >> 17U) & 0x7ffU;
            }
        }

        // Z Channel (5-bit exponent, 5-bit mantissa)
        u32 sign = ivalue[2] & 0x80000000;
        u32 i = ivalue[2] & 0x7FFFFFFF;

        if ((i & 0x7F800000) == 0x7F800000)
        {
            // INF or NAN
            result[2] = 0x3e0;
            if (i & 0x7FFFFF)
            {
                result[2] = 0x3e0 | (((i >> 18) | (i >> 13) | (i >> 3) | (i)) & 0x1f);
            }
            else if (sign)
            {
                // -INF is clamped to 0 since 3PK is positive only
                result[2] = 0;
            }
        }
        else if (sign)
        {
            // 3PK is positive only, so clamp to zero
            result[2] = 0;
        }
        else if (i > 0x477C0000U)
        {
            // The number is too large to be represented as a float10, set to max
            result[2] = 0x3df;
        }
        else
        {
            if (i < 0x38800000U)
            {
                // The number is too small to be represented as a normalized float10
                // Convert it to a denormalized value.
                u32 shift = 113U - (i >> 23U);
                i = (0x800000U | (i & 0x7FFFFFU)) >> shift;
            }
            else
            {
                // Rebias the exponent to represent the value as a normalized float10
                i += 0xC8000000U;
            }
            result[2] = ((i + 0x1FFFFU + ((i >> 18U) & 1U)) >> 18U) & 0x3ffU;
        }

        // Pack result into memory
        value = (result[0] & 0x7ff) | ((result[1] & 0x7ff) << 11) | ((result[2] & 0x3ff) << 22);
    }

    FloatR11G11B10::FloatR11G11B10(const FVec3 &v)
        : FloatR11G11B10(v.x, v.y, v.z)
    {
    }

    FloatR11G11B10::FloatR11G11B10(const FVec4 &v)
        : FloatR11G11B10(v.x, v.y, v.z)
    {
    }

    FloatR11G11B10::FloatR11G11B10(const Color &v)
        : FloatR11G11B10(v.r, v.g, v.b)
    {
    }

    FloatR11G11B10::FloatR11G11B10(const float *values)
        : FloatR11G11B10(values[0], values[1], values[2])
    {
    }

    FloatR11G11B10::operator FVec3() const
    {
        return toFVec3();
    }

    FVec3 FloatR11G11B10::toFVec3() const
    {
        // Reference: https://github.com/microsoft/DirectXMath

        u32 result[4];
        u32 mantissa;
        u32 exponent;

        // TODO: optimize this?

        // X Channel (6-bit mantissa)
        mantissa = xm;

        if (xe == 0x1f) // INF or NAN
        {
            result[0] = 0x7f800000 | (xm << 17);
        }
        else
        {
            if (xe != 0) // The value is normalized
            {
                exponent = xe;
            }
            else if (mantissa != 0) // The value is denormalized
            {
                // Normalize the value in the resulting float
                exponent = 1;

                do
                {
                    exponent--;
                    mantissa <<= 1;
                } while ((mantissa & 0x40) == 0);

                mantissa &= 0x3F;
            }
            else // The value is zero
            {
                exponent = (u32)-112;
            }

            result[0] = ((exponent + 112) << 23) | (mantissa << 17);
        }

        // Y Channel (6-bit mantissa)
        mantissa = ym;

        if (ye == 0x1f) // INF or NAN
        {
            result[1] = 0x7f800000 | (ym << 17);
        }
        else
        {
            if (ye != 0) // The value is normalized
            {
                exponent = ye;
            }
            else if (mantissa != 0) // The value is denormalized
            {
                // Normalize the value in the resulting float
                exponent = 1;

                do
                {
                    exponent--;
                    mantissa <<= 1;
                } while ((mantissa & 0x40) == 0);

                mantissa &= 0x3F;
            }
            else // The value is zero
            {
                exponent = (u32)-112;
            }

            result[1] = ((exponent + 112) << 23) | (mantissa << 17);
        }

        // Z Channel (5-bit mantissa)
        mantissa = zm;

        if (ze == 0x1f) // INF or NAN
        {
            result[2] = 0x7f800000 | (zm << 17);
        }
        else
        {
            if (ze != 0) // The value is normalized
            {
                exponent = ze;
            }
            else if (mantissa != 0) // The value is denormalized
            {
                // Normalize the value in the resulting float
                exponent = 1;

                do
                {
                    exponent--;
                    mantissa <<= 1;
                } while ((mantissa & 0x20) == 0);

                mantissa &= 0x1F;
            }
            else // The value is zero
            {
                exponent = (u32)-112;
            }

            result[2] = ((exponent + 112) << 23) | (mantissa << 18);
        }

        return FVec3((float *)result);
    }

    FVec2 RG16UNorm::toFVec2() const
    {
        return FVec2(
            (float)x / kMaxU16,
            (float)y / kMaxU16);
    }

    FVec4 RGBA16UNorm::toFVec4() const
    {
        return FVec4(
            (float)x / kMaxU16,
            (float)y / kMaxU16,
            (float)z / kMaxU16,
            (float)w / kMaxU16);
    }

}