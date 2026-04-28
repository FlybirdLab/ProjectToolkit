/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

TEST_F(WindowSystemFixture, ClosedFirstWindowIsSkippedAsMainUntilDestroyed)
{
    TkWindow *first = createTestWindow("LifecycleFirst", Pos(90.0, 90.0), Pos(220.0, 140.0));
    TkWindow *second = createTestWindow("LifecycleSecond", Pos(140.0, 140.0), Pos(220.0, 140.0));
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(WindowSystem::getMainWindow(), first);
    first->close(ClosingReason::kUser);

    EXPECT_TRUE(first->isClosed());
    EXPECT_TRUE(WindowSystem::isLiveWindow(first));
    EXPECT_EQ(WindowSystem::getMainWindow(), second);

    destroyTestWindow(first);
    destroyTestWindow(second);
}

TEST_F(WindowSystemFixture, DestroyClosedWindowDoesNotRepeatClosedEvent)
{
    TkWindow *window = createTestWindow("LifecycleClosedOnce", Pos(120.0, 120.0), Pos(220.0, 140.0));
    ASSERT_NE(window, nullptr);

    int closedCount = 0;
    window->onClosed.bind([&]() { ++closedCount; });

    window->close(ClosingReason::kUser);
    EXPECT_EQ(closedCount, 1);

    WindowSystem::destroyWindow(window);
    EXPECT_EQ(closedCount, 1);
}
