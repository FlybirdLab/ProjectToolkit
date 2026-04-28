/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

namespace
{
    Rect getWindowBounds(TkWindow *window)
    {
        return Rect(window->getPos(), window->getSize());
    }

    Rect getMonitorBoundsForWindow(TkWindow *window)
    {
        const HWND hwnd = static_cast<HWND>(window->getWindowPtr());
        const HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info{};
        info.cbSize = sizeof(info);
        EXPECT_TRUE(GetMonitorInfoW(monitor, &info));
        return Rect(
            static_cast<float>(info.rcMonitor.left),
            static_cast<float>(info.rcMonitor.top),
            static_cast<float>(info.rcMonitor.right - info.rcMonitor.left),
            static_cast<float>(info.rcMonitor.bottom - info.rcMonitor.top));
    }
}

TEST_F(WindowSystemFixture, FullscreenCoversMonitorAndRestoresBounds)
{
    TkWindow *window = createTestWindow("FullscreenRestore", Pos(220.0, 180.0), Pos(360.0, 240.0));
    ASSERT_NE(window, nullptr);

    const Rect beforeBounds = getWindowBounds(window);
    const Pos beforeClientSize = window->getClientSize();
    const Rect monitorBounds = getMonitorBoundsForWindow(window);
    EXPECT_FALSE(window->isFullscreen());

    window->setFullscreen(true);
    EXPECT_TRUE(window->isFullscreen());
    expectRectNear(getWindowBounds(window), monitorBounds, 3.0);

    window->setFullscreen(false);
    EXPECT_FALSE(window->isFullscreen());
    expectRectNear(getWindowBounds(window), beforeBounds, 6.0);
    expectPosNear(window->getClientSize(), beforeClientSize, 6.0);

    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, FullscreenFailurePathNeedsWin32ApiMock)
{
    GTEST_SKIP() << "TODO: wrap SetWindowLong/SetWindowPos/GetMonitorInfo behind a testable Win32 window API shim, then inject SetWindowPos failure and assert fullscreen state/rect rollback.";
}

TEST_F(WindowSystemFixture, MaximizeUpdatesClientSizeAndRestoreKeepsWindowUsable)
{
    TkWindow *window = createTestWindow("MaximizeResize", Pos(240.0, 220.0), Pos(320.0, 220.0));
    ASSERT_NE(window, nullptr);

    const Pos beforeClientSize = window->getClientSize();
    window->show();
    window->maximize();
    WindowSystem::update();

    EXPECT_TRUE(window->getClientSize().x > 0.0f);
    EXPECT_TRUE(window->getClientSize().y > 0.0f);
    EXPECT_TRUE(window->getClientSize().x >= beforeClientSize.x || window->getClientSize().y >= beforeClientSize.y);

    window->restore();
    WindowSystem::update();

    EXPECT_FALSE(window->isFullscreen());
    EXPECT_TRUE(window->getClientSize().x > 0.0f);
    EXPECT_TRUE(window->getClientSize().y > 0.0f);

    destroyTestWindow(window);
}
