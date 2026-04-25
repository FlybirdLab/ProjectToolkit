/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/debug/Profiler.hpp"
#define UNICODE 1
#include "Win32Window.hpp"
#include "toolkit/utils/Convertor.hpp"

#if TK_WINDOWS

#include "toolkit/debug/Logger.hpp"
#include "Win32Input.hpp"
#include "Win32WindowSys.hpp"
#include "toolkit/types/StringView.hpp"
#include <propidl.h>
#include <WinUser.h>

#define DefaultDPI 96

#define WINDOWS_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define WINDOWS_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

// Optional experimental borderless-window paths kept disabled in Toolkit.
#define WINDOWS_USE_NEW_BORDER_LESS 0
#if WINDOWS_USE_NEW_BORDER_LESS
#pragma comment(lib, "Gdi32.lib")
WIN_API HRGN WIN_API_CALLCONV CreateRectRgn(int x1, int y1, int x2, int y2);
#endif
#define WINDOWS_USE_NEWER_BORDER_LESS 0
#if WINDOWS_USE_NEWER_BORDER_LESS
#pragma comment(lib, "dwmapi.lib")
WIN_API HRESULT WIN_API_CALLCONV DwmExtendFrameIntoClientArea(HWND hWnd, const void *pMarInset);
WIN_API HRESULT WIN_API_CALLCONV DwmIsCompositionEnabled(BOOL *pfEnabled);

namespace toolkit
{
    bool isCompositionEnabled()
    {
        BOOL result = FALSE;
        const bool success = ::DwmIsCompositionEnabled(&result) == S_OK;
        return result && success;
    }
}
#endif

namespace toolkit
{
    using GetDpiForWindowProc = UINT(STDAPICALLTYPE *)(HWND hwnd);

    static GetDpiForWindowProc getCachedDpiForWindowProc()
    {
        static GetDpiForWindowProc proc = []() -> GetDpiForWindowProc
        {
            const HMODULE user32Dll = ::GetModuleHandleW(L"user32.dll");
            if (!user32Dll)
                return nullptr;
            return reinterpret_cast<GetDpiForWindowProc>(::GetProcAddress(user32Dll, "GetDpiForWindow"));
        }();
        return proc;
    }

    static u32 queryWindowDpi(HWND hwnd)
    {
        if (auto proc = getCachedDpiForWindowProc())
            return proc(hwnd);

        if (HDC dc = ::GetDC(hwnd))
        {
            const u32 dpi = static_cast<u32>(::GetDeviceCaps(dc, LOGPIXELSX));
            ::ReleaseDC(hwnd, dc);
            return dpi > 0 ? dpi : System::getDpi();
        }

        return System::getDpi();
    }

    static RECT clientRectToWindowRect(HWND hwnd, const Rect &clientRect)
    {
        const float clientWidth = clientRect.w > 1.0f ? clientRect.w : 1.0f;
        const float clientHeight = clientRect.h > 1.0f ? clientRect.h : 1.0f;
        RECT winRect{
            static_cast<LONG>(clientRect.x),
            static_cast<LONG>(clientRect.y),
            static_cast<LONG>(clientRect.x + clientWidth),
            static_cast<LONG>(clientRect.y + clientHeight)};

        WINDOWINFO winInfo;
        TkMemory::memoryClear(&winInfo, sizeof(WINDOWINFO));
        winInfo.cbSize = sizeof(winInfo);
        if (::GetWindowInfo(hwnd, &winInfo))
            ::AdjustWindowRectEx(&winRect, winInfo.dwStyle, FALSE, winInfo.dwExStyle);

        return winRect;
    }

    bool isWindowMaximized(HWND window)
    {
        WINDOWPLACEMENT placement;
        if (!::GetWindowPlacement(window, &placement))
            return false;
        return placement.showCmd == SW_MAXIMIZE;
    }

    NativeWindow::NativeWindow(const WindowParams &p, TkWindow *window)
        : m_window{window}
    {
        i32 x = TkMath::truncToInt(p.pos.x);
        i32 y = TkMath::truncToInt(p.pos.y);
        i32 clientWidth = TkMath::truncToInt(p.size.x);
        i32 clientHeight = TkMath::truncToInt(p.size.y);
        i32 windowWidth = clientWidth;
        i32 windowHeight = clientHeight;
        window->m_clientSize = Pos((float)clientWidth, (float)clientHeight);

        // Setup window style
        u32 style = WS_POPUP, exStyle = 0;
        if (p.supportsTransparency)
            exStyle |= WS_EX_LAYERED;
        if (!p.activateWhenFirstShown)
            exStyle |= WS_EX_NOACTIVATE;
        if (p.showInTaskbar)
            exStyle |= WS_EX_APPWINDOW;
        else
            exStyle |= WS_EX_TOOLWINDOW;
        if (p.isTopmost)
            exStyle |= WS_EX_TOPMOST;
        if (!p.allowInput)
            exStyle |= WS_EX_TRANSPARENT;
        if (p.allowMaximize)
            style |= WS_MAXIMIZEBOX;
        if (p.allowMinimize)
            style |= WS_MINIMIZEBOX;
        if (p.hasSizingFrame)
            style |= WS_THICKFRAME;

        // Check if window should have a border
        if (p.hasBorder)
        {
            // Create window style flags
            style |= WS_OVERLAPPED | WS_SYSMENU | WS_BORDER | WS_CAPTION;
            exStyle |= 0;

            // Adjust window size and positions to take into account window border
            RECT winRect = {0, 0, clientWidth, clientHeight};
            AdjustWindowRectEx(&winRect, style, FALSE, exStyle);
            x += winRect.left;
            y += winRect.top;
            windowWidth = winRect.right - winRect.left;
            windowHeight = winRect.bottom - winRect.top;
        }
        else
        {
            // Create window style flags
            style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
#if WINDOWS_USE_NEW_BORDER_LESS
            if (p.isRegularWindow)
                style |= WS_BORDER | WS_CAPTION | WS_DLGFRAME | WS_SYSMENU | WS_THICKFRAME | WS_GROUP;
#elif WINDOWS_USE_NEWER_BORDER_LESS
            if (p.isRegularWindow)
                style |= WS_THICKFRAME | WS_SYSMENU | WS_CAPTION;
#endif
            exStyle |= WS_EX_WINDOWEDGE;
        }

        // Creating the window
        m_handle = ::CreateWindowExW(
            exStyle,
            L"ToolkitWindow",
            StringAsUTF16<>(p.title.getText()).get(),
            style,
            x,
            y,
            windowWidth,
            windowHeight,
            p.parent ? static_cast<HWND>(p.parent->getWindowPtr()) : nullptr,
            nullptr,
            (HINSTANCE)Win32WindowSys::s_Instance,
            nullptr);
        if (m_handle == nullptr)
        {
            LogError("Cannot create window.");
            return;
        }

        // Query DPI
        m_window->m_dpi = queryWindowDpi(m_handle);
        m_window->m_dpiScale = (float)m_window->m_dpi / (float)DefaultDPI;

#if WINDOWS_USE_NEWER_BORDER_LESS
        // Enable shadow
        if (window->m_params.isRegularWindow && !window->m_params.hasBorder && isCompositionEnabled())
        {
            const int margin[4] = {1, 1, 1, 1};
            ::DwmExtendFrameIntoClientArea(m_handle, margin);
        }
#endif

        updateRegion();
    }

