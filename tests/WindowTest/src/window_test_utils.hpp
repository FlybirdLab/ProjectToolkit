/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#include <toolkit/base/Math.hpp>
#include <toolkit/platforms/Screen.hpp>
#include <toolkit/windowing/Window.hpp>
#include <toolkit/windowing/WindowSystem.hpp>
#include <toolkit/windowing/Dialog.hpp>
#if TK_WINDOWS
#include <toolkit/platforms/win32/IncludeWindows.hpp>
#endif

inline void initWindowSystemForTest()
{
#if TK_WINDOWS
    WindowSystem::init(GetModuleHandleW(nullptr));
#else
    WindowSystem::init(nullptr);
#endif
}

inline void expectPosNear(const Pos &actual, const Pos &expected, double epsilon = 1e-6)
{
    EXPECT_NEAR(actual.x, expected.x, epsilon);
    EXPECT_NEAR(actual.y, expected.y, epsilon);
}

inline void expectRectNear(const Rect &actual, const Rect &expected, double epsilon = 1e-6)
{
    EXPECT_NEAR(actual.x, expected.x, epsilon);
    EXPECT_NEAR(actual.y, expected.y, epsilon);
    EXPECT_NEAR(actual.w, expected.w, epsilon);
    EXPECT_NEAR(actual.h, expected.h, epsilon);
}

class WindowSystemFixture : public ::testing::Test
{
protected:
    static inline bool s_initialized = false;

    static void SetUpTestSuite()
    {
        if (!s_initialized)
        {
            initWindowSystemForTest();
            s_initialized = true;
        }
    }

    static void TearDownTestSuite()
    {
        if (s_initialized)
        {
            WindowSystem::dispose();
            s_initialized = false;
        }
    }

    TkWindow *createTestWindow(const TkStringView &title = "WindowTest", const Pos &pos = Pos(120.0, 140.0), const Pos &size = Pos(320.0, 240.0), bool transparent = false)
    {
        WindowParams params;
        params.title = title;
        params.pos = pos;
        params.size = size;
        params.supportsTransparency = transparent;

        WindowSystem system;
        TkWindow *window = system.createWindow(params);
        EXPECT_NE(window, nullptr);
        EXPECT_NE(window->getWindowPtr(), nullptr);
        EXPECT_NE(window->getNativeWindow(), nullptr);
        return window;
    }

    void destroyTestWindow(TkWindow *window)
    {
        if (!window)
            return;

        WindowSystem system;
        system.destroyWindow(window);
    }
};
