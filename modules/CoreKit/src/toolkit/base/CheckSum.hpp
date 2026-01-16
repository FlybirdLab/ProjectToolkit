/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{
    class TkCheckSum final
    {
    public:
        static void init();
        static u32 memCrc32(const void *data, i32 length, u32 crc = 0);
    };
}