    NativeWindow::~NativeWindow()
    {
        if (m_window != nullptr)
        {
            if (m_window->m_track.isClippingCursor)
                m_window->endClippingCursor();
            if (m_window->m_track.isTrackingMouse)
                m_window->endTrackingMouse();
        }

        HWND iconOwner = m_handle;
        if (hasHWND())
        {
            HWND handle = m_handle;
            m_destroying = true;
            // Destroy window
            if (DestroyWindow(handle) == 0 && IsWindow(handle))
            {
                LogWarn("DestroyWindow failed! Error: 0x%x\n", GetLastError());
            }

            // Clear
            m_handle = nullptr;
            if (m_window != nullptr)
                m_window->m_state.visible = false;
        }

        if (m_bigIcon != nullptr)
        {
            if (iconOwner != nullptr && IsWindow(iconOwner))
                SendMessageW(iconOwner, WM_SETICON, ICON_BIG, 0);
            DestroyIcon(m_bigIcon);
            m_bigIcon = nullptr;
        }
        if (m_smallIcon != nullptr)
        {
            if (iconOwner != nullptr && IsWindow(iconOwner))
                SendMessageW(iconOwner, WM_SETICON, ICON_SMALL, 0);
            DestroyIcon(m_smallIcon);
            m_smallIcon = nullptr;
        }
    }

    void *TkWindow::getWindowPtr() const
    {
        return m_native ? m_native->m_handle : nullptr;
    }

    void TkWindow::show()
    {
        if (m_native == nullptr || !m_native->hasHWND() || m_state.closed)
            return;
        if (!m_state.visible)
        {
            m_state.suppressFocusEvents = true;
            ShowWindow(m_native->m_handle, (m_params.allowInput && m_params.activateWhenFirstShown) ? SW_SHOW : SW_SHOWNA);
            m_state.suppressFocusEvents = false;
#if WINDOWS_USE_NEW_BORDER_LESS
            if (!m_params.hasBorder && m_params.isRegularWindow)
            {
                SetWindowPos(m_native->m_handle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            }
#elif WINDOWS_USE_NEWER_BORDER_LESS
            if (!m_params.hasBorder)
            {
                SetWindowPos(m_native->m_handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
            }
#endif

            if (m_state.visible)
                return;
            m_state.visible = true;
            onShown();
        }
    }

    void TkWindow::hide()
    {
        if (m_native == nullptr || !m_native->hasHWND() || m_state.closed)
            return;
        if (m_state.visible)
        {
            m_state.suppressFocusEvents = true;
            ::ShowWindow(m_native->m_handle, SW_HIDE);
            m_state.suppressFocusEvents = false;

            if (!m_state.visible)
                return;
            endClippingCursor();
            endTrackingMouse();
            m_state.visible = false;
            m_state.showAfterFirstPaint = m_params.showAfterFirstPaint;
            onHidden();
        }
    }

    void TkWindow::minimize()
    {
        if (!m_params.allowMinimize || m_native == nullptr || !m_native->hasHWND())
            return;
        if (isFullscreen())
            return;
        TK_ASSERT(m_native->hasHWND());
        ShowWindow(m_native->m_handle, SW_MINIMIZE);
    }

    void TkWindow::maximize()
    {
        if (!m_params.allowMaximize || m_native == nullptr || !m_native->hasHWND())
            return;
        if (isFullscreen())
            return;
        TK_ASSERT(m_native->hasHWND());
        m_native->m_isDuringMax = true;
        ShowWindow(m_native->m_handle, SW_MAXIMIZE);
        m_native->m_isDuringMax = false;
    }

    void TkWindow::setBorderless(bool isBorderless, bool maximized)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        if (isFullscreen())
            setFullscreen(false);

        // Fixes issue of borderless window not going full screen
        if (isMaximized())
            restore();

        m_params.hasBorder = !isBorderless;

        bringToFront();

        if (isBorderless)
        {
            LONG lStyle = GetWindowLong(m_native->m_handle, GWL_STYLE);
            lStyle &= ~(WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED | WS_BORDER | WS_CAPTION);
            lStyle |= WS_POPUP;
            lStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
#if WINDOWS_USE_NEW_BORDER_LESS
            if (m_params.isRegularWindow)
                lStyle |= WS_BORDER | WS_CAPTION | WS_DLGFRAME | WS_SYSMENU | WS_THICKFRAME | WS_GROUP;
#elif WINDOWS_USE_NEWER_BORDER_LESS
            if (m_params.isRegularWindow)
                lStyle |= WS_THICKFRAME | WS_SYSMENU;
#endif

            SetWindowLong(m_native->m_handle, GWL_STYLE, lStyle);
            SetWindowPos(m_native->m_handle, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

            if (maximized)
            {
                ShowWindow(m_native->m_handle, SW_SHOWMAXIMIZED);
            }
            else
            {
                ShowWindow(m_native->m_handle, SW_SHOW);
            }
        }
        else
        {
            LONG lStyle = GetWindowLong(m_native->m_handle, GWL_STYLE);
            lStyle &= ~(WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
            if (m_params.allowMaximize)
                lStyle |= WS_MAXIMIZEBOX;
            if (m_params.allowMinimize)
                lStyle |= WS_MINIMIZEBOX;
            if (m_params.hasSizingFrame)
                lStyle |= WS_THICKFRAME;
            lStyle |= WS_OVERLAPPED | WS_SYSMENU | WS_BORDER | WS_CAPTION;

            SetWindowLong(m_native->m_handle, GWL_STYLE, lStyle);
            const Pos clientSize = getClientSize();
            const Pos desktopSize = System::getDesktopSize();
            // Move window and half size if it is larger than desktop size
            if (clientSize.x >= desktopSize.x && clientSize.y >= desktopSize.y)
            {
                const Pos halfSize = desktopSize * 0.5f;
                const Pos middlePos = halfSize * 0.5f;
                SetWindowPos(m_native->m_handle, nullptr, (int)middlePos.x, (int)middlePos.y, (int)halfSize.x, (int)halfSize.y, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);
            }
            else
            {
                SetWindowPos(m_native->m_handle, nullptr, 0, 0, (int)clientSize.x, (int)clientSize.y, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }

            if (maximized)
            {
                maximize();
            }
            else
            {
                ShowWindow(m_native->m_handle, SW_SHOW);
            }
        }

        m_native->checkForWindowResize();
    }

    void TkWindow::restore()
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        if (isFullscreen())
            return;
        TK_ASSERT(m_native->hasHWND());
        ::ShowWindow(m_native->m_handle, SW_RESTORE);
    }

    bool TkWindow::isClosed() const
    {
        return m_state.closed || m_native == nullptr || !m_native->hasHWND();
    }

    bool TkWindow::isForegroundWindow() const
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return false;
        return ::GetForegroundWindow() == m_native->m_handle;
    }

    void TkWindow::close(ClosingReason reason)
    {
        // Prevent from calling close during or after close action
        if (m_state.isClosing || m_state.closed)
            return;
        m_state.isClosing = true;

        // Check if can close window
        bool cancel = false;
        m_events.onClosing(reason, cancel);
        if (cancel)
        {
            m_state.isClosing = false;
            return;
        }

        if (m_state.visible)
            hide();
        if (m_native && m_native->hasHWND())
        {
            HWND handle = m_native->m_handle;
            if (::DestroyWindow(handle) == 0 && ::IsWindow(handle))
            {
                LogWarn("DestroyWindow failed during close. Error: 0x%x", GetLastError());
                m_state.isClosing = false;
            }
            else if (!m_state.closed)
            {
                onClosedTrigger();
            }
        }
        else
        {
            onClosedTrigger();
        }
    }

    void TkWindow::bringToFront(bool force)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        if (m_params.isRegularWindow)
        {
            if (IsIconic(m_native->m_handle))
            {
                ShowWindow(m_native->m_handle, SW_RESTORE);
            }
            else
            {
                SetActiveWindow(m_native->m_handle);
            }
        }
        else
        {
            HWND hWndInsertAfter = HWND_TOP;
            u32 flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER;

            if (!force)
            {
                flags |= SWP_NOACTIVATE;
            }

            if (m_params.isTopmost)
            {
                hWndInsertAfter = HWND_TOPMOST;
            }

            SetWindowPos(m_native->m_handle, hWndInsertAfter, 0, 0, 0, 0, flags);
        }
    }

    void TkWindow::setClientBounds(const Rect &clientArea)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        // Check if position or/and size will change
        const auto rect = getClientBounds();
        const bool changeLocation = !Pos::nearEqual(Pos{rect.x, rect.y}, Pos{clientArea.x, clientArea.y});
        const bool changeSize = !Pos::nearEqual(Pos{rect.w, rect.h}, Pos{clientArea.w, clientArea.h});
        if (!changeLocation && !changeSize)
            return;

        const Rect clampedClientArea{
            clientArea.x,
            clientArea.y,
            clientArea.w > 1.0f ? clientArea.w : 1.0f,
            clientArea.h > 1.0f ? clientArea.h : 1.0f};
        const RECT winRect = m_params.hasBorder
            ? clientRectToWindowRect(m_native->m_handle, clampedClientArea)
            : RECT{
                  static_cast<LONG>(clampedClientArea.x),
                  static_cast<LONG>(clampedClientArea.y),
                  static_cast<LONG>(clampedClientArea.x + clampedClientArea.w),
                  static_cast<LONG>(clampedClientArea.y + clampedClientArea.h)};
        const i32 x = winRect.left;
        const i32 y = winRect.top;
        const i32 width = winRect.right - winRect.left;
        const i32 height = winRect.bottom - winRect.top;

        // Change window size and location
        if (!SetWindowPos(m_native->m_handle, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE))
            return;

        if (changeSize)
        {
            m_native->checkForWindowResize();
            m_native->updateRegion();
        }
    }

