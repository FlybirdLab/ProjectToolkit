/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LinuxWindowSys.hpp"

#if TK_LINUX

#include "LinuxInput.hpp"
#include "LinuxWindow.hpp"
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/windowing/Dialog.hpp>
#include <toolkit/windowing/Window.hpp>
#include <toolkit/windowing/WindowSystem.hpp>

namespace toolkit
{
    X11::Display *LinuxWindowSys::s_Display = nullptr;
    X11::Atom LinuxWindowSys::s_DeleteWindow = 0;
    X11::Atom LinuxWindowSys::s_NetWmState = 0;
    X11::Atom LinuxWindowSys::s_NetWmStateAbove = 0;
    X11::Atom LinuxWindowSys::s_NetWmStateFullscreen = 0;
    X11::Atom LinuxWindowSys::s_NetWmStateMaximizedHorz = 0;
    X11::Atom LinuxWindowSys::s_NetWmStateMaximizedVert = 0;
    X11::Atom LinuxWindowSys::s_NetWmStateSkipTaskbar = 0;
    X11::Atom LinuxWindowSys::s_NetWmStateSkipPager = 0;
    X11::Atom LinuxWindowSys::s_NetWmStateDemandsAttention = 0;
    X11::Atom LinuxWindowSys::s_NetActiveWindow = 0;
    X11::Atom LinuxWindowSys::s_WmChangeState = 0;
    X11::Atom LinuxWindowSys::s_NetWmPid = 0;
    X11::Atom LinuxWindowSys::s_NetWmName = 0;
    X11::Atom LinuxWindowSys::s_Utf8String = 0;
    X11::Atom LinuxWindowSys::s_WmWindowOpacity = 0;
    X11::Atom LinuxWindowSys::s_MotifWmHints = 0;
    X11::Atom LinuxWindowSys::s_NetWmWindowType = 0;
    X11::Atom LinuxWindowSys::s_NetWmWindowTypeDock = 0;
    X11::Atom LinuxWindowSys::s_XdndAware = 0;

    constexpr i32 DefaultDPI = 96;
    bool s_initialized = false;
    TkHashMap<void *, TkWindow *> s_handleMap;

    IAllocator &getWindowAllocator(const WindowParams &params)
    {
        return params.alloc != nullptr ? *params.alloc : TkMemory::getDefault();
    }

    bool containsWindowUnlocked(TkWindow *win)
    {
        for (TkWindow *window : WindowSystem::Windows)
        {
            if (window == win)
                return true;
        }
        return false;
    }

    void syncHandleMapUnlocked(TkWindow *win)
    {
        if (win == nullptr)
            return;

        void *handle = win->getWindowPtr();
        if (handle != nullptr)
            s_handleMap[handle] = win;
    }

    void removeHandleMapEntriesForWindowUnlocked(TkWindow *win)
    {
        if (win == nullptr)
            return;

        TkArray<void *> keysToRemove;
        for (auto iterator = s_handleMap.Begin(); iterator.isNotEnd(); ++iterator)
        {
            if (iterator->value == win)
                keysToRemove.add(iterator->key);
        }
        for (void *key : keysToRemove)
            s_handleMap.remove(key);
    }

    X11::Atom internAtom(const char *name)
    {
        if (LinuxWindowSys::s_Display == nullptr)
            return 0;
        return X11::XInternAtom(LinuxWindowSys::s_Display, name, 0);
    }

    WindowParams resolveWindowParams(const WindowParams &p)
    {
        WindowParams resolvedParams = p;
        if (resolvedParams.startPos == WindowStartPos::kCenterParent || resolvedParams.startPos == WindowStartPos::kCenterScreen)
        {
            Rect parentBounds = Rect(Pos{0, 0}, System::getDesktopSize());
            if (resolvedParams.parent != nullptr && resolvedParams.startPos == WindowStartPos::kCenterParent)
                parentBounds = resolvedParams.parent->getClientBounds();

            resolvedParams.pos = Pos(parentBounds.location) + (Pos(parentBounds.size) - resolvedParams.size) * 0.5f;
        }
        return resolvedParams;
    }

    X11::Display *LinuxWindowSys::display()
    {
        return s_Display;
    }

    X11::Window LinuxWindowSys::rootWindow()
    {
        return s_Display ? X11::XDefaultRootWindow(s_Display) : 0;
    }

    void *LinuxWindowSys::toHandle(X11::Window window)
    {
        return reinterpret_cast<void *>(static_cast<uintptr>(window));
    }

