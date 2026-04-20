/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/input/InputSystem.hpp>

#if TK_WINDOWS

#include <toolkit/platforms/win32/IncludeWindows.hpp>

namespace toolkit
{ 
    // Windows platform specific implementation of the input system parts. Handles XInput devices.
    class Win32Input
    {
    public:
        static void init();
        static void update();
        static bool wndProc(TkWindow *window, UINT msg, WPARAM wParam, LPARAM lParam);
    };

}

#endif