    void TkWindow::setPos(const Pos &position)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        // Cache data
        i32 x = (i32)position.x;
        i32 y = (i32)position.y;

        // Change window location
        SetWindowPos(m_native->m_handle, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }

    void TkWindow::setClientPos(const Pos &position)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        // Cache data
        i32 x = (i32)position.x;
        i32 y = (i32)position.y;

        // Check if need to adjust window rectangle
        if (m_params.hasBorder)
        {
            const RECT winRect = clientRectToWindowRect(m_native->m_handle, Rect{static_cast<float>(x), static_cast<float>(y), 1.0f, 1.0f});
            x = winRect.left;
            y = winRect.top;
        }

        // Change window location
        SetWindowPos(m_native->m_handle, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }

    bool TkWindow::isFirstWindow() const
    {
        return WindowSystem::getMainWindow() == this;
    }

    bool TkWindow::isFullscreen() const
    {
        return m_state.fullscreen;
    }

    void TkWindow::setFullscreen(bool isFullscreen)
    {
        if (m_native == nullptr || !m_native->hasHWND() || m_state.fullscreen == isFullscreen)
            return;

        m_native->m_isSwitchingFullScreen = true;

        LogInfo("Changing window fullscreen mode to %v", isFullscreen);

        HWND hwnd = m_native->m_handle;
        bool changed = false;
        if (isFullscreen)
        {
            m_native->m_restorePlacement.length = sizeof(WINDOWPLACEMENT);
            SetLastError(ERROR_SUCCESS);
            m_native->m_restoreStyle = GetWindowLong(hwnd, GWL_STYLE);
            const DWORD styleError = GetLastError();
            SetLastError(ERROR_SUCCESS);
            m_native->m_restoreExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            const DWORD exStyleError = GetLastError();
            if (styleError != ERROR_SUCCESS || exStyleError != ERROR_SUCCESS || !GetWindowPlacement(hwnd, &m_native->m_restorePlacement) || !GetWindowRect(hwnd, &m_native->m_restoreRect))
            {
                LogWarn("Failed to capture window state before fullscreen. Error: 0x%x", GetLastError());
                m_native->m_isSwitchingFullScreen = false;
                return;
            }
            m_native->m_hasFullscreenRestore = true;

            HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO monitorInfo;
            monitorInfo.cbSize = sizeof(MONITORINFO);
            if (!GetMonitorInfoW(monitor, &monitorInfo))
            {
                LogWarn("Failed to query monitor info for fullscreen. Error: 0x%x", GetLastError());
                m_native->m_hasFullscreenRestore = false;
                m_native->m_isSwitchingFullScreen = false;
                return;
            }

            LONG style = m_native->m_restoreStyle;
            style &= ~(WS_CAPTION | WS_THICKFRAME | WS_BORDER | WS_DLGFRAME | WS_MAXIMIZEBOX);
            style |= WS_POPUP;
            SetLastError(ERROR_SUCCESS);
            if (SetWindowLong(hwnd, GWL_STYLE, style) == 0 && GetLastError() != ERROR_SUCCESS)
            {
                LogWarn("Failed to set fullscreen window style. Error: 0x%x", GetLastError());
                m_native->m_hasFullscreenRestore = false;
                m_native->m_isSwitchingFullScreen = false;
                return;
            }
            SetLastError(ERROR_SUCCESS);
            if (SetWindowLong(hwnd, GWL_EXSTYLE, m_native->m_restoreExStyle & ~(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)) == 0 && GetLastError() != ERROR_SUCCESS)
            {
                LogWarn("Failed to set fullscreen ex style. Error: 0x%x", GetLastError());
                SetWindowLong(hwnd, GWL_STYLE, m_native->m_restoreStyle);
                m_native->m_hasFullscreenRestore = false;
                m_native->m_isSwitchingFullScreen = false;
                return;
            }
            changed = SetWindowPos(
                hwnd,
                HWND_TOP,
                monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE) != 0;
            if (!changed)
            {
                LogWarn("Failed to enter fullscreen. Error: 0x%x", GetLastError());
                SetWindowLong(hwnd, GWL_STYLE, m_native->m_restoreStyle);
                SetWindowLong(hwnd, GWL_EXSTYLE, m_native->m_restoreExStyle);
                SetWindowPlacement(hwnd, &m_native->m_restorePlacement);
                m_native->m_hasFullscreenRestore = false;
                m_native->m_isSwitchingFullScreen = false;
                return;
            }
        }
        else if (m_native->m_hasFullscreenRestore)
        {
            SetLastError(ERROR_SUCCESS);
            if (SetWindowLong(hwnd, GWL_STYLE, m_native->m_restoreStyle) == 0 && GetLastError() != ERROR_SUCCESS)
            {
                LogWarn("Failed to restore fullscreen window style. Error: 0x%x", GetLastError());
                m_native->m_isSwitchingFullScreen = false;
                return;
            }
            SetLastError(ERROR_SUCCESS);
            if (SetWindowLong(hwnd, GWL_EXSTYLE, m_native->m_restoreExStyle) == 0 && GetLastError() != ERROR_SUCCESS)
            {
                LogWarn("Failed to restore fullscreen ex style. Error: 0x%x", GetLastError());
                m_native->m_isSwitchingFullScreen = false;
                return;
            }
            if (!SetWindowPlacement(hwnd, &m_native->m_restorePlacement))
            {
                LogWarn("Failed to restore fullscreen placement. Error: 0x%x", GetLastError());
                m_native->m_isSwitchingFullScreen = false;
                return;
            }
            changed = SetWindowPos(
                hwnd,
                nullptr,
                m_native->m_restoreRect.left,
                m_native->m_restoreRect.top,
                m_native->m_restoreRect.right - m_native->m_restoreRect.left,
                m_native->m_restoreRect.bottom - m_native->m_restoreRect.top,
                SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE) != 0;
            if (!changed)
            {
                LogWarn("Failed to exit fullscreen. Error: 0x%x", GetLastError());
                m_native->m_isSwitchingFullScreen = false;
                return;
            }
            m_native->m_hasFullscreenRestore = false;
        }
        else
        {
            changed = true;
        }

        if (changed)
        {
            m_state.fullscreen = isFullscreen;
            m_native->checkForWindowResize();
        }

        m_native->m_isSwitchingFullScreen = false;
    }

