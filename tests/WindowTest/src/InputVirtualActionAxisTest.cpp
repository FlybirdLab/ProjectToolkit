/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

#include <toolkit/input/Gamepad.hpp>
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

    class TestGamepad final : public Gamepad
    {
    public:
        TestGamepad()
            : Gamepad(TkUuid::create(), "VirtualAxisGamepad")
        {
        }

        void setAxis(GamepadAxis axis, float value)
        {
            m_state.axis[static_cast<i32>(axis)] = value;
        }

        bool updateState() override
        {
            return false;
        }
    };

    ActionConfig makeAction(const TkStringView &name, InputActionMode mode, KeyboardKeys key)
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

    AxisConfig makeGamepadAxis(const TkStringView &name, InputGamepadIndex gamepad)
    {
        AxisConfig config{};
        config.name = name;
        config.axis = InputAxisType::kGamepadLeftStickX;
        config.gamepad = gamepad;
        config.positiveButton = KeyboardKeys::kNone;
        config.negativeButton = KeyboardKeys::kNone;
        config.gamepadPositiveButton = GamepadButton::kNone;
        config.gamepadNegativeButton = GamepadButton::kNone;
        config.deadZone = 0.0f;
        config.sensitivity = 1.0f;
        config.gravity = 1.0f;
        config.scale = 1.0f;
        config.snap = false;
        return config;
    }

    void resetVirtualInputBindings()
    {
        InputSystem::ActionTriggered.unbindAll();
        InputSystem::AxisValueChanged.unbindAll();
        InputSystem::Keyboard = nullptr;
        InputSystem::Mouse = nullptr;
        InputSystem::Gamepads.clear();
        InputSystem::CustomDevices.clear();
        InputSystem::ActionMappings.clear();
        InputSystem::AxisMappings.clear();
    }
}

TEST_F(WindowSystemFixture, ActionReleasePriorityFallsBackToPressingNextFrame)
{
    TkWindow *window = createTestWindow("ActionReleasePriority");
    window->onGotFocusTrigger();

    TestKeyboard keyboard;
    InputSystem::Keyboard = &keyboard;
    InputSystem::Mouse = nullptr;
    InputSystem::Gamepads.clear();
    InputSystem::ActionMappings.clear();
    InputSystem::AxisMappings.clear();
    InputSystem::ActionMappings.add(makeAction("fireReleasePriority", InputActionMode::kPressing, KeyboardKeys::kA));
    InputSystem::ActionMappings.add(makeAction("fireReleasePriority", InputActionMode::kRelease, KeyboardKeys::kB));

    keyboard.onKeyDown(KeyboardKeys::kA, window);
    keyboard.onKeyDown(KeyboardKeys::kB, window);
    InputSystem::update(1.0f);
    EXPECT_EQ(InputSystem::getActionState("fireReleasePriority"), InputActionState::kPress);

    keyboard.onKeyUp(KeyboardKeys::kB, window);
    InputSystem::update(1.0f);
    EXPECT_EQ(InputSystem::getActionState("fireReleasePriority"), InputActionState::kRelease);

    InputSystem::update(1.0f);
    EXPECT_EQ(InputSystem::getActionState("fireReleasePriority"), InputActionState::kPressing);

    resetVirtualInputBindings();
    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, AxisValueChangedUsesEpsilonForTinyJitter)
{
    TkWindow *window = createTestWindow("AxisEpsilon");
    window->onGotFocusTrigger();

    TestGamepad gamepad;
    InputSystem::Gamepads.add(&gamepad);
    InputSystem::ActionMappings.clear();
    InputSystem::AxisMappings.clear();
    InputSystem::AxisMappings.add(makeGamepadAxis("epsilonAxis", InputGamepadIndex::kGamepad0));

    int changedCount = 0;
    InputSystem::AxisValueChanged.bind([&](TkStringView name) {
        if (name == TkStringView("epsilonAxis"))
            ++changedCount;
    });

    gamepad.setAxis(GamepadAxis::kLeftStickX, 0.5f);
    InputSystem::update(1.0f);
    EXPECT_EQ(changedCount, 1);

    gamepad.setAxis(GamepadAxis::kLeftStickX, 0.50005f);
    InputSystem::update(1.0f);
    EXPECT_EQ(changedCount, 1);

    resetVirtualInputBindings();
    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, DuplicateAxesUseLargestAbsoluteCandidate)
{
    TkWindow *window = createTestWindow("AxisLargestMagnitude");
    window->onGotFocusTrigger();

    TestGamepad gamepadA;
    TestGamepad gamepadB;
    gamepadA.setAxis(GamepadAxis::kLeftStickX, 0.3f);
    gamepadB.setAxis(GamepadAxis::kLeftStickX, -0.8f);

    InputSystem::Gamepads.add(&gamepadA);
    InputSystem::Gamepads.add(&gamepadB);
    InputSystem::ActionMappings.clear();
    InputSystem::AxisMappings.clear();
    InputSystem::AxisMappings.add(makeGamepadAxis("mergedAxis", InputGamepadIndex::kGamepad0));
    InputSystem::AxisMappings.add(makeGamepadAxis("mergedAxis", InputGamepadIndex::kGamepad1));

    InputSystem::update(1.0f);
    EXPECT_FLOAT_EQ(InputSystem::getAxisRaw("mergedAxis"), -0.8f);
    EXPECT_FLOAT_EQ(InputSystem::getAxis("mergedAxis"), -0.8f);

    resetVirtualInputBindings();
    destroyTestWindow(window);
}
