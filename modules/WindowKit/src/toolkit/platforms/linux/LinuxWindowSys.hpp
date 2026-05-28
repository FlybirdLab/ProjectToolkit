/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_LINUX

#include <toolkit/platforms/linux/LinuxX11.hpp>
#include <toolkit/windowing/WindowSystem.hpp>

namespace toolkit
{
    class LinuxWindowSys
    {
    public:
        static X11::Display *getDisplay();
        static X11::Atom getDeleteWindowAtom();
        static X11::Atom getWmStateAtom();
        static X11::Atom getWmStateFullscreenAtom();
    };
}

#endif
