/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/math/common.hpp>
#include <toolkit/base/Math.hpp>
#include <toolkit/base/Format.hpp>

namespace toolkit
{

    struct Color32;

    // Representation of the RGBA color.
    class TK_API Color
    {
    public:
        static Color Transparent;
        static Color AliceBlue;
        static Color AntiqueWhite;
        static Color Aqua;
        static Color Aquamarine;
        static Color Azure;
        static Color Beige;
        static Color Bisque;
        static Color Black;
        static Color BlanchedAlmond;
        static Color Blue;
        static Color BlueViolet;
        static Color Brown;
        static Color BurlyWood;
        static Color CadetBlue;
        static Color Chartreuse;
        static Color Chocolate;
        static Color Coral;
        static Color CornflowerBlue;
        static Color Cornsilk;
        static Color Crimson;
        static Color Cyan;
        static Color DarkBlue;
        static Color DarkCyan;
        static Color DarkGoldenrod;
        static Color DarkGray;
        static Color DarkGreen;
        static Color DarkKhaki;
        static Color DarkMagenta;
        static Color DarkOliveGreen;
        static Color DarkOrange;
        static Color DarkOrchid;
        static Color DarkRed;
        static Color DarkSalmon;
        static Color DarkSeaGreen;
        static Color DarkSlateBlue;
        static Color DarkSlateGray;
        static Color DarkTurquoise;
        static Color DarkViolet;
        static Color DeepPink;
        static Color DeepSkyBlue;
        static Color DimGray;
        static Color DodgerBlue;
        static Color Firebrick;
        static Color FloralWhite;
        static Color ForestGreen;
        static Color Fuchsia;
        static Color Gainsboro;
        static Color GhostWhite;
        static Color Gold;
        static Color Goldenrod;
        static Color Gray;
        static Color Green;
        static Color GreenYellow;
        static Color Honeydew;
        static Color HotPink;
        static Color IndianRed;
        static Color Indigo;
        static Color Ivory;
        static Color Khaki;
        static Color Lavender;
        static Color LavenderBlush;
        static Color LawnGreen;
        static Color LemonChiffon;
        static Color LightBlue;
        static Color LightCoral;
        static Color LightCyan;
        static Color LightGoldenrodYellow;
        static Color LightGreen;
        static Color LightGray;
        static Color LightPink;
        static Color LightSalmon;
        static Color LightSeaGreen;
        static Color LightSkyBlue;
        static Color LightSlateGray;
        static Color LightSteelBlue;
        static Color LightYellow;
        static Color Lime;
        static Color LimeGreen;
        static Color Linen;
        static Color Magenta;
        static Color Maroon;
        static Color MediumAquamarine;
        static Color MediumBlue;
        static Color MediumOrchid;
        static Color MediumPurple;
        static Color MediumSeaGreen;
        static Color MediumSlateBlue;
        static Color MediumSpringGreen;
        static Color MediumTurquoise;
        static Color MediumVioletRed;
        static Color MidnightBlue;
        static Color MintCream;
        static Color MistyRose;
        static Color Moccasin;
        static Color NavajoWhite;
        static Color Navy;
        static Color OldLace;
        static Color Olive;
        static Color OliveDrab;
        static Color Orange;
        static Color OrangeRed;
        static Color Orchid;
        static Color PaleGoldenrod;
        static Color PaleGreen;
        static Color PaleTurquoise;
        static Color PaleVioletRed;
        static Color PapayaWhip;
        static Color PeachPuff;
        static Color Peru;
        static Color Pink;
        static Color Plum;
        static Color PowderBlue;
        static Color Purple;
        static Color Red;
        static Color RosyBrown;
        static Color RoyalBlue;
        static Color SaddleBrown;
        static Color Salmon;
        static Color SandyBrown;
        static Color SeaGreen;
        static Color SeaShell;
        static Color Sienna;
        static Color Silver;
        static Color SkyBlue;
        static Color SlateBlue;
        static Color SlateGray;
        static Color Snow;
        static Color SpringGreen;
        static Color SteelBlue;
        static Color Tan;
        static Color Teal;
        static Color Thistle;
        static Color Tomato;
        static Color Turquoise;
        static Color Violet;
        static Color Wheat;
        static Color White;
        static Color WhiteSmoke;
        static Color Yellow;
        static Color YellowGreen;

    public:
        Color() = default;
        explicit Color(float rgba)
            : r(rgba), g(rgba), b(rgba), a(rgba) {}
        TK_FORCE_INLINE Color(float r, float g, float b, float a = 1)
            : r(r), g(g), b(b), a(a) {}
        Color(const Color &rgb, float a)
            : r(rgb.r), g(rgb.g), b(rgb.b), a(a) {}
        Color(const FVec3 &rgb, float a);
        explicit Color(const FVec4 &rgba);
        explicit Color(const Color32 &color);

