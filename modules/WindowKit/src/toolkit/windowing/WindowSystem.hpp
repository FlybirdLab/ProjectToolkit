/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/threading/Sync.hpp"
#include <toolkit/windowing/common.hpp>

namespace toolkit
{

    class TkWindow;
    struct WindowParams;

    class WindowSystem
    {
    public:
        // Initialize the platform window backend with the process instance.
        static void init(void *instance);
        static void update();
        static void dispose();

        static bool getHasFocus();

    public:
        static TkWindow *createWindow(const WindowParams &p);
        static bool createWindow(TkWindow *window);
        static void destroyWindow(TkWindow *);

    public:
        // Protects access to the live window collection.
        static TkMutex WindowSysMtx;
        // Live windows owned by the window system.
        static TkArray<TkWindow *> Windows;

    public:
        // Gets the window by native handle.
        static TkWindow *getByWindowPtr(void *handle);
        static TkArray<TkWindow *> getWindowsSnapshot();
        static bool isLiveWindow(TkWindow *win);
        static void unregisterWindowHandle(void *handle);
        // Used by platform window implementations.
        static void registerWnd(TkWindow *win);
        static void unregisterWnd(TkWindow *win);

        static TkWindow* getMainWindow();
    };
}
