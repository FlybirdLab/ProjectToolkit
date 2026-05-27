/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Color.hpp"

#include <cstdlib>

#include "toolkit/math/Vector3.hpp"
#include "toolkit/math/Vector4.hpp"
#include "Color32.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    static_assert(sizeof(Color) == 16, "Invalid Color type size.");

    Color::Color(const FVec3 &rgb, float a)
        : r(rgb.x), g(rgb.y), b(rgb.z), a(a)
    {
    }

    Color::Color(const FVec4 &rgba)
        : r(rgba.x), g(rgba.y), b(rgba.z), a(rgba.w)
    {
    }

    Color::Color(const Color32 &color)
        : r(color.r / 255.0f), g(color.g / 255.0f), b(color.b / 255.0f), a(color.a / 255.0f)
    {
    }

    Color Color::fromHex(const TkString &hex, bool &isValid)
    {
        i32 r, g, b, a = 255;
        isValid = true;
        i32 startIndex = !hex.isEmpty() && hex[0] == char('#') ? 1 : 0;
        if (hex.length() == 3 + startIndex)
        {
            r = TkStringUtils::hexDigit(hex[startIndex++]);
            g = TkStringUtils::hexDigit(hex[startIndex++]);
            b = TkStringUtils::hexDigit(hex[startIndex]);

            r = (r << 4) + r;
            g = (g << 4) + g;
            b = (b << 4) + b;
        }
        else if (hex.length() == 6 + startIndex)
        {
            r = (TkStringUtils::hexDigit(hex[startIndex + 0]) << 4) + TkStringUtils::hexDigit(hex[startIndex + 1]);
            g = (TkStringUtils::hexDigit(hex[startIndex + 2]) << 4) + TkStringUtils::hexDigit(hex[startIndex + 3]);
            b = (TkStringUtils::hexDigit(hex[startIndex + 4]) << 4) + TkStringUtils::hexDigit(hex[startIndex + 5]);
        }
        else if (hex.length() == 8 + startIndex)
        {
            r = (TkStringUtils::hexDigit(hex[startIndex + 0]) << 4) + TkStringUtils::hexDigit(hex[startIndex + 1]);
            g = (TkStringUtils::hexDigit(hex[startIndex + 2]) << 4) + TkStringUtils::hexDigit(hex[startIndex + 3]);
            b = (TkStringUtils::hexDigit(hex[startIndex + 4]) << 4) + TkStringUtils::hexDigit(hex[startIndex + 5]);
            a = (TkStringUtils::hexDigit(hex[startIndex + 6]) << 4) + TkStringUtils::hexDigit(hex[startIndex + 7]);
        }
        else
        {
            r = g = b = 0;
            isValid = false;
        }
        return fromBytes(r, g, b, a);
    }

    Color Color::fromHSV(float hue, float saturation, float value, float alpha)
    {
        const float hDiv60 = hue / 60.0f;
        const float hDiv60Floor = TkMath::floor(hDiv60);
        const float hDiv60Fraction = hDiv60 - hDiv60Floor;

        const float rgbValues[4] =
            {
                value,
                value * (1.0f - saturation),
                value * (1.0f - hDiv60Fraction * saturation),
                value * (1.0f - (1.0f - hDiv60Fraction) * saturation),
            };
        const i32 rgbSwizzle[6][3]{
            {0, 3, 1},
            {2, 0, 1},
            {1, 0, 3},
            {1, 2, 0},
            {3, 1, 0},
            {0, 1, 2},
        };
        const i32 swizzleIndex = (i32)hDiv60Floor % 6;

        return Color(rgbValues[rgbSwizzle[swizzleIndex][0]],
                     rgbValues[rgbSwizzle[swizzleIndex][1]],
                     rgbValues[rgbSwizzle[swizzleIndex][2]],
                     alpha);
    }

    Color Color::fromHSV(const FVec3 &hsv, float alpha)
    {
        return fromHSV(hsv.x, hsv.y, hsv.z, alpha);
    }

    Color Color::random()
    {
        return fromRGB(std::rand(), 1);
    }

    TkString Color::toString() const
    {
        return TkString::format("{}", *this);
    }

    TkString Color::toHexString() const
    {
        static const char *digits = "0123456789ABCDEF";

        const byte r = static_cast<byte>(this->r * kMaxU8);
        const byte g = static_cast<byte>(this->g * kMaxU8);
        const byte b = static_cast<byte>(this->b * kMaxU8);
        const byte a = static_cast<byte>(this->a * kMaxU8);

        char result[8];

        result[0] = digits[r >> 4 & 0x0f];
        result[1] = digits[r & 0x0f];

        result[2] = digits[g >> 4 & 0x0f];
        result[3] = digits[g & 0x0f];

        result[4] = digits[b >> 4 & 0x0f];
        result[5] = digits[b & 0x0f];

        result[6] = digits[a >> 4 & 0x0f];
        result[7] = digits[a & 0x0f];

        return TkString(result, 8);
    }

    bool Color::isTransparent() const
    {
        return TkMath::isZero(r + g + b + a);
    }

    bool Color::hasOpacity() const
    {
        return !TkMath::isOne(a);
    }

    bool Color::nearEqual(const Color &a, const Color &b)
    {
        return TkMath::nearEqual(a.r, b.r) && TkMath::nearEqual(a.g, b.g) && TkMath::nearEqual(a.b, b.b) && TkMath::nearEqual(a.a, b.a);
    }

    bool Color::nearEqual(const Color &a, const Color &b, float epsilon)
    {
        return TkMath::nearEqual(a.r, b.r, epsilon) && TkMath::nearEqual(a.g, b.g, epsilon) && TkMath::nearEqual(a.b, b.b, epsilon) && TkMath::nearEqual(a.a, b.a, epsilon);
    }

    FVec3 Color::toFVec3() const
    {
        return FVec3(r, g, b);
    }

    FVec4 Color::toFVec4() const
    {
        return FVec4(r, g, b, a);
    }

    FVec3 Color::toHSV() const
    {
        const float rgbMin = TkMath::min(r, g, b);
        const float rgbMax = TkMath::max(r, g, b);
        const float rgbRange = rgbMax - rgbMin;
        const float hue = rgbMax == rgbMin ? 0.0f : rgbMax == r ? TkMath::mod((g - b) / rgbRange * 60.0f + 360.0f, 360.0f)
                                                : rgbMax == g   ? (b - r) / rgbRange * 60.0f + 120.0f
                                                : rgbMax == b   ? (r - g) / rgbRange * 60.0f + 240.0f
                                                                : 0.0f;
        const float saturation = rgbMax == 0.0f ? 0.0f : rgbRange / rgbMax;
        const float value = rgbMax;
        return FVec3(hue, saturation, value);
    }

    void Color::lerp(const Color &start, const Color &end, float amount, Color &result)
    {
        result.r = TkMath::lerp(start.r, end.r, amount);
        result.g = TkMath::lerp(start.g, end.g, amount);
        result.b = TkMath::lerp(start.b, end.b, amount);
        result.a = TkMath::lerp(start.a, end.a, amount);
    }

    Color Color::lerp(const Color &start, const Color &end, float amount)
    {
        Color result;
        lerp(start, end, amount, result);
        return result;
    }

    Color Color::linearToSrgb(const Color &linear)
    {
#define LINEAR_TO_SRGB(value) value < 0.00313067f ? value * 12.92f : TkMath::pow(value, (1.0f / 2.4f)) * 1.055f - 0.055f
        return Color(LINEAR_TO_SRGB(linear.r), LINEAR_TO_SRGB(linear.g), LINEAR_TO_SRGB(linear.b), LINEAR_TO_SRGB(linear.a));
#undef LINEAR_TO_SRGB
    }

    Color Color::srgbToLinear(const Color &srgb)
    {
#define SRGB_TO_LINEAR(value) value < 0.04045f ? value / 12.92f : TkMath::pow((value + 0.055f) / 1.055f, 2.4f)
        return Color(SRGB_TO_LINEAR(srgb.r), SRGB_TO_LINEAR(srgb.g), SRGB_TO_LINEAR(srgb.b), SRGB_TO_LINEAR(srgb.a));
#undef LINEAR_TO_SRGB
    }

    u32 Color::getHashCode() const
    {
        const i32 range = 100000;
        i32 hashCode = (i32)r * range;
        hashCode = hashCode * 397 ^ (i32)(g * range);
        hashCode = hashCode * 397 ^ (i32)(b * range);
        hashCode = hashCode * 397 ^ (i32)(a * range);
        return hashCode;
    }

    Color Color::Transparent = Color(0, 0, 0, 0);
    Color Color::AliceBlue = fromRGB(0xF0F8FF);
    Color Color::AntiqueWhite = fromRGB(0xFAEBD7);
    Color Color::Aqua = fromRGB(0x00FFFF);
    Color Color::Aquamarine = fromRGB(0x7FFFD4);
    Color Color::Azure = fromRGB(0xF0FFFF);
    Color Color::Beige = fromRGB(0xF5F5DC);
    Color Color::Bisque = fromRGB(0xFFE4C4);
    Color Color::Black = fromRGB(0x000000);
    Color Color::BlanchedAlmond = fromRGB(0xFFEBCD);
    Color Color::Blue = fromRGB(0x0000FF);
    Color Color::BlueViolet = fromRGB(0x8A2BE2);
    Color Color::Brown = fromRGB(0xA52A2A);
    Color Color::BurlyWood = fromRGB(0xDEB887);
    Color Color::CadetBlue = fromRGB(0x5F9EA0);
    Color Color::Chartreuse = fromRGB(0x7FFF00);
    Color Color::Chocolate = fromRGB(0xD2691E);
    Color Color::Coral = fromRGB(0xFF7F50);
    Color Color::CornflowerBlue = fromRGB(0x6495ED);
    Color Color::Cornsilk = fromRGB(0xFFF8DC);
    Color Color::Crimson = fromRGB(0xDC143C);
    Color Color::Cyan = fromRGB(0x00FFFF);
    Color Color::DarkBlue = fromRGB(0x00008B);
    Color Color::DarkCyan = fromRGB(0x008B8B);
    Color Color::DarkGoldenrod = fromRGB(0xB8860B);
    Color Color::DarkGray = fromRGB(0xA9A9A9);
    Color Color::DarkGreen = fromRGB(0x006400);
    Color Color::DarkKhaki = fromRGB(0xBDB76B);
    Color Color::DarkMagenta = fromRGB(0x8B008B);
    Color Color::DarkOliveGreen = fromRGB(0x556B2F);
    Color Color::DarkOrange = fromRGB(0xFF8C00);
    Color Color::DarkOrchid = fromRGB(0x9932CC);
    Color Color::DarkRed = fromRGB(0x8B0000);
    Color Color::DarkSalmon = fromRGB(0xE9967A);
    Color Color::DarkSeaGreen = fromRGB(0x8FBC8F);
    Color Color::DarkSlateBlue = fromRGB(0x483D8B);
    Color Color::DarkSlateGray = fromRGB(0x2F4F4F);
    Color Color::DarkTurquoise = fromRGB(0x00CED1);
    Color Color::DarkViolet = fromRGB(0x9400D3);
    Color Color::DeepPink = fromRGB(0xFF1493);
    Color Color::DeepSkyBlue = fromRGB(0x00BFFF);
    Color Color::DimGray = fromRGB(0x696969);
    Color Color::DodgerBlue = fromRGB(0x1E90FF);
    Color Color::Firebrick = fromRGB(0xB22222);
    Color Color::FloralWhite = fromRGB(0xFFFAF0);
    Color Color::ForestGreen = fromRGB(0x228B22);
    Color Color::Fuchsia = fromRGB(0xFF00FF);
    Color Color::Gainsboro = fromRGB(0xDCDCDC);
    Color Color::GhostWhite = fromRGB(0xF8F8FF);
    Color Color::Gold = fromRGB(0xFFD700);
    Color Color::Goldenrod = fromRGB(0xDAA520);
    Color Color::Gray = fromRGB(0x808080);
    Color Color::Green = fromRGB(0x008000);
    Color Color::GreenYellow = fromRGB(0xADFF2F);
    Color Color::Honeydew = fromRGB(0xF0FFF0);
    Color Color::HotPink = fromRGB(0xFF69B4);
    Color Color::IndianRed = fromRGB(0xCD5C5C);
    Color Color::Indigo = fromRGB(0x4B0082);
    Color Color::Ivory = fromRGB(0xFFFFF0);
    Color Color::Khaki = fromRGB(0xF0E68C);
    Color Color::Lavender = fromRGB(0xE6E6FA);
    Color Color::LavenderBlush = fromRGB(0xFFF0F5);
    Color Color::LawnGreen = fromRGB(0x7CFC00);
    Color Color::LemonChiffon = fromRGB(0xFFFACD);
    Color Color::LightBlue = fromRGB(0xADD8E6);
    Color Color::LightCoral = fromRGB(0xF08080);
    Color Color::LightCyan = fromRGB(0xE0FFFF);
    Color Color::LightGoldenrodYellow = fromRGB(0xFAFAD2);
    Color Color::LightGreen = fromRGB(0x90EE90);
    Color Color::LightGray = fromRGB(0xD3D3D3);
    Color Color::LightPink = fromRGB(0xFFB6C1);
    Color Color::LightSalmon = fromRGB(0xFFA07A);
    Color Color::LightSeaGreen = fromRGB(0x20B2AA);
    Color Color::LightSkyBlue = fromRGB(0x87CEFA);
    Color Color::LightSlateGray = fromRGB(0x778899);
    Color Color::LightSteelBlue = fromRGB(0xB0C4DE);
    Color Color::LightYellow = fromRGB(0xFFFFE0);
    Color Color::Lime = fromRGB(0x00FF00);
    Color Color::LimeGreen = fromRGB(0x32CD32);
    Color Color::Linen = fromRGB(0xFAF0E6);
    Color Color::Magenta = fromRGB(0xFF00FF);
    Color Color::Maroon = fromRGB(0x800000);
    Color Color::MediumAquamarine = fromRGB(0x66CDAA);
    Color Color::MediumBlue = fromRGB(0x0000CD);
    Color Color::MediumOrchid = fromRGB(0xBA55D3);
    Color Color::MediumPurple = fromRGB(0x9370DB);
    Color Color::MediumSeaGreen = fromRGB(0x3CB371);
    Color Color::MediumSlateBlue = fromRGB(0x7B68EE);
    Color Color::MediumSpringGreen = fromRGB(0x00FA9A);
    Color Color::MediumTurquoise = fromRGB(0x48D1CC);
    Color Color::MediumVioletRed = fromRGB(0xC71585);
    Color Color::MidnightBlue = fromRGB(0x191970);
    Color Color::MintCream = fromRGB(0xF5FFFA);
    Color Color::MistyRose = fromRGB(0xFFE4E1);
    Color Color::Moccasin = fromRGB(0xFFE4B5);
    Color Color::NavajoWhite = fromRGB(0xFFDEAD);
    Color Color::Navy = fromRGB(0x000080);
    Color Color::OldLace = fromRGB(0xFDF5E6);
    Color Color::Olive = fromRGB(0x808000);
    Color Color::OliveDrab = fromRGB(0x6B8E23);
    Color Color::Orange = fromRGB(0xFFA500);
    Color Color::OrangeRed = fromRGB(0xFF4500);
    Color Color::Orchid = fromRGB(0xDA70D6);
    Color Color::PaleGoldenrod = fromRGB(0xEEE8AA);
    Color Color::PaleGreen = fromRGB(0x98FB98);
    Color Color::PaleTurquoise = fromRGB(0xAFEEEE);
    Color Color::PaleVioletRed = fromRGB(0xDB7093);
    Color Color::PapayaWhip = fromRGB(0xFFEFD5);
    Color Color::PeachPuff = fromRGB(0xFFDAB9);
    Color Color::Peru = fromRGB(0xCD853F);
    Color Color::Pink = fromRGB(0xFFC0CB);
    Color Color::Plum = fromRGB(0xDDA0DD);
    Color Color::PowderBlue = fromRGB(0xB0E0E6);
    Color Color::Purple = fromRGB(0x800080);
    Color Color::Red = fromRGB(0xFF0000);
    Color Color::RosyBrown = fromRGB(0xBC8F8F);
    Color Color::RoyalBlue = fromRGB(0x4169E1);
    Color Color::SaddleBrown = fromRGB(0x8B4513);
    Color Color::Salmon = fromRGB(0xFA8072);
    Color Color::SandyBrown = fromRGB(0xF4A460);
    Color Color::SeaGreen = fromRGB(0x2E8B57);
    Color Color::SeaShell = fromRGB(0xFFF5EE);
    Color Color::Sienna = fromRGB(0xA0522D);
    Color Color::Silver = fromRGB(0xC0C0C0);
    Color Color::SkyBlue = fromRGB(0x87CEEB);
    Color Color::SlateBlue = fromRGB(0x6A5ACD);
    Color Color::SlateGray = fromRGB(0x708090);
    Color Color::Snow = fromRGB(0xFFFAFA);
    Color Color::SpringGreen = fromRGB(0x00FF7F);
    Color Color::SteelBlue = fromRGB(0x4682B4);
    Color Color::Tan = fromRGB(0xD2B48C);
    Color Color::Teal = fromRGB(0x008080);
    Color Color::Thistle = fromRGB(0xD8BFD8);
    Color Color::Tomato = fromRGB(0xFF6347);
    Color Color::Turquoise = fromRGB(0x40E0D0);
    Color Color::Violet = fromRGB(0xEE82EE);
    Color Color::Wheat = fromRGB(0xF5DEB3);
    Color Color::White = fromRGB(0xFFFFFF);
    Color Color::WhiteSmoke = fromRGB(0xF5F5F5);
    Color Color::Yellow = fromRGB(0xFFFF00);
    Color Color::YellowGreen = fromRGB(0x9ACD32);
}
