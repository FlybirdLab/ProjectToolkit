// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include <toolkit/platforms/win32/Win32.hpp>
#include <toolkit/platforms/win32/IncludeWindows.hpp>

namespace toolkit
{

    static void initConsoleUtf8()
    {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }

    static const bool _console_utf8_inited = []
    {
        initConsoleUtf8();
        return true;
    }();

}