/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

#if TK_WINDOWS
TEST_F(WindowSystemFixture, ExternalShowWindowMessagesSynchronizeVisibilityState)
{
    TkWindow *window = createTestWindow("ExternalShowWindow", Pos(260.0, 240.0), Pos(260.0, 180.0));
    ASSERT_NE(window, nullptr);

    int shownCount = 0;
    int hiddenCount = 0;
    window->onShown.bind([&]() { ++shownCount; });
    window->onHidden.bind([&]() { ++hiddenCount; });

    HWND hwnd = static_cast<HWND>(window->getWindowPtr());
    ASSERT_NE(hwnd, nullptr);

    ShowWindow(hwnd, SW_SHOW);
    WindowSystem::update();
    EXPECT_TRUE(window->isVisible());
    EXPECT_EQ(shownCount, 1);

    ShowWindow(hwnd, SW_SHOW);
    WindowSystem::update();
    EXPECT_TRUE(window->isVisible());
    EXPECT_EQ(shownCount, 1);

    ShowWindow(hwnd, SW_HIDE);
    WindowSystem::update();
    EXPECT_FALSE(window->isVisible());
    EXPECT_EQ(hiddenCount, 1);

    ShowWindow(hwnd, SW_HIDE);
    WindowSystem::update();
    EXPECT_FALSE(window->isVisible());
    EXPECT_EQ(hiddenCount, 1);

    destroyTestWindow(window);
}
#endif
