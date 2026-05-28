/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_LINUX

#include <toolkit/platforms/linux/LinuxX11.hpp>
#include <toolkit/windowing/Window.hpp>

namespace toolkit
{
    class NativeWindow
    {
        friend class TkWindow;

    public:
        NativeWindow(const WindowParams &p, TkWindow *window);
        ~NativeWindow();

    public:
        TK_FORCE_INLINE X11::Window getXWindow() const
        {
            return m_handle;
        }

        TK_FORCE_INLINE bool hasXWindow() const
        {
            return m_handle != 0;
        }

        void handleEvent(X11::XEvent &event);
        void getScreenInfo(i32 &x, i32 &y, i32 &width, i32 &height) const;

    private:
        void checkForWindowResize();
        void setFullscreenState(bool fullscreen);
        void updateMinMaxHints();

    private:
        TkWindow *m_window = nullptr;
        X11::Window m_handle = 0;
        bool m_destroying = false;
        bool m_visible = false;
        bool m_hasFullscreenRestore = false;
        Rect m_restoreRect = {};
    };
}

#endif
