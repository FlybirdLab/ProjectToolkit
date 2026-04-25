/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

TEST(WindowSystemLifecycleTest, InitDisposeAreReentrant)
{
    WindowSystem::dispose();
    EXPECT_EQ(WindowSystem::getMainWindow(), nullptr);

    WindowSystem::init(GetModuleHandleW(nullptr));
    WindowSystem::init(GetModuleHandleW(nullptr));
    WindowSystem::dispose();
    WindowSystem::dispose();

    WindowSystem::init(GetModuleHandleW(nullptr));
}

TEST_F(WindowSystemFixture, DestroyNullAndClosedWindowAreSafe)
{
    WindowSystem::destroyWindow(nullptr);

    TkWindow* window = createTestWindow("DestroyIdempotent");
    ASSERT_NE(window, nullptr);
    EXPECT_TRUE(WindowSystem::isLiveWindow(window));
    void* handle = window->getWindowPtr();

    int closedCount = 0;
    window->onClosed.bind([&]() { ++closedCount; });
    window->close(ClosingReason::kUser);
    EXPECT_LE(closedCount, 1);
    EXPECT_EQ(WindowSystem::getByWindowPtr(handle), nullptr);
    EXPECT_TRUE(WindowSystem::isLiveWindow(window));

    WindowSystem::destroyWindow(window);
    EXPECT_EQ(WindowSystem::getByWindowPtr(handle), nullptr);
    EXPECT_FALSE(WindowSystem::isLiveWindow(window));
}

TEST_F(WindowSystemFixture, SnapshotAndHandleMapTrackMultipleWindows)
{
    TkWindow* first = createTestWindow("SnapshotA", Pos(100.0, 100.0));
    TkWindow* second = createTestWindow("SnapshotB", Pos(180.0, 180.0));
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_TRUE(first->isFirstWindow());
    EXPECT_FALSE(second->isFirstWindow());
    EXPECT_EQ(WindowSystem::getByWindowPtr(first->getWindowPtr()), first);
    EXPECT_EQ(WindowSystem::getByWindowPtr(second->getWindowPtr()), second);
    EXPECT_TRUE(WindowSystem::isLiveWindow(first));
    EXPECT_TRUE(WindowSystem::isLiveWindow(second));
    EXPECT_EQ(WindowSystem::getByWindowPtr(reinterpret_cast<void*>(0x12345678)), nullptr);

    const auto snapshot = WindowSystem::getWindowsSnapshot();
    EXPECT_TRUE(snapshot.contains(first));
    EXPECT_TRUE(snapshot.contains(second));

    void* firstHandle = first->getWindowPtr();
    destroyTestWindow(first);
    EXPECT_EQ(WindowSystem::getByWindowPtr(firstHandle), nullptr);
    EXPECT_FALSE(WindowSystem::isLiveWindow(first));
    EXPECT_EQ(WindowSystem::getMainWindow(), second);

    destroyTestWindow(second);
}
