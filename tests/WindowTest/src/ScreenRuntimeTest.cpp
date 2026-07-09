/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

TEST_F(WindowSystemFixture, SetSizeChangesMainWindowClientSize)
{
    TkWindow *window = createTestWindow("ScreenSetSize", Pos(180.0, 160.0), Pos(320.0, 240.0));
    ASSERT_NE(window, nullptr);
    ASSERT_EQ(WindowSystem::getMainWindow(), window);

    Screen::setSize(Pos(640.0, 480.0));
    expectPosNear(window->getClientSize(), Pos(640.0, 480.0), 3.0);
    expectPosNear(Screen::getSize(), Pos(640.0, 480.0), 3.0);

    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, CursorVisibilityRestoresPreviousVisibleCursor)
{
    TkWindow *window = createTestWindow("ScreenCursorRestore", Pos(200.0, 180.0), Pos(240.0, 160.0));
    ASSERT_NE(window, nullptr);

    window->setCursor(CursorType::kHand);
    EXPECT_EQ(window->getCursor(), CursorType::kHand);

    Screen::setCursorVisible(false);
    EXPECT_FALSE(Screen::getCursorVisible());
    EXPECT_EQ(window->getCursor(), CursorType::kHidden);

    Screen::setCursorVisible(true);
    EXPECT_TRUE(Screen::getCursorVisible());
    EXPECT_EQ(window->getCursor(), CursorType::kHand);

    destroyTestWindow(window);
}

TEST(ScreenRuntimeTest, ScreenApisAreSafeWithoutMainWindow)
{
    WindowSystem::dispose();
    EXPECT_EQ(WindowSystem::getMainWindow(), nullptr);
    expectPosNear(Screen::getSize(), Pos(0.0, 0.0));

    Screen::setSize(Pos(640.0, 480.0));
    Screen::setIsFullscreen(false);
    Screen::setCursorVisible(false);
    Screen::setCursorVisible(true);
    Screen::setCursorLock(CursorLockMode::kClipped);
    Screen::setCursorLock(CursorLockMode::kLocked);
    Screen::setCursorLock(CursorLockMode::kNone);

    initWindowSystemForTest();
}
