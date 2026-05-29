/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_LINUX

#include <toolkit/platforms/unix/linux/IncludeX11.hpp>
#include <toolkit/windowing/Window.hpp>

namespace toolkit
{
    class NativeWindow
    {
        friend class TkWindow;

    public:
        NativeWindow(const WindowParams& p, TkWindow* window);
        ~NativeWindow();

    public:
        TK_FORCE_INLINE X11::Window getXWindow() const
        {
            return m_handle;
        }

        TK_FORCE_INLINE bool hasHandle() const
        {
            return m_handle != 0;
        }

        void getScreenInfo(i32& x, i32& y, i32& width, i32& height) const;
        bool processEvent(X11::XEvent& event);

    private:
        void checkForWindowResize();
        void sendNetWmState(X11::Atom state, bool enable, X11::Atom state2 = 0);
        void setMotifDecorations(bool enabled);
        void updateCursor();
        bool isWindowMapped() const;

    private:
        TkWindow* m_window = nullptr;
        X11::Window m_handle = 0;
        X11::Cursor m_cursor = 0;
        bool m_clipCursorSet = false;
        bool m_hasFullscreenRestore = false;
        Rect m_restoreBounds{};
        f32 m_opacity = 1.0f;
    };
}

#endif
