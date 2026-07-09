/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include <toolkit/input/Keyboard.hpp>
#include <toolkit/input/Mouse.hpp>
#include <toolkit/input/Gamepad.hpp>

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

        void setMousePosition(const Pos& newPosition) override
        {
            onMouseMoved(newPosition);
        }
    };

    class TestGamepad final : public Gamepad
    {
    public:
        TestGamepad()
            : Gamepad(TkUuid::create(), "TestGamepad")
        {
        }
    };
}

TEST(InputSafetyTest, KeyboardRejectsInvalidKeysAndKeepsUnicodeText)
{
    TestKeyboard keyboard;
    EXPECT_FALSE(keyboard.getKey(KeyboardKeys::kNone));
    EXPECT_FALSE(keyboard.getKey(KeyboardKeys::kMax));
    EXPECT_FALSE(keyboard.getKey(static_cast<KeyboardKeys>(9999)));

    EventQueue events;
    keyboard.onCharInput(L'\n');
    keyboard.onCharInput(L'A');
    keyboard.onCharInput(L'你');
    keyboard.update(events);
    EXPECT_EQ(keyboard.getInputText(), TkStringView("A你"));

    keyboard.update(events);
    EXPECT_EQ(keyboard.getInputText(), TkStringView::Empty);
}

TEST(InputSafetyTest, MouseRejectsInvalidButtons)
{
    TestMouse mouse;
    EventQueue events;
    mouse.onMouseDown(Pos(1.0, 2.0), MouseButton::kNone);
    mouse.onMouseDown(Pos(1.0, 2.0), MouseButton::kMax);
    mouse.onMouseDown(Pos(1.0, 2.0), static_cast<MouseButton>(999));
    mouse.update(events);
    EXPECT_FALSE(mouse.getButton(MouseButton::kNone));
    EXPECT_FALSE(mouse.getButton(MouseButton::kMax));
    EXPECT_FALSE(mouse.getButton(static_cast<MouseButton>(999)));
    EXPECT_TRUE(events.isEmpty());
}

TEST(InputSafetyTest, GamepadRejectsInvalidAxisAndButtons)
{
    TestGamepad gamepad;
    EXPECT_FLOAT_EQ(gamepad.getAxis(GamepadAxis::kNone), 0.0f);
    EXPECT_FLOAT_EQ(gamepad.getAxis(GamepadAxis::kMax), 0.0f);
    EXPECT_FLOAT_EQ(gamepad.getAxis(static_cast<GamepadAxis>(999)), 0.0f);
    EXPECT_FALSE(gamepad.getButton(GamepadButton::kNone));
    EXPECT_FALSE(gamepad.getButton(GamepadButton::kMax));
    EXPECT_FALSE(gamepad.getButton(static_cast<GamepadButton>(999)));
}
