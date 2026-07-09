/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

TEST_F(WindowSystemFixture, NonTransparentWindowOpacityBehaviorIsFixed)
{
    TkWindow *window = createTestWindow("OpaqueOpacity", Pos(280.0, 260.0), Pos(240.0, 160.0), false);
    ASSERT_NE(window, nullptr);

    EXPECT_FLOAT_EQ(window->getOpacity(), 1.0f);
    window->setOpacity(0.5f);
    EXPECT_NEAR(window->getOpacity(), 0.5f, 0.05f);

    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, InvalidIconPathIsSafeThroughDestroy)
{
    TkWindow *window = createTestWindow("InvalidIconPath", Pos(300.0, 280.0), Pos(240.0, 160.0));
    ASSERT_NE(window, nullptr);

    window->setIcon("not_exists.ico");

    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, CursorClippingUsesScreenSpaceClientBounds)
{
    TkWindow *window = createTestWindow("ClipScreenRect", Pos(320.0, 300.0), Pos(240.0, 160.0));
    ASSERT_NE(window, nullptr);
    window->onGotFocusTrigger();

    const Rect clientBounds = window->getClientBounds();
    window->startClippingCursor(clientBounds);
    EXPECT_TRUE(window->isCursorClipping());

#if TK_WINDOWS
    RECT clipped{};
    ASSERT_TRUE(GetClipCursor(&clipped));
    EXPECT_NEAR(static_cast<float>(clipped.left), clientBounds.x, 2.5);
    EXPECT_NEAR(static_cast<float>(clipped.top), clientBounds.y, 2.5);
    EXPECT_NEAR(static_cast<float>(clipped.right), clientBounds.x + clientBounds.w, 2.5);
    EXPECT_NEAR(static_cast<float>(clipped.bottom), clientBounds.y + clientBounds.h, 2.5);
#endif

    window->endClippingCursor();
    EXPECT_FALSE(window->isCursorClipping());

    destroyTestWindow(window);
}
