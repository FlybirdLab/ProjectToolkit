// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Format.hpp"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

namespace toolkit {

    int formatRaw(char* buf, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        int r = stbsp_vsprintf(buf, fmt, args);
        va_end(args);
        return r;
    }

}