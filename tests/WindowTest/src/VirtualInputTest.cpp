/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

#include <toolkit/input/InputSystem.hpp>
#include <toolkit/input/Keyboard.hpp>

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

    ActionConfig makeKeyAction(const TkStringView &name, InputActionMode mode, KeyboardKeys key)
    {
        ActionConfig config{};
        config.name = name;
        config.mode = mode;
        config.key = key;
        config.mouseButton = MouseButton::kNone;
        config.gamepadButton = GamepadButton::kNone;
        config.gamepad = InputGamepadIndex::kAll;
        return config;
    }

    AxisConfig makeKeyAxis(const TkStringView &name, KeyboardKeys positive, KeyboardKeys negative)
    {
        AxisConfig config{};
        config.name = name;
        config.axis = InputAxisType::kKeyboardOnly;
        config.gamepad = InputGamepadIndex::kAll;
        config.positiveButton = positive;
        config.negativeButton = negative;
        config.gamepadPositiveButton = GamepadButton::kNone;
        config.gamepadNegativeButton = GamepadButton::kNone;
        config.deadZone = 0.1f;
        config.sensitivity = 10.0f;
        config.gravity = 10.0f;
        config.scale = 1.0f;
        config.snap = true;
        return config;
    }
}

TEST_F(WindowSystemFixture, VirtualActionsMergeDuplicateMappingsWithStablePriority)
{
    TkWindow *window = createTestWindow("VirtualAction");
    window->onGotFocusTrigger();

    TestKeyboard keyboard;
    InputSystem::Keyboard = &keyboard;
    InputSystem::Mouse = nullptr;
    InputSystem::ActionMappings.clear();
    InputSystem::AxisMappings.clear();

    InputSystem::ActionMappings.add(makeKeyAction("fire", InputActionMode::kPressing, KeyboardKeys::kA));
    InputSystem::ActionMappings.add(makeKeyAction("fire", InputActionMode::kRelease, KeyboardKeys::kB));

    keyboard.onKeyDown(KeyboardKeys::kA, window);
    keyboard.onKeyDown(KeyboardKeys::kB, window);
    InputSystem::update(1.0f);
    EXPECT_TRUE(InputSystem::getAction("fire"));
    EXPECT_EQ(InputSystem::getActionState("fire"), InputActionState::kPress);

    keyboard.onKeyUp(KeyboardKeys::kB, window);
    InputSystem::update(1.0f);
    EXPECT_TRUE(InputSystem::getAction("fire"));
    EXPECT_EQ(InputSystem::getActionState("fire"), InputActionState::kRelease);

    InputSystem::Keyboard = nullptr;
    InputSystem::ActionMappings.clear();
    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, VirtualAxisAppliesDeadzoneSmoothingAndSnap)
{
    TkWindow *window = createTestWindow("VirtualAxis");
    window->onGotFocusTrigger();

    TestKeyboard keyboard;
    InputSystem::Keyboard = &keyboard;
    InputSystem::Mouse = nullptr;
    InputSystem::ActionMappings.clear();
    InputSystem::AxisMappings.clear();

    InputSystem::AxisMappings.add(makeKeyAxis("moveX", KeyboardKeys::kD, KeyboardKeys::kA));

    keyboard.onKeyDown(KeyboardKeys::kD, window);
    InputSystem::update(1.0f);
    EXPECT_FLOAT_EQ(InputSystem::getAxis("moveX"), 1.0f);
    EXPECT_FLOAT_EQ(InputSystem::getAxisRaw("moveX"), 1.0f);

    keyboard.onKeyUp(KeyboardKeys::kD, window);
    keyboard.onKeyDown(KeyboardKeys::kA, window);
    InputSystem::update(1.0f);
    EXPECT_FLOAT_EQ(InputSystem::getAxis("moveX"), -1.0f);
    EXPECT_FLOAT_EQ(InputSystem::getAxisRaw("moveX"), -1.0f);

    keyboard.onKeyUp(KeyboardKeys::kA, window);
    InputSystem::update(1.0f);
    EXPECT_FLOAT_EQ(InputSystem::getAxis("moveX"), 0.0f);
    EXPECT_FLOAT_EQ(InputSystem::getAxisRaw("moveX"), 0.0f);

    InputSystem::Keyboard = nullptr;
    InputSystem::AxisMappings.clear();
    destroyTestWindow(window);
}
