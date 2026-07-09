/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/UUID.hpp"
#include <toolkit/input/InputDevice.hpp>

namespace toolkit
{
    // General identifiers for ptential force feedback channels. These will be mapped according to the platform specific implementation.
    struct TK_API GamepadVibrationState
    {
        // The left large motor vibration.
        f32 leftLarge;
        // The left small motor vibration.
        f32 leftSmall;
        // The right large motor vibration.
        f32 rightLarge;
        // The right small motor vibration.
        f32 rightSmall;

        GamepadVibrationState()
            : leftLarge(0.0f), leftSmall(0.0f), rightLarge(0.0f), rightSmall(0.0f)
        {
        }
    };

    // Gamepad buttons and axis mapping description.
    // Allows converting input from the different gamepads into a universal format (see <see cref="Gamepad::State::ButtonTypes"/> and <see cref="Gamepad::State::AxisTypes"/>).
    struct TK_API GamepadLayout
    {
        // The buttons mapping. Index by gamepad button id from 0 to 31 (see <see cref="Gamepad::State::ButtonTypes"/>).
        GamepadButton buttons[(i32)GamepadButton::kMax];
        // The axis mapping. Index by gamepad axis id from 0 to 5 (see <see cref="Gamepad::State::AxisTypes"/>).
        GamepadAxis axis[(i32)GamepadAxis::kMax];
        // The axis ranges mapping (X is scale, Y is offset. Eg. mappedVal = X * value + Y). It allows to invert any axis or map axis range.
        Pos axisMap[(i32)GamepadAxis::kMax];
        // Initializes layout with default values.
        void init();
    };

    // Represents a single hardware gamepad device. Used by the Input to report raw gamepad input events.
    class TK_API Gamepad : public InputDevice
    {
    public:
        // The universal gamepad state description. All hardware gamepad device handlers should map input to match this structure.
        // Later on, each gamepad may use individual layout for a game.
        struct State
        {
            // The buttons state (pressed if true).
            bool buttons[(i32)GamepadButton::kMax];
            // The axis state (normalized value).
            f32 axis[(i32)GamepadAxis::kMax];

            // Clears the state.
            void clear()
            {
                TkMemory::memoryClear(this, sizeof(State));
            }
        };

    protected:
        TkUuid m_productId;
        State m_state;
        State m_mappedState;
        State m_mappedPrevState;

        explicit Gamepad(const TkUuid &productId, const TkString &name);

    public:
        // The gamepad layout.
        GamepadLayout layout;

    public:
        // Get the gamepad device type identifier.
        TK_FORCE_INLINE const TkUuid &getProductID() const
        {
            return m_productId;
        }

        // Get the current gamepad state.
        TK_FORCE_INLINE void getState(State &result) const
        {
            result = m_state;
        }

        // Get the gamepad axis value.
        TK_FORCE_INLINE f32 getAxis(const GamepadAxis axis) const
        {
            if (!isValidGamepadAxis(axis))
                return 0.0f;
            return m_mappedState.axis[static_cast<i32>(axis)];
        }

        // Get the gamepad button state (true if being pressed during the current frame).
        TK_FORCE_INLINE bool getButton(const GamepadButton button) const
        {
            if (!isValidGamepadButton(button))
                return false;
            return m_mappedState.buttons[static_cast<i32>(button)];
        }

        // Get the gamepad button down state (true if was pressed during the current frame).
        TK_FORCE_INLINE bool getButtonDown(const GamepadButton button) const
        {
            if (!isValidGamepadButton(button))
                return false;
            return m_mappedState.buttons[static_cast<i32>(button)] && !m_mappedPrevState.buttons[static_cast<i32>(button)];
        }

        // Check if any gamepad button is currently pressed.
        bool isAnyButtonDown() const;

        // Get the gamepad button up state (true if was released during the current frame).
        TK_FORCE_INLINE bool getButtonUp(const GamepadButton button) const
        {
            if (!isValidGamepadButton(button))
                return false;
            return !m_mappedState.buttons[static_cast<i32>(button)] && m_mappedPrevState.buttons[static_cast<i32>(button)];
        }

    public:
        // Set the state of the gamepad vibration. Ignored if controller does not support this.
        virtual void setVibration(const GamepadVibrationState &state)
        {
        }

        // Set the color of the gamepad light. Ignored if controller does not support this; platform implementations must clamp channels to [0, 1].
        virtual void setColor(float r, float g, float b, float a = 1)
        {
            (void)r;
            (void)g;
            (void)b;
            (void)a;
        }

        // Reset the color of the gamepad light to the default. Ignored if controller does not support this.
        virtual void resetColor()
        {
        }

    public:
        void resetState() override;
        bool update(EventQueue &queue) final override;
    };

}