    void WindowSystem::init(void *instance)
    {
        (void)instance;
        if (s_initialized)
            return;

        X11::XInitThreads();
        LinuxWindowSys::s_Display = X11::XOpenDisplay(nullptr);
        if (LinuxWindowSys::s_Display == nullptr)
        {
            LogError("Cannot open X11 display.");
            return;
        }

        LinuxWindowSys::s_DeleteWindow = internAtom("WM_DELETE_WINDOW");
        LinuxWindowSys::s_NetWmState = internAtom("_NET_WM_STATE");
        LinuxWindowSys::s_NetWmStateAbove = internAtom("_NET_WM_STATE_ABOVE");
        LinuxWindowSys::s_NetWmStateFullscreen = internAtom("_NET_WM_STATE_FULLSCREEN");
        LinuxWindowSys::s_NetWmStateMaximizedHorz = internAtom("_NET_WM_STATE_MAXIMIZED_HORZ");
        LinuxWindowSys::s_NetWmStateMaximizedVert = internAtom("_NET_WM_STATE_MAXIMIZED_VERT");
        LinuxWindowSys::s_NetWmStateSkipTaskbar = internAtom("_NET_WM_STATE_SKIP_TASKBAR");
        LinuxWindowSys::s_NetWmStateSkipPager = internAtom("_NET_WM_STATE_SKIP_PAGER");
        LinuxWindowSys::s_NetWmStateDemandsAttention = internAtom("_NET_WM_STATE_DEMANDS_ATTENTION");
        LinuxWindowSys::s_NetActiveWindow = internAtom("_NET_ACTIVE_WINDOW");
        LinuxWindowSys::s_WmChangeState = internAtom("WM_CHANGE_STATE");
        LinuxWindowSys::s_NetWmPid = internAtom("_NET_WM_PID");
        LinuxWindowSys::s_NetWmName = internAtom("_NET_WM_NAME");
        LinuxWindowSys::s_Utf8String = internAtom("UTF8_STRING");
        LinuxWindowSys::s_WmWindowOpacity = internAtom("_NET_WM_WINDOW_OPACITY");
        LinuxWindowSys::s_MotifWmHints = internAtom("_MOTIF_WM_HINTS");
        LinuxWindowSys::s_NetWmWindowType = internAtom("_NET_WM_WINDOW_TYPE");
        LinuxWindowSys::s_NetWmWindowTypeDock = internAtom("_NET_WM_WINDOW_TYPE_DOCK");
        LinuxWindowSys::s_XdndAware = internAtom("XdndAware");

        LinuxInput::init();
        s_initialized = true;
    }

    void WindowSystem::update()
    {
        LinuxInput::update();

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;

        while (X11::XPending(display) > 0)
        {
            X11::XEvent event;
            X11::XNextEvent(display, &event);

            TkWindow *window = nullptr;
            if (event.xany.window != 0)
                window = WindowSystem::getByWindowPtr(LinuxWindowSys::toHandle(event.xany.window));
            if (window != nullptr && window->getNativeWindow() != nullptr)
                window->getNativeWindow()->processEvent(event);
        }
    }

    TkMutex WindowSystem::WindowSysMtx{};
    TkArray<TkWindow *> WindowSystem::Windows{};

    TkWindow *WindowSystem::getByWindowPtr(void *handle)
    {
        if (handle == nullptr)
            return nullptr;

        WindowSysMtx.lock();
        const auto iterator = s_handleMap.find(handle);
        TkWindow *result = iterator.isEnd() ? nullptr : iterator->value;
        WindowSysMtx.unlock();
        return result;
    }

    TkArray<TkWindow *> WindowSystem::getWindowsSnapshot()
    {
        TkArray<TkWindow *> result;
        WindowSysMtx.lock();
        result = Windows;
        WindowSysMtx.unlock();
        return result;
    }

    bool WindowSystem::isLiveWindow(TkWindow *win)
    {
        if (win == nullptr)
            return false;

        WindowSysMtx.lock();
        const bool result = containsWindowUnlocked(win);
        WindowSysMtx.unlock();
        return result;
    }

    void WindowSystem::unregisterWindowHandle(void *handle)
    {
        if (handle == nullptr)
            return;

        WindowSysMtx.lock();
        s_handleMap.remove(handle);
        WindowSysMtx.unlock();
    }

    void WindowSystem::registerWnd(TkWindow *win)
    {
        if (win == nullptr)
            return;

        WindowSysMtx.lock();
        if (!containsWindowUnlocked(win))
            Windows.add(win);
        syncHandleMapUnlocked(win);
        WindowSysMtx.unlock();
    }

    void WindowSystem::unregisterWnd(TkWindow *win)
    {
        if (win == nullptr)
            return;

        WindowSysMtx.lock();
        Windows.remove(win);
        removeHandleMapEntriesForWindowUnlocked(win);
        WindowSysMtx.unlock();
    }

    TkWindow *WindowSystem::getMainWindow()
    {
        WindowSysMtx.lock();
        TkWindow *result = nullptr;
        for (TkWindow *window : Windows)
        {
            if (window != nullptr && !window->isClosed())
            {
                result = window;
                break;
            }
        }
        WindowSysMtx.unlock();
        return result;
    }

    void WindowSystem::dispose()
    {
        if (!s_initialized && Windows.isEmpty())
            return;

        TkArray<TkWindow *> windows = getWindowsSnapshot();
        for (TkWindow *window : windows)
        {
            if (window != nullptr)
                destroyWindow(window);
        }

        WindowSysMtx.lock();
        Windows.clear();
        s_handleMap.clear();
        WindowSysMtx.unlock();

        if (LinuxWindowSys::s_Display != nullptr)
        {
            X11::XCloseDisplay(LinuxWindowSys::s_Display);
            LinuxWindowSys::s_Display = nullptr;
        }
        s_initialized = false;
    }

    bool WindowSystem::getHasFocus()
    {
        for (TkWindow *window : getWindowsSnapshot())
        {
            if (window != nullptr && window->isFocused())
                return true;
        }
        return false;
    }

    TkWindow *WindowSystem::createWindow(const WindowParams &p)
    {
        WindowParams resolvedParams = resolveWindowParams(p);
        IAllocator &alloc = getWindowAllocator(resolvedParams);
        auto *ptr = TkMemory::newObject<TkWindow>(alloc, resolvedParams);
        if (ptr == nullptr)
            return nullptr;

        if (!WindowSystem::createWindow(ptr))
        {
            TkMemory::deletePtr(alloc, ptr);
            return nullptr;
        }
        return ptr;
    }

    bool WindowSystem::createWindow(TkWindow *window)
    {
        if (window == nullptr)
            return false;

        if (!s_initialized)
            WindowSystem::init(nullptr);
        if (!s_initialized)
            return false;

        WindowParams resolvedParams = resolveWindowParams(window->m_params);
        window->m_params = resolvedParams;
        window->m_clientSize = resolvedParams.size;

        IAllocator &alloc = getWindowAllocator(resolvedParams);
        auto *nativeWnd = TkMemory::newObject<NativeWindow>(alloc, resolvedParams, window);
        if (nativeWnd == nullptr)
            return false;

        window->m_native = nativeWnd;
        if (!nativeWnd->hasHandle())
        {
            window->m_native = nullptr;
            TkMemory::deletePtr(alloc, nativeWnd);
            return false;
        }

        WindowSystem::registerWnd(window);
        return true;
    }

    void WindowSystem::destroyWindow(TkWindow *window)
    {
        if (window == nullptr)
            return;

        IAllocator &alloc = getWindowAllocator(window->getParams());
        window->onBeforeDestroy();
        if (window->getNativeWindow() != nullptr)
        {
            TkMemory::deletePtr(alloc, window->getNativeWindow());
            window->m_native = nullptr;
        }
        WindowSystem::unregisterWnd(window);
        TkMemory::deletePtr(alloc, window);
    }

    TkWindow::TkWindow(const WindowParams &p)
        : m_events{},
          m_native{nullptr},
          m_state{},
          m_track{},
          m_params{p},
          m_dpi{DefaultDPI},
          m_dpiScale{1.0f},
          m_cursor{CursorType::kDefault},
          m_clientSize{p.size}
    {
        m_state.visible = false;
        m_state.minimized = false;
        m_state.maximized = false;
        m_state.fullscreen = p.fullscreen;
        m_state.isClosing = false;
        m_state.closed = false;
        m_state.showAfterFirstPaint = p.showAfterFirstPaint;
        m_state.focused = false;
        m_state.suppressFocusEvents = false;
        m_track.mouseOffset = {0, 0};
    }

    TkWindow::~TkWindow()
    {
    }

    void TkWindow::onBeforeDestroy()
    {
    }

    DialogResult Dialog::fromPlatformDialogResult(const int result)
    {
        return result == 0 ? DialogResult::kCancel : DialogResult::kOK;
    }

    DialogResult Dialog::msgBox(TkWindow *parent, const TkStringView &text, const TkStringView &caption, MsgBoxButtons buttons, MsgBoxIcon icon)
    {
        (void)parent;
        (void)text;
        (void)caption;
        (void)buttons;
        (void)icon;
        return DialogResult::kOK;
    }

    bool Dialog::showOpenFileDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &filter, bool multiSelect, const TkStringView &title, TkArray<TkString> &filenames)
    {
        (void)parentWindow;
        (void)initialDirectory;
        (void)filter;
        (void)multiSelect;
        (void)title;
        (void)filenames;
        return false;
    }

    bool Dialog::showSaveFileDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &filter, bool multiSelect, const TkStringView &title, TkArray<TkString> &filenames)
    {
        (void)parentWindow;
        (void)initialDirectory;
        (void)filter;
        (void)multiSelect;
        (void)title;
        (void)filenames;
        return false;
    }

    bool Dialog::showBrowseFolderDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &title, TkString &path)
    {
        (void)parentWindow;
        (void)initialDirectory;
        (void)title;
        (void)path;
        return false;
    }

    bool Dialog::showFileExplorer(const TkStringView &path)
    {
        if (!path.hasChars())
            return false;
        return System::openUrl(path);
    }
}

#endif
