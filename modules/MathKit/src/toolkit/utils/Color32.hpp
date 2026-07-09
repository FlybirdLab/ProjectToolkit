/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/math/common.hpp>

#include <toolkit/base/Format.hpp>
#include <toolkit/base/TypeTraits.hpp>

namespace toolkit
{

    class Color;

    // Representation of RGBa colors in 32 bit format (8 bits per component in RGBa order).
    struct TK_API Color32
    {
    public:
        union
        {
            struct
            {
                byte r;
                byte g;
                byte b;
                byte a;
            };
            u32 raw;
        };

        static Color32 Transparent;
        static Color32 Black;
        static Color32 White;

        Color32() = default;
        // Constructs a new Color32 with given r, g, b, a components.
        Color32(byte r, byte g, byte b, byte a)
        {
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }

        explicit Color32(const Color &c);
        explicit Color32(const Vec4 &c);

        TkString toString() const;
        TkString toHexString() const;

        bool operator==(const Color32 &other) const
        {
            return raw == other.raw;
        }

        bool operator!=(const Color32 &other) const
        {
            return raw != other.raw;
        }

        Color32 operator+(const Color32 &b) const
        {
            return Color32(r + b.r, g + b.g, this->b + b.b, a + b.a);
        }

        Color32 operator-(const Color32 &b) const
        {
            return Color32(r - b.r, g - b.g, this->b - b.b, a - b.a);
        }

        Color32 operator*(const Color32 &b) const
        {
            return Color32(r * b.r, g * b.g, this->b * b.b, a * b.a);
        }

        Color32 &operator+=(const Color32 &b)
        {
            r += b.r;
            g += b.g;
            this->b += b.b;
            a += b.a;
            return *this;
        }

        Color32 &operator-=(const Color32 &b)
        {
            r -= b.r;
            g -= b.g;
            this->b -= b.b;
            a -= b.a;
            return *this;
        }

        Color32 &operator*=(const Color32 &b)
        {
            r *= b.r;
            g *= b.g;
            this->b *= b.b;
            a *= b.a;
            return *this;
        }

        Color32 operator*(float b) const
        {
            return Color32((byte)((float)r * b), (byte)((float)g * b), (byte)((float)this->b * b), (byte)((float)a * b));
        }

        // Returns true if color is fully transparent (all components are equal zero).
        bool isTransparent() const
        {
            return raw == 0;
        }

        // Returns true if color has opacity channel in use (different from 255).
        bool hasOpacity() const
        {
            return a != 255;
        }

        u32 getAsABGR() const
        {
            return (r << 24) + (g << 16) + (b << 8) + a;
        }

        u32 getAsBGRA() const
        {
            return (a << 24) + (r << 16) + (g << 8) + b;
        }

        u32 getAsARGB() const
        {
            return (b << 24) + (g << 16) + (r << 8) + a;
        }

        u32 getAsRGB() const
        {
            return (r << 16) + (g << 8) + b;
        }

        u32 getAsRGBA() const
        {
            return raw;
        }

        u32 getHashCode() const;

        static u32 getHashCode(const Color32 &v)
        {
            return v.getHashCode();
        }

        // Initializes from packed RGb value of the color and separate alpha channel value.
        static Color32 fromRGB(u32 rgb, byte a = 255)
        {
            return Color32(static_cast<byte>(rgb >> 16 & 0xff), static_cast<byte>(rgb >> 8 & 0xff), static_cast<byte>(rgb & 0xff), a);
        }

        static Color32 fromRGBA(float r, float g, float b, float a)
        {
            return Color32(static_cast<byte>(r * 255), static_cast<byte>(g * 255), static_cast<byte>(b * 255), static_cast<byte>(a * 255));
        }

        // Gets random color with opaque alpha.
        static Color32 random();

        // Linearly interpolates between colors a and b by normalized weight t.
        static Color32 Lerp(const Color32 &a, const Color32 &b, const float t)
        {
            return Color32((byte)(a.r + (b.r - a.r) * t), (byte)(a.g + (b.g - a.g) * t), (byte)(a.b + (b.b - a.b) * t), (byte)(a.a + (b.a - a.a) * t));
        }
    };

    inline Color32 operator*(float a, const Color32 &b)
    {
        return b * a;
    }

    TK_FORCE_INLINE static bool nearEqual(const Color32 &a, const Color32 &b)
    {
        return a.raw == b.raw;
    }

    template <>
    struct TIsPODType<Color32>
    {
        enum
        {
            Value = true
        };
    };

    inline u32 getHash(const Color32 &key)
    {
        return key.getHashCode();
    }

}

FORMAT_CUSTOM(toolkit::Color32, "R:%d G:%d B:%d A:%d", t.r, t.g, t.b, t.a);
