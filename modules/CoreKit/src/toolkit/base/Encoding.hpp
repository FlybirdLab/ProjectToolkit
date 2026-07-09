/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/base/Enum.hpp>

namespace toolkit
{
    // Encoding with in support
    enum class Encoding : u8
    {
        kANSI = 0,
        kUnicode,
        kUnicodeBigEndian,
        kUTF8
    };
    const char* toString(Encoding e);
}