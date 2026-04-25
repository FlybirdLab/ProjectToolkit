/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "window_test_utils.hpp"

TEST_F(WindowSystemFixture, ScreenDefaultsAndStateChangesAreConsistent)
{
    Screen::setIsFullscreen(false);
    EXPECT_FALSE(Screen::getIsFullscreen());

    Screen::setCursorVisible(true);
    EXPECT_TRUE(Screen::getCursorVisible());
    Screen::setCursorVisible(false);
    EXPECT_FALSE(Screen::getCursorVisible());
    Screen::setCursorVisible(true);
    EXPECT_TRUE(Screen::getCursorVisible());

    Screen::setCursorLock(CursorLockMode::kNone);
    EXPECT_EQ(Screen::getCursorLock(), CursorLockMode::kNone);
    Screen::setCursorLock(CursorLockMode::kLocked);
    EXPECT_EQ(Screen::getCursorLock(), CursorLockMode::kLocked);
    Screen::setCursorLock(CursorLockMode::kNone);
    EXPECT_EQ(Screen::getCursorLock(), CursorLockMode::kNone);

    Screen::setSize(Pos(1280.0, 720.0));
    Screen::updateScreen(0.016f);
    Screen::drawScreen();
}

TEST_F(WindowSystemFixture, ScreenUsesMainWindowClientConversions)
{
    TkWindow *window = createTestWindow("ScreenRuntime", Pos(180.0, 200.0), Pos(360.0, 260.0));
    ASSERT_NE(window, nullptr);

    window->show();
    expectPosNear(Screen::getSize(), window->getClientSize(), 2.5);

    const Pos viewportPos(40.0, 28.0);
    const Pos screenPos = Screen::gameViewportToScreen(viewportPos);
    expectPosNear(Screen::screenToGameViewport(screenPos), viewportPos, 2.5);

    Screen::setCursorLock(CursorLockMode::kClipped);
    EXPECT_EQ(Screen::getCursorLock(), CursorLockMode::kClipped);
    EXPECT_TRUE(window->isCursorClipping());

    Screen::setCursorLock(CursorLockMode::kLocked);
    EXPECT_EQ(Screen::getCursorLock(), CursorLockMode::kLocked);
    EXPECT_FALSE(window->isCursorClipping());

    window->startTrackingMouse(false);
    EXPECT_TRUE(window->isMouseTracking());
    Screen::setCursorLock(CursorLockMode::kNone);
    EXPECT_EQ(Screen::getCursorLock(), CursorLockMode::kNone);
    EXPECT_FALSE(window->isCursorClipping());
    EXPECT_FALSE(window->isMouseTracking());

    destroyTestWindow(window);
}