    bool TkWindow::isVisible() const
    {
        return m_state.visible;
        ;
    }

    void TkWindow::setVisible(bool isVisible)
    {
        // Check if state will change
        if (this->isVisible() != isVisible)
        {
            // Check if show or hide a window
            if (isVisible)
                show();
            else
                hide();
        }
    }

    Pos TkWindow::getPos() const
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return Pos{0, 0};
        TK_ASSERT(m_native->hasHWND());

        RECT rect;
        GetWindowRect(m_native->m_handle, &rect);
        return Pos(static_cast<float>(rect.left), static_cast<float>(rect.top));
    }

    Pos TkWindow::getSize() const
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return Pos{0, 0};
        TK_ASSERT(m_native->hasHWND());

        RECT rect;
        GetWindowRect(m_native->m_handle, &rect);
        return Pos(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));
    }

    Pos TkWindow::getClientSize() const
    {
        return m_clientSize;
    }

    Pos TkWindow::screenToClient(const Pos &screenPos) const
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return Pos{0, 0};
        TK_ASSERT(m_native->hasHWND());

        POINT p;
        p.x = static_cast<LONG>(screenPos.x);
        p.y = static_cast<LONG>(screenPos.y);
        ::ScreenToClient(m_native->m_handle, &p);
        return Pos(static_cast<float>(p.x), static_cast<float>(p.y));
    }

    Pos TkWindow::clientToScreen(const Pos &clientPos) const
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return Pos{0, 0};
        TK_ASSERT(m_native->hasHWND());

        if (m_state.minimized)
        {
            // Return cached position when window is not on screen
            return m_native->m_minScreenPos + clientPos;
        }

        POINT p;
        p.x = static_cast<LONG>(clientPos.x);
        p.y = static_cast<LONG>(clientPos.y);
        ::ClientToScreen(m_native->m_handle, &p);
        return Pos(static_cast<float>(p.x), static_cast<float>(p.y));
    }

    void TkWindow::flashWindow()
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        if (isFocused())
            return;

        ::FlashWindow(m_native->m_handle, FALSE);
    }

    void NativeWindow::getScreenInfo(i32 &x, i32 &y, i32 &width, i32 &height) const
    {
        TK_ASSERT(hasHWND());

        // Pick the current monitor data for sizing
        const HMONITOR monitor = ::MonitorFromWindow(m_handle, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfoW(monitor, &monitorInfo);

        // Calculate result
        x = monitorInfo.rcMonitor.left;
        y = monitorInfo.rcMonitor.top;
        width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
        height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
    }

    float TkWindow::getOpacity() const
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return 1.0f;
        TK_ASSERT(m_native->hasHWND());

        COLORREF color = 0;
        BYTE alpha = 255;
        DWORD flags = 0;
        if (!(GetWindowLong(m_native->m_handle, GWL_EXSTYLE) & WS_EX_LAYERED))
            return 1.0f;
        if (!::GetLayeredWindowAttributes(m_native->m_handle, &color, &alpha, &flags))
            return 1.0f;
        return (float)alpha / 255.0f;
    }

    void TkWindow::setOpacity(f32 opacity)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        LONG exStyle = GetWindowLong(m_native->m_handle, GWL_EXSTYLE);
        if (!(exStyle & WS_EX_LAYERED))
            SetWindowLong(m_native->m_handle, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
        ::SetLayeredWindowAttributes(m_native->m_handle, 0, static_cast<BYTE>(TkMath::saturate(opacity) * 255), LWA_ALPHA);
    }

    void TkWindow::focus()
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());
        if (GetFocus() != m_native->m_handle)
        {
            SetFocus(m_native->m_handle);
        }
    }

    TkString TkWindow::getTitle() const
    {
        return m_params.title;
    }

    void TkWindow::setTitle(const TkStringView &title)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        if (::SetWindowTextW(m_native->m_handle, StringAsUTF16<>(title.get()).get()))
        {
            m_params.title = title;
        }
    }

    void TkWindow::startTrackingMouse(bool useMouseScreenOffset)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        TK_ASSERT(m_native->hasHWND());

        if (!m_track.isTrackingMouse)
        {
            m_track.isTrackingMouse = true;
            m_track.mouseOffset = {0, 0};
            m_track.isUsingMouseOffset = useMouseScreenOffset;
            m_track.isHorizontalFlippingMouse = false;
            m_track.isVerticalFlippingMouse = false;

            i32 x = 0, y = 0, width = 0, height = 0;
            m_native->getScreenInfo(x, y, width, height);
            m_track.mouseOffsetScreenSize = Rect((float)x, (float)y, (float)width, (float)height);

            SetCapture(m_native->m_handle);
        }
    }

    void TkWindow::endTrackingMouse()
    {
        if (m_track.isTrackingMouse)
        {
            m_track.isTrackingMouse = false;
            m_track.isHorizontalFlippingMouse = false;
            m_track.isVerticalFlippingMouse = false;

            ReleaseCapture();
        }
    }

    void TkWindow::startClippingCursor(const Rect &bounds)
    {
        if (m_native == nullptr || !m_native->hasHWND())
            return;
        m_track.isClippingCursor = true;
        *(RECT *)m_native->m_clipCursorRect = {
            (LONG)bounds.getUpperLeft().x,
            (LONG)bounds.getUpperLeft().y,
            (LONG)bounds.getBottomRight().x,
            (LONG)bounds.getBottomRight().y};
        if (isFocused())
        {
            m_native->m_clipCursorSet = true;
            ::ClipCursor((RECT *)m_native->m_clipCursorRect);
        }
    }

    void TkWindow::endClippingCursor()
    {
        if (m_track.isClippingCursor)
        {
            m_track.isClippingCursor = false;
            m_native->m_clipCursorSet = false;
            ::ClipCursor(nullptr);
        }
    }

    void TkWindow::setCursor(CursorType type)
    {
        if (static_cast<i32>(type) < 0 || type >= CursorType::MAX)
            type = CursorType::kDefault;
        // Base
        m_cursor = type;

        if (m_native != nullptr && m_native->hasHWND())
            m_native->updateCursor();
    }

    void TkWindow::setIcon(const TkStringView icon)
    {
        if (m_native == nullptr || !m_native->hasHWND() || !icon.hasChars())
            return;

        const auto path = StringAsUTF16<>(icon.get());
        HICON bigIcon = static_cast<HICON>(LoadImageW(nullptr, path.get(), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_LOADFROMFILE));
        HICON smallIcon = static_cast<HICON>(LoadImageW(nullptr, path.get(), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE));
        if (bigIcon == nullptr && smallIcon == nullptr)
        {
            LogWarn("Failed to load window icon.");
            return;
        }

        if (bigIcon != nullptr)
        {
            HICON old = m_native->m_bigIcon;
            SendMessageW(m_native->m_handle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(bigIcon));
            m_native->m_bigIcon = bigIcon;
            if (old != nullptr)
                DestroyIcon(old);
        }
        if (smallIcon != nullptr)
        {
            HICON old = m_native->m_smallIcon;
            SendMessageW(m_native->m_handle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(smallIcon));
            m_native->m_smallIcon = smallIcon;
            if (old != nullptr)
                DestroyIcon(old);
        }
    }

    void NativeWindow::checkForWindowResize()
    {
        // Skip for minimized window (GetClientRect for minimized window returns 0)
        if (m_window->m_state.minimized)
            return;

        if (!hasHWND())
            return;

        // Cache client size
        RECT rect;
        if (!GetClientRect(m_handle, &rect))
            return;
        i32 width = TkMath::max(rect.right - rect.left, 0L);
        i32 height = TkMath::max(rect.bottom - rect.top, 0L);

        // Check for windows maximized size and see if it needs to adjust position if needed
        if (m_window->m_state.maximized)
        {
            // Pick the current monitor data for sizing
            const HMONITOR monitor = MonitorFromWindow(m_handle, MONITOR_DEFAULTTONEAREST);
            MONITORINFO monitorInfo;
            monitorInfo.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfoW(monitor, &monitorInfo))
            {
                auto cwidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
                auto cheight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
                if (width > cwidth && height > cheight)
                {
                    width = cwidth;
                    height = cheight;
                    SetWindowPos(m_handle, HWND_TOP, monitorInfo.rcWork.left, monitorInfo.rcWork.top, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
                }
            }
        }

        const Pos newSize(static_cast<float>(width), static_cast<float>(height));
        if (!Pos::nearEqual(m_window->m_clientSize, newSize))
        {
            m_window->m_clientSize = newSize;
            m_window->onResizeTrigger(width, height);
        }
    }

    void NativeWindow::updateCursor()
    {
        if (!hasHWND())
            return;

        // Don't hide cursor when window is not focused. Use WM_SETCURSOR/SetCursor only; do not touch ShowCursor's global counter.
        if (m_window->m_cursor == CursorType::kHidden && m_window->m_state.focused)
        {
            ::SetCursor(nullptr);
            return;
        }

        i32 index = 0;
        switch (m_window->m_cursor)
        {
        case CursorType::kDefault:
            break;
        case CursorType::kCross:
            index = 1;
            break;
        case CursorType::kHand:
            index = 2;
            break;
        case CursorType::kHelp:
            index = 3;
            break;
        case CursorType::kIBeam:
            index = 4;
            break;
        case CursorType::kNo:
            index = 5;
            break;
        case CursorType::kWait:
            index = 11;
            break;
        case CursorType::kSizeAll:
            index = 6;
            break;
        case CursorType::kSizeNESW:
            index = 7;
            break;
        case CursorType::kSizeNS:
            index = 8;
            break;
        case CursorType::kSizeNWSE:
            index = 9;
            break;
        case CursorType::kSizeWE:
            index = 10;
            break;
        default:
            index = 0;
            break;
        }

        static const LPCWSTR cursors[] =
            {
                IDC_ARROW,
                IDC_CROSS,
                IDC_HAND,
                IDC_HELP,
                IDC_IBEAM,
                IDC_NO,
                IDC_SIZEALL,
                IDC_SIZENESW,
                IDC_SIZENS,
                IDC_SIZENWSE,
                IDC_SIZEWE,
                IDC_WAIT,
            };

        static HCURSOR cursorCache[lengthOf(cursors)] = {};
        if (cursorCache[index] == nullptr)
            cursorCache[index] = LoadCursorW(nullptr, cursors[index]);
        const HCURSOR cursor = cursorCache[index] != nullptr ? cursorCache[index] : LoadCursorW(nullptr, IDC_ARROW);
        ::SetCursor(cursor);
    }

    void NativeWindow::updateRegion()
    {
#if WINDOWS_USE_NEW_BORDER_LESS
        if (!m_window->m_params.hasBorder && m_window->m_params.isRegularWindow)
        {
            if (!m_window->m_state.maximized && !m_isResizing)
            {
                RECT rcWnd;
                GetWindowRect(m_handle, &rcWnd);
                const i32 width = rcWnd.right - rcWnd.left;
                const i32 height = rcWnd.bottom - rcWnd.top;
                if (_regionWidth != width || _regionHeight != height)
                {
                    _regionWidth = width;
                    _regionHeight = height;
                    const HRGN region = CreateRectRgn(0, 0, width, height);
                    SetWindowRgn(m_handle, region, FALSE);
                }
            }
            else if (_regionWidth != 0 || _regionHeight != 0)
            {
                _regionWidth = 0;
                _regionHeight = 0;
                SetWindowRgn(m_handle, nullptr, FALSE);
            }
        }
#endif
    }

    void TrackMouse(HWND hwnd)
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.dwHoverTime = 5000;
        tme.hwndTrack = hwnd;
        TrackMouseEvent(&tme);
    }

    LRESULT NativeWindow::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        const UINT_PTR MouseStopTimerID = 1;
        switch (msg)
        {
        case WM_PAINT:
        { 
            break;
        }
        case WM_TIMER:
        {
            if (wParam != MouseStopTimerID)
                break;

            // Kill the timer after processing it
            KillTimer(m_handle, MouseStopTimerID);
            return 0;
        }
        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) != HTCLIENT)
                break;

            updateCursor();
            return true;
        }
        case WM_MOUSEMOVE:
        {
            if (!m_trackingMouse)
            {
                TrackMouse(m_handle);
                m_trackingMouse = true;
            }

            if (m_window->m_track.isTrackingMouse)
            {
                KillTimer(m_handle, MouseStopTimerID);
                SetTimer(m_handle, MouseStopTimerID, 100, nullptr);
            }

            // Here we can transfer mouse pointer over virtual workspace
            if (m_window->m_track.isTrackingMouse && m_window->m_track.isUsingMouseOffset)
            {
                // Check if move mouse to another edge of the desktop
                Pos desktopLocation = m_window->m_track.mouseOffsetScreenSize.location;
                Pos desktopSize = m_window->m_track.mouseOffsetScreenSize.getBottomRight();

                const Pos mousePos(static_cast<float>(WINDOWS_GET_X_LPARAM(lParam)), static_cast<float>(WINDOWS_GET_Y_LPARAM(lParam)));
                Pos mousePosition = m_window->clientToScreen(mousePos);
                Pos newMousePosition = mousePosition;
                if ((m_window->m_track.isHorizontalFlippingMouse = mousePosition.x <= desktopLocation.x + 2))
                    newMousePosition.x = desktopSize.x - 3;
                else if ((m_window->m_track.isHorizontalFlippingMouse = mousePosition.x >= desktopSize.x - 1))
                    newMousePosition.x = desktopLocation.x + 3;
                if ((m_window->m_track.isVerticalFlippingMouse = mousePosition.y <= desktopLocation.y + 2))
                    newMousePosition.y = desktopSize.y - 3;
                else if ((m_window->m_track.isVerticalFlippingMouse = mousePosition.y >= desktopSize.y - 1))
                    newMousePosition.y = desktopLocation.y + 3;
                if (!Pos::nearEqual(mousePosition, newMousePosition))
                {
                    m_window->m_track.mouseOffset -= newMousePosition - mousePosition;
                    m_window->setMousePos(m_window->screenToClient(newMousePosition));
                }
            }

            break;
        }
        case WM_MOUSELEAVE:
        {
            m_trackingMouse = false;
            break;
        }
        case WM_NCCALCSIZE:
        {
#if WINDOWS_USE_NEW_BORDER_LESS
            if (wParam && !m_params.hasBorder && m_params.isRegularWindow)
            {
                if (_maximized)
                {
                    WINDOWINFO winInfo = {0};
                    winInfo.cbSize = sizeof(winInfo);
                    ::GetWindowInfo(m_native->m_handle, &winInfo);

                    LPNCCALCSIZE_PARAMS rects = (LPNCCALCSIZE_PARAMS)lParam;

                    rects->rgrc[0].left += winInfo.cxWindowBorders;
                    rects->rgrc[0].top += winInfo.cxWindowBorders;
                    rects->rgrc[0].right -= winInfo.cxWindowBorders;
                    rects->rgrc[0].bottom -= winInfo.cxWindowBorders;

                    rects->rgrc[1].left = rects->rgrc[0].left;
                    rects->rgrc[1].top = rects->rgrc[0].top;
                    rects->rgrc[1].right = rects->rgrc[0].right;
                    rects->rgrc[1].bottom = rects->rgrc[0].bottom;

                    rects->lppos->x += winInfo.cxWindowBorders;
                    rects->lppos->y += winInfo.cxWindowBorders;
                    rects->lppos->cx -= 2 * winInfo.cxWindowBorders;
                    rects->lppos->cy -= 2 * winInfo.cxWindowBorders;

                    return WVR_VALIDRECTS;
                }
                else
                {
                    SetWindowPos(m_native->m_handle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                    return 0;
                }
            }
#elif WINDOWS_USE_NEWER_BORDER_LESS
            if (wParam == TRUE && !m_params.hasBorder) // && m_params.isRegularWindow)
            {
                // In maximized mode fill the whole work area of the monitor (excludes task bar)
                if (isWindowMaximized(m_native->m_handle))
                {
                    HMONITOR monitor = ::MonitorFromWindow(m_native->m_handle, MONITOR_DEFAULTTONULL);
                    if (monitor)
                    {
                        MONITORINFO monitorInfo;
                        monitorInfo.cbSize = sizeof(monitorInfo);
                        if (::GetMonitorInfoW(monitor, &monitorInfo))
                        {
                            LPNCCALCSIZE_PARAMS rects = (LPNCCALCSIZE_PARAMS)lParam;
                            rects->rgrc[0] = monitorInfo.rcWork;
                        }
                    }
                }
                // SetWindowPos(m_native->m_handle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                return 0;
            }
#endif
            break;
        }
        case WM_NCHITTEST:
        {
            // Override it for fullscreen mode
            if (m_window->isFullscreen())
                return static_cast<i32>(WindowHitCodes::kClient);

            const Pos mouse(static_cast<float>(WINDOWS_GET_X_LPARAM(lParam)), static_cast<float>(WINDOWS_GET_Y_LPARAM(lParam)));
            WindowHitCodes hit = WindowHitCodes::kClient;
            bool handled = false;
            m_window->m_events.onHitTest(mouse, hit, handled);
            if (!handled)
                break;

            return static_cast<i32>(hit);
        }
        case WM_NCLBUTTONDOWN:
        {
            bool result = false;
            m_window->m_events.onLeftButtonHit(static_cast<WindowHitCodes>(wParam), result);
            if (!result)
                break;

            return 0;
        }
        case WM_NCLBUTTONDBLCLK:
        {
            // Handle non-client area double click manually
            if (m_window->isMaximized())
                m_window->restore();
            else
                m_window->maximize();
            return 0;
        }
#if WINDOWS_USE_NEWER_BORDER_LESS
        case WM_NCACTIVATE:
        {
            // Skip for border-less windows
        if (!m_window->m_params.hasBorder && !isCompositionEnabled())
                return 1;
            break;
        }
#endif
#if 0
    case WM_NCPAINT:
        // Skip for border-less windows
        if (!m_params.hasBorder)
            return 1;
        break;
#endif
        case WM_ERASEBKGND:
            // Skip the window background erasing
            return 1;
        case WM_GETMINMAXINFO:
        {
            const auto minMax = reinterpret_cast<MINMAXINFO *>(lParam);
            minMax->ptMinTrackSize.x = (i32)m_window->m_params.minSize.x;
            minMax->ptMinTrackSize.y = (i32)m_window->m_params.minSize.y;
            if (m_window->m_params.maxSize.sumValues() > 0)
            {
                i32 borderWidth = 0, borderHeight = 0;
                if (m_window->m_params.hasBorder)
                {
                    const DWORD windowStyle = GetWindowLongW(m_handle, GWL_STYLE);
                    const DWORD windowExStyle = GetWindowLongW(m_handle, GWL_EXSTYLE);
                    RECT borderRect = {0, 0, 0, 0};
                    AdjustWindowRectEx(&borderRect, windowStyle, false, windowExStyle);
                    borderWidth = borderRect.right - borderRect.left;
                    borderHeight = borderRect.bottom - borderRect.top;
                }
                minMax->ptMaxTrackSize.x = (i32)m_window->m_params.maxSize.x + borderWidth;
                minMax->ptMaxTrackSize.y = (i32)m_window->m_params.maxSize.y + borderHeight;
            }

            // Include Windows task bar size into maximized tool window
            WINDOWPLACEMENT e;
            if (!m_window->isFullscreen() && ((GetWindowPlacement(m_handle, &e) && (e.showCmd == SW_SHOWMAXIMIZED || e.showCmd == SW_SHOWMINIMIZED)) || m_isDuringMax))
            {
                // Adjust the maximized size and position to fit the work area of the correct monitor
                const HMONITOR monitor = MonitorFromWindow(m_handle, MONITOR_DEFAULTTONEAREST);
                if (monitor != nullptr)
                {
                    MONITORINFO monitorInfo;
                    monitorInfo.cbSize = sizeof(MONITORINFO);
                    if (::GetMonitorInfoW(monitor, &monitorInfo))
                    {
                        minMax->ptMaxPosition.x = TkMath::abs((i64)monitorInfo.rcWork.left - monitorInfo.rcMonitor.left);
                        minMax->ptMaxPosition.y = TkMath::abs((i64)monitorInfo.rcWork.top - monitorInfo.rcMonitor.top);
                        minMax->ptMaxSize.x = TkMath::abs((i64)monitorInfo.rcWork.right - monitorInfo.rcWork.left);
                        minMax->ptMaxSize.y = TkMath::abs((i64)monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
                    }
                }
            }

            return 0;
        }
        case WM_SYSCOMMAND:
            // Prevent moving/sizing in full screen mode
            if (m_window->isFullscreen())
            {
                switch ((wParam & 0xFFF0))
                {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_KEYMENU:
                    return 0;
                }
            }
            break;
        case WM_CREATE:
            return 0;
        case WM_SIZE:
        {
            if (SIZE_MINIMIZED == wParam)
            {
                // Get the minimized window position in workspace coordinates
                WINDOWPLACEMENT placement;
                placement.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(m_handle, &placement);

                // Calculate client offsets from window borders and title bar
                RECT winRect = {0, 0, static_cast<LONG>(m_window->m_clientSize.x), static_cast<LONG>(m_window->m_clientSize.y)};
                LONG style = GetWindowLong(m_handle, GWL_STYLE);
                LONG exStyle = GetWindowLong(m_handle, GWL_EXSTYLE);
                AdjustWindowRectEx(&winRect, style, FALSE, exStyle);

                // Calculate monitor offsets from taskbar position
                const HMONITOR monitor = MonitorFromWindow(m_handle, MONITOR_DEFAULTTONEAREST);
                MONITORINFO monitorInfo;
                monitorInfo.cbSize = sizeof(MONITORINFO);
                GetMonitorInfoW(monitor, &monitorInfo);

                // Convert the workspace coordinates to screen space and store it
                m_minScreenPos = Pos(
                    static_cast<float>(placement.rcNormalPosition.left + monitorInfo.rcWork.left - monitorInfo.rcMonitor.left - winRect.left),
                    static_cast<float>(placement.rcNormalPosition.top + monitorInfo.rcWork.top - monitorInfo.rcMonitor.top - winRect.top));

                m_window->m_state.minimized = true;
                m_window->m_state.maximized = false;
            }
            else
            {
                RECT rcCurrentClient;
                GetClientRect(m_handle, &rcCurrentClient);
                if (rcCurrentClient.top == 0 && rcCurrentClient.bottom == 0)
                {
                    // Rapidly clicking the task bar to minimize and restore a window can cause a WM_SIZE message with SIZE_RESTORED when
                    // the window has actually become minimized due to rapid change so just ignore this message.
                }
                else if (SIZE_MAXIMIZED == wParam)
                {
                    m_window->m_state.minimized = false;
                    m_window->m_state.maximized = true;
                    checkForWindowResize();
                    updateRegion();
                }
                else if (SIZE_RESTORED == wParam)
                {
                    if (m_window->m_state.maximized)
                    {
                        m_window->m_state.maximized = false;
                        checkForWindowResize();
                        updateRegion();
                    }
                    else if (m_window->m_state.minimized)
                    {
                        m_window->m_state.minimized = false;
                        checkForWindowResize();
                    }
                    else if (m_isResizing)
                    {
                        // If we're neither maximized nor minimized, the window size is changing by the user dragging the window edges.
                        // In this case, we don't resize yet -- we wait until the user stops dragging, and a WM_EXITSIZEMOVE message comes.
                        updateRegion();
                    }
                    else if (m_isSwitchingFullScreen)
                    {
                        // Ignored
                    }
                    else
                    {
                        // This WM_SIZE come from resizing the window via an API like SetWindowPos() so resize
                        checkForWindowResize();
                    }
                }
            }
            break;
        }
        case WM_SHOWWINDOW:
        {
            const bool visible = wParam == TRUE;
            if (visible && !m_window->m_state.visible)
            {
                m_window->m_state.visible = true;
                m_window->onShown();
            }
            else if (!visible && m_window->m_state.visible)
            {
                m_window->endClippingCursor();
                m_window->endTrackingMouse();
                m_window->m_state.visible = false;
                m_window->m_state.showAfterFirstPaint = m_window->m_params.showAfterFirstPaint;
                m_window->onHidden();
            }
            break;
        }
        case WM_DPICHANGED:
        {
            // Maybe https://stackoverflow.com/a/45110656
            m_window->m_dpi = HIWORD(wParam);
            m_window->m_dpiScale = (float)m_window->m_dpi / (float)DefaultDPI;
            RECT *windowRect = (RECT *)lParam;
            SetWindowPos(m_handle, nullptr, windowRect->left, windowRect->top, windowRect->right - windowRect->left, windowRect->bottom - windowRect->top, SWP_NOZORDER | SWP_NOACTIVATE);
            // TODO: Recalculate fonts
            return 0;
        }
        case WM_ENTERSIZEMOVE:
        {
            m_isResizing = true;
            break;
        }
        case WM_EXITSIZEMOVE:
            m_isResizing = false;
            checkForWindowResize();
            updateRegion();
            break;
        case WM_SETFOCUS:
            m_window->onGotFocusTrigger();
            updateCursor();
            if (m_window->m_track.isClippingCursor && !m_clipCursorSet)
            {
                m_clipCursorSet = true;
                ClipCursor((RECT *)m_clipCursorRect);
            }
            break;
        case WM_KILLFOCUS:
            if (m_clipCursorSet)
            {
                m_clipCursorSet = false;
                ClipCursor(nullptr);
            }
            m_window->onLostFocusTrigger();
            updateCursor();
            break;
        case WM_ACTIVATEAPP:
            if (wParam == TRUE)
            {
                if (!m_window->m_state.focused)
                {
                    m_window->onGotFocusTrigger();
                }
            }
            else if (m_window->m_state.focused)
            {
                m_window->onLostFocusTrigger();
                if (m_window->isFullscreen() && !m_isSwitchingFullScreen)
                {
                    m_window->setFullscreen(false);
                }
            }
            updateCursor();
            break;
        case WM_MENUCHAR:
            // A menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key so just ignore and don't beep
            return MAKELRESULT(0, MNC_CLOSE);
        case WM_SYSKEYDOWN:
            if (wParam == VK_F4)
            {
                LogInfo("Alt+F4 pressed");
                m_window->close(ClosingReason::kUser);
                return 0;
            }
            if (wParam == VK_RETURN)
            {
                LogInfo("Alt+Enter pressed");
                m_window->setFullscreen(!m_window->isFullscreen());
                return 0;
            }
            break;
        case WM_POWERBROADCAST:
            switch (wParam)
            {
            case PBT_APMQUERYSUSPEND:
                // TODO: add OnSystemSuspend event
                return true;
            case PBT_APMRESUMESUSPEND:
                // TODO: add OnSystemResume event
                return true;
            }
            break;
        case WM_CLOSE:
            m_window->close(ClosingReason::kUser);
            return 0;
        case WM_DESTROY:
        {
            if (!m_window->m_state.isClosing)
                m_window->m_state.isClosing = true;
            HWND destroyedHandle = m_handle;
            m_handle = nullptr;
            m_window->m_state.visible = false;
            WindowSystem::unregisterWindowHandle(destroyedHandle);
            m_window->onClosedTrigger();
            return 0;
        }
        }

        if (!m_window->m_params.allowInput)
            return DefWindowProc(m_handle, msg, wParam, lParam);

        if (Win32Input::wndProc(m_window, msg, wParam, lParam))
            return true;

        return DefWindowProc(m_handle, msg, wParam, lParam);
    }

    DragDropEffect TkWindow::doDragDrop(const TkStringView &data)
    {
        (void)data;
        // Active OLE drag is intentionally unsupported for now. WindowParams::allowDragAndDrop
        // only controls receiving drops; this API is fixed to kNone until a real DoDragDrop path exists.
        return DragDropEffect::kNone;
    }

    void TkWindow::onShowTrigger()
    {
        PROFILE_NAMED("Window.OnShow");
        onShown();
    }

    void TkWindow::onResizeTrigger(i32 width, i32 height)
    {
        PROFILE_NAMED("Window.OnResize");
        onResized({static_cast<float>(width), static_cast<float>(height)});
    }

    void TkWindow::onClosedTrigger()
    {
        if (m_state.closed)
            return;

        m_state.isClosing = true;
        m_state.closed = true;
        m_state.visible = false;
        endClippingCursor();
        endTrackingMouse();

        onClosed();
    }

    void TkWindow::onGotFocusTrigger()
    {
        if (m_state.focused)
            return;
        m_state.focused = true;
        if (m_state.suppressFocusEvents)
            return;

        onGotFocus();
    }

    void TkWindow::onLostFocusTrigger()
    {
        if (!m_state.focused)
            return;
        m_state.focused = false;
        if (m_state.suppressFocusEvents)
            return;

        onLostFocus();
    }

}

#endif
