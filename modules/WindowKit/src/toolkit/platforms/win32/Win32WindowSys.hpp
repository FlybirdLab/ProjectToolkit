/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/windowing/WindowSystem.hpp>

#if TK_WINDOWS

namespace toolkit {

    class Win32WindowSys 
    {
    public:
        // Win32 application windows class name.
        static const wchar *s_AppWindowClass;
        // Handle to Win32 application instance.
        static void *s_Instance;
    };
}

#endif