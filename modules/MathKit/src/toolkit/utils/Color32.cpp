/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Color32.hpp"

#include <cstdlib>

#include "Color.hpp"
#include "toolkit/math/Vector4.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    Color32 Color32::Transparent(0, 0, 0, 0);
    Color32 Color32::Black(0, 0, 0, 255);
    Color32 Color32::White(255, 255, 255, 255);

    Color32::Color32(const Color &c)
    {
        r = static_cast<byte>(c.r * 255);
        g = static_cast<byte>(c.g * 255);
        b = static_cast<byte>(c.b * 255);
        a = static_cast<byte>(c.a * 255);
    }

    Color32::Color32(const Vec4 &c)
    {
        r = static_cast<byte>(c.x * 255);
        g = static_cast<byte>(c.y * 255);
        b = static_cast<byte>(c.z * 255);
        a = static_cast<byte>(c.w * 255);
    }

    TkString Color32::toString() const
    {
        return TkString::format("%s", *this);
    }

    TkString Color32::toHexString() const
    {
        static const char *digits = "0123456789ABCDEF";

        char result[6];

        result[0] = digits[r >> 4 & 0x0f];
        result[1] = digits[r & 0x0f];

        result[2] = digits[g >> 4 & 0x0f];
        result[3] = digits[g & 0x0f];

        result[4] = digits[b >> 4 & 0x0f];
        result[5] = digits[b & 0x0f];

        return TkString(result, 6);
    }

    u32 Color32::getHashCode() const
    {
        u32 hashCode = (u32)r;
        hashCode = hashCode * 397 ^ (u32)g;
        hashCode = hashCode * 397 ^ (u32)b;
        hashCode = hashCode * 397 ^ (u32)a;
        return hashCode;
    }

    Color32 Color32::random()
    {
        return fromRGB(std::rand());
    }

}
