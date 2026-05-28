/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_LINUX

#include <toolkit/input/InputSystem.hpp>
#include <toolkit/platforms/linux/LinuxX11.hpp>

namespace toolkit
{
    class LinuxInput
    {
    public:
        static void init();
        static void update();
        static bool handleEvent(TkWindow *window, X11::XEvent &event);
    };
}

#endif
