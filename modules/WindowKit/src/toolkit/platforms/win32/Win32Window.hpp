/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_WINDOWS

#include <toolkit/windowing/Window.hpp>
#include <toolkit/platforms/win32/IncludeWindows.hpp>

namespace toolkit
{
    class NativeWindow
    {
        friend class TkWindow;

    public:
        NativeWindow(const WindowParams &p, TkWindow *window);
        ~NativeWindow();

    public:
        // Get the window handle.
        TK_FORCE_INLINE HWND getHWND() const
        {
            return m_handle;
        }

        // Check if the window has valid handle created.
        TK_FORCE_INLINE bool hasHWND() const
        {
            return m_handle != nullptr;
        }

        // Get the information about screen which contains window.
        void getScreenInfo(i32 &x, i32 &y, i32 &width, i32 &height) const;

    public:
        // The Windows messages procedure.
        LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        void checkForWindowResize();
        void updateCursor();
        void updateRegion();

    private:
        TkWindow *m_window;
        HWND m_handle;
        bool m_isResizing = false;
        bool m_isSwitchingFullScreen = false;
        bool m_trackingMouse = false;
        bool m_clipCursorSet = false;
        bool m_lastCursorHidden = false;
        int m_cursorHiddenSafetyCount = 0;
        bool m_isDuringMax = false;
        HANDLE m_monitor = nullptr;
        LONG m_clipCursorRect[4];
        i32 m_regionWidth = 0, _regionHeight = 0;
        Pos m_minScreenPos = {0, 0};
    };
}

#endif
