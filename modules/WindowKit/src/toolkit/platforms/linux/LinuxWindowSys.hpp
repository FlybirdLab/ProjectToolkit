/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_LINUX

#include <toolkit/platforms/unix/linux/IncludeX11.hpp>

namespace toolkit
{
    class LinuxWindowSys
    {
    public:
        static X11::Display* s_Display;
        static X11::Atom s_DeleteWindow;
        static X11::Atom s_NetWmState;
        static X11::Atom s_NetWmStateAbove;
        static X11::Atom s_NetWmStateFullscreen;
        static X11::Atom s_NetWmStateMaximizedHorz;
        static X11::Atom s_NetWmStateMaximizedVert;
        static X11::Atom s_NetWmStateSkipTaskbar;
        static X11::Atom s_NetWmStateSkipPager;
        static X11::Atom s_NetWmStateDemandsAttention;
        static X11::Atom s_NetActiveWindow;
        static X11::Atom s_WmChangeState;
        static X11::Atom s_NetWmPid;
        static X11::Atom s_NetWmName;
        static X11::Atom s_Utf8String;
        static X11::Atom s_WmWindowOpacity;
        static X11::Atom s_MotifWmHints;
        static X11::Atom s_NetWmWindowType;
        static X11::Atom s_NetWmWindowTypeDock;
        static X11::Atom s_XdndAware;

        static X11::Display* display();
        static X11::Window rootWindow();
        static void* toHandle(X11::Window window);
    };
}

#endif
