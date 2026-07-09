/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

#include <toolkit/input/InputSystem.hpp>
#include <toolkit/input/Keyboard.hpp>
#include <toolkit/input/Mouse.hpp>

namespace
{
    class TestKeyboard final : public Keyboard
    {
    public:
        TestKeyboard()
            : Keyboard()
        {
        }
    };

    class TestMouse final : public Mouse
    {
    public:
        TestMouse()
            : Mouse()
        {
        }

        void setMousePosition(const Pos &newPosition) override
        {
            onMouseMoved(newPosition);
        }
    };
}

TEST(InputKeyboardTest, ResetClearsUtf8InputCacheImmediately)
{
    TestKeyboard keyboard;
    EventQueue events;

    keyboard.onCharInput(L'你');
    keyboard.update(events);
    EXPECT_NE(keyboard.getInputText(), TkStringView::Empty);

    keyboard.resetState();
    EXPECT_EQ(keyboard.getInputText(), TkStringView::Empty);

    keyboard.update(events);
    EXPECT_EQ(keyboard.getInputText(), TkStringView::Empty);
}

TEST_F(WindowSystemFixture, ClosedTargetDoesNotReceiveQueuedInputEvents)
{
    TkWindow *closedWindow = createTestWindow("ClosedInputTarget", Pos(100.0, 100.0), Pos(220.0, 140.0));
    TkWindow *focusedWindow = createTestWindow("FocusedInputWindow", Pos(180.0, 180.0), Pos(220.0, 140.0));
    ASSERT_NE(closedWindow, nullptr);
    ASSERT_NE(focusedWindow, nullptr);

    int keyCount = 0;
    int mouseCount = 0;
    closedWindow->getEventGroup().onKeyDown.bind([&](KeyboardKeys) { ++keyCount; });
    closedWindow->getEventGroup().onMouseDown.bind([&](const Pos &, MouseButton) { ++mouseCount; });

    closedWindow->close(ClosingReason::kUser);
    ASSERT_TRUE(closedWindow->isClosed());
    focusedWindow->onGotFocusTrigger();

    TestKeyboard keyboard;
    TestMouse mouse;
    InputSystem::Keyboard = &keyboard;
    InputSystem::Mouse = &mouse;
    InputSystem::Gamepads.clear();
    InputSystem::CustomDevices.clear();
    InputSystem::ActionMappings.clear();
    InputSystem::AxisMappings.clear();

    keyboard.onKeyDown(KeyboardKeys::kA, closedWindow);
    mouse.onMouseDown(Pos(12.0, 34.0), MouseButton::kLeft, closedWindow);
    InputSystem::update(1.0f);

    EXPECT_EQ(keyCount, 0);
    EXPECT_EQ(mouseCount, 0);

    InputSystem::Keyboard = nullptr;
    InputSystem::Mouse = nullptr;
    destroyTestWindow(closedWindow);
    destroyTestWindow(focusedWindow);
}