    public:
        // Initializes from values in range [0;255].
        static Color fromBytes(byte r, byte g, byte b, byte a = 255)
        {
            return Color((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
        }

        // Initializes from packed RGB value (bottom bits contain Blue) of the color and separate alpha channel value.
        static Color fromRGB(u32 rgb, float a = 1.0f)
        {
            return Color(static_cast<float>(rgb >> 16 & 0xff) / 255.0f, static_cast<float>(rgb >> 8 & 0xff) / 255.0f, static_cast<float>(rgb & 0xff) / 255.0f, a);
        }

        // Initializes from packed ARGB value (bottom bits contain Blue).
        static Color fromARGB(u32 argb)
        {
            return Color((float)((argb >> 16) & 0xff) / 255.0f, (float)((argb >> 8) & 0xff) / 255.0f, (float)((argb >> 0) & 0xff) / 255.0f, (float)((argb >> 24) & 0xff) / 255.0f);
        }

        // Initializes from packed RGBA value (bottom bits contain Alpha).
        static Color fromRGBA(u32 rgba)
        {
            return Color((float)((rgba >> 24) & 0xff) / 255.0f, (float)((rgba >> 16) & 0xff) / 255.0f, (float)((rgba >> 8) & 0xff) / 255.0f, (float)((rgba >> 0) & 0xff) / 255.0f);
        }

        static Color fromHex(const TkString &hex)
        {
            bool isValid;
            return fromHex(hex, isValid);
        }

        static Color fromHex(const TkString &hex, bool &isValid);
        // Creates RGB color from Hue[0-360], Saturation[0-1] and Value[0-1].
        static Color fromHSV(float hue, float saturation, float value, float alpha = 1.0f);
        // Creates RGB color from Hue[0-360], Saturation[0-1] and Value[0-1] packed to XYZ vector.
        static Color fromHSV(const FVec3 &hsv, float alpha = 1.0f);
        // Gets random color with opaque alpha.
        static Color random();

    public:
        TkString toString() const;
        TkString toHexString() const;

    public:
        bool operator==(const Color &other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        bool operator!=(const Color &other) const
        {
            return r != other.r || g != other.g || b != other.b || a != other.a;
        }

        Color operator+(const Color &b) const
        {
            return Color(r + b.r, g + b.g, this->b + b.b, a + b.a);
        }

        Color operator-(const Color &b) const
        {
            return Color(r - b.r, g - b.g, this->b - b.b, a - b.a);
        }

        TK_FORCE_INLINE Color operator*(const Color &b) const
        {
            return Color(r * b.r, g * b.g, this->b * b.b, a * b.a);
        }

        Color &operator+=(const Color &b)
        {
            r += b.r;
            g += b.g;
            this->b += b.b;
            a += b.a;
            return *this;
        }

        Color &operator-=(const Color &b)
        {
            r -= b.r;
            g -= b.g;
            this->b -= b.b;
            a -= b.a;
            return *this;
        }

        Color &operator*=(const Color &b)
        {
            r *= b.r;
            g *= b.g;
            this->b *= b.b;
            a *= b.a;
            return *this;
        }

        Color &operator*=(const float b)
        {
            r = r * b;
            g = g * b;
            this->b = this->b * b;
            a = a * b;
            return *this;
        }

        Color operator+(float b) const
        {
            return Color(r + b, g + b, this->b + b, a + b);
        }

        Color operator-(float b) const
        {
            return Color(r - b, g - b, this->b - b, a - b);
        }

        Color operator*(float b) const
        {
            return Color(r * b, g * b, this->b * b, a * b);
        }

        Color operator/(float b) const
        {
            return Color(r / b, g / b, this->b / b, a / b);
        }

    public:
        // Returns true if color is fully transparent (all components are equal zero).
        bool isTransparent() const;
        // Returns true if color has opacity channel in use (different from 1).
        bool hasOpacity() const;

        static bool nearEqual(const Color &a, const Color &b);
        static bool nearEqual(const Color &a, const Color &b, float epsilon);

        // Get color as vector structure.
        FVec3 toFVec3() const;
        // Get color as vector structure.
        FVec4 toFVec4() const;
        // Gets Hue[0-360], Saturation[0-1] and Value[0-1] from RGB color.
        FVec3 toHSV() const;
        // Performs a linear interpolation between two colors.
        static void lerp(const Color &start, const Color &end, float amount, Color &result);
        // Performs a linear interpolation between two colors.
        static Color lerp(const Color &start, const Color &end, float amount);
        // Converts a [0.0, 1.0] linear value into a [0.0, 1.0] sRGB value.
        static Color linearToSrgb(const Color &linear);
        // Converts a [0.0, 1.0] sRGB value into a [0.0, 1.0] linear value.
        static Color srgbToLinear(const Color &srgb);
        // Returns the color with RGB channels multiplied by the given scale factor. The alpha channel remains the same.
        Color rgbMultiplied(float multiplier) const
        {
            return Color(r * multiplier, g * multiplier, b * multiplier, a);
        }
        // Returns the color with RGB channels multiplied by the given color. The alpha channel remains the same.
        Color rgbMultiplied(Color multiplier) const
        {
            return Color(r * multiplier.r, g * multiplier.g, b * multiplier.b, a);
        }
        // Returns the color with alpha channel multiplied by the given scale factor. The RGB channels remain the same.
        Color alphaMultiplied(float multiplier) const
        {
            return Color(r, g, b, a * multiplier);
        }

    public:
        u32 getHashCode() const;

        static u32 getHashCode(const Color &v)
        {
            return v.getHashCode();
        }

    public:
        union
        {
            struct
            {
                float r;
                float g;
                float b;
                float a;
            };
            float raw[4];
        };
    };

    inline Color operator+(float a, const Color &b)
    {
        return b + a;
    }

    inline Color operator*(float a, const Color &b)
    {
        return b * a;
    }

    TK_FORCE_INLINE static bool nearEqual(const Color &a, const Color &b)
    {
        return Color::nearEqual(a, b);
    }

    template <>
    struct TIsPODType<Color>
    {
        enum
        {
            Value = true
        };
    };

    inline u32 getHash(const Color &key)
    {
        return key.getHashCode();
    }

}

FORMAT_CUSTOM(toolkit::Color, "R:%g G:%g B:%g A:%g", t.r, t.g, t.b, t.a);
