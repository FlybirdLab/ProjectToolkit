/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/input/InputDevice.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{
    // Represent a single hardware mouse device. Used by the Input to report raw mouse input events.
    class TK_API Mouse : public InputDevice
    {
    public:
        // The mouse state.
        struct State
        {
            // The mouse position.
            Pos mousePos;
            // The mouse wheel delta.
            float mouseWheelDelta;
            // The mouse buttons state.
            bool mouseButtons[(i32)MouseButton::kMax];

            // Clear the state.
            void clear()
            {
                TkMemory::memoryClear(this, sizeof(State));
            }
        };

    protected:
        State m_state;
        State m_prevState;

        explicit Mouse()
            : InputDevice("Mouse")
        {
            m_state.clear();
            m_prevState.clear();
        }

    public:
        // Get the position of the mouse in the screen-space coordinates.
        TK_FORCE_INLINE Pos getPos() const
        {
            return m_state.mousePos;
        }

        // Checks if any mouse button is currently pressed.
        bool isAnyButtonDown() const;

        // Get the delta position of the mouse in the screen-space coordinates.
        TK_FORCE_INLINE Pos getPosDelta() const
        {
            return m_state.mousePos - m_prevState.mousePos;
        }

        // Get the mouse wheel change during the last frame.
        TK_FORCE_INLINE float getScrollDelta() const
        {
            return m_state.mouseWheelDelta;
        }

        // Get the mouse button state (true if being pressed during the current frame).
        TK_FORCE_INLINE bool getButton(MouseButton button) const
        {
            return m_state.mouseButtons[static_cast<i32>(button)];
        }

        // Get the mouse button down state (true if was pressed during the current frame).
        TK_FORCE_INLINE bool getButtonDown(MouseButton button) const
        {
            return m_state.mouseButtons[static_cast<i32>(button)] && !m_prevState.mouseButtons[static_cast<i32>(button)];
        }

        // Get the mouse button up state (true if was released during the current frame).
        TK_FORCE_INLINE bool getButtonUp(MouseButton button) const
        {
            return !m_state.mouseButtons[static_cast<i32>(button)] && m_prevState.mouseButtons[static_cast<i32>(button)];
        }

    public:
        // Set the mouse position.
        virtual void setMousePosition(const Pos &newPosition) = 0;

        // Called when mouse cursor gets moved by the application. Invalidates the previous cached mouse position to prevent mouse jitter when locking the cursor programmatically.
        void onMouseMoved(const Pos &newPosition);
        // Called when mouse button goes down.
        void onMouseDown(const Pos &position, const MouseButton button, TkWindow *target = nullptr);
        // Called when mouse button goes up.
        void onMouseUp(const Pos &position, const MouseButton button, TkWindow *target = nullptr);
        // Called when mouse double clicks.
        void onMouseDoubleClick(const Pos &position, const MouseButton button, TkWindow *target = nullptr);
        // Called when mouse moves.
        void onMouseMove(const Pos &position, TkWindow *target = nullptr);
        // Called when mouse leaves the input source area.
        void onMouseLeave(TkWindow *target = nullptr);
        // Called when mouse wheel moves.
        void onMouseWheel(const Pos &position, float delta, TkWindow *target = nullptr);

    public:
        void resetState() override;
        bool update(EventQueue &queue) final override;
    };
}