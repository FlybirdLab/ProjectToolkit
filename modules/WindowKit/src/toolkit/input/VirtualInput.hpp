/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/input/InputDevice.hpp>

namespace toolkit
{
    // Maps keyboard, controller, or mouse inputs to a "friendly name" that will later be bound to continuous game behavior, such as movement. The inputs mapped in AxisMappings are continuously polled, even if they are just reporting that their input value.
    struct ActionConfig
    {
        // The action "friendly name" used to access it from code.
        TkString name;
        // The trigger mode. Allows to specify when input event should be fired.
        InputActionMode mode;
        // The keyboard key to map for this action. Use <see cref="KeyboardKeys.None"/> to ignore it.
        KeyboardKeys key;
        // The mouse button to map for this action. Use <see cref="MouseButton.None"/> to ignore it.
        MouseButton mouseButton;
        // The gamepad button to map for this action. Use <see cref="GamepadButton.None"/> to ignore it.
        GamepadButton gamepadButton;
        // Which gamepad should be used.
        InputGamepadIndex gamepad;
    };

    // Maps keyboard, controller, or mouse inputs to a "friendly name" that will later be bound to continuous game behavior, such as movement. The inputs mapped in AxisMappings are continuously polled, even if they are just reporting that their input value.
    struct AxisConfig
    {
        // The axis "friendly name" used to access it from code.
        TkString name;
        // The axis type (mouse, gamepad, etc.).
        InputAxisType axis;
        // Which gamepad should be used.
        InputGamepadIndex gamepad;
        // The button to be pressed for movement in positive direction. Use <see cref="KeyboardKeys.None"/> to ignore it.
        KeyboardKeys positiveButton;
        // The button to be pressed for movement in negative direction. Use <see cref="KeyboardKeys.None"/> to ignore it.
        KeyboardKeys negativeButton;
        // The button to be pressed for movement in positive direction. Use <see cref="GamepadButton.None"/> to ignore it.
        GamepadButton gamepadPositiveButton;
        // The button to be pressed for movement in negative direction. Use <see cref="GamepadButton.None"/> to ignore it.
        GamepadButton gamepadNegativeButton;
        // Any positive or negative values that are less than this number will register as zero. Useful for gamepads to specify the deadzone.
        float deadZone;
        // For keyboard input, a larger value will result in faster response time (in units/s). A lower value will be more smooth. For Mouse delta the value will scale the actual mouse delta.
        float sensitivity;
        // For keyboard input describes how fast will the input recenter. Speed (in units/s) that output value will rest to neutral value if not when device at rest.
        float gravity;
        // Additional scale parameter applied to the axis value. Allows to invert it or modify the range.
        float scale;
        // If enabled, the axis value will be immediately reset to zero after it receives opposite inputs. For keyboard input only.
        bool snap;
    };
}