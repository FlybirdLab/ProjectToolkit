/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Mouse.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    void Mouse::onMouseMoved(const Pos &newPosition)
    {
        m_prevState.mousePos = newPosition;
        m_state.mousePos = newPosition;
    }

    void Mouse::onMouseDown(const Pos &position, const MouseButton button, TkWindow *target)
    {
        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kMouseDown;
        e.Target = target;
        e.mouseData.button = button;
        e.mouseData.pos = position;
    }

    bool Mouse::isAnyButtonDown() const
    {
        // TODO: optimize with SIMD
        bool result = false;
        for (const bool e : m_state.mouseButtons)
            result |= e;
        return result;
    }

    void Mouse::onMouseUp(const Pos &position, const MouseButton button, TkWindow *target)
    {
        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kMouseUp;
        e.Target = target;
        e.mouseData.button = button;
        e.mouseData.pos = position;
    }

    void Mouse::onMouseDoubleClick(const Pos &position, const MouseButton button, TkWindow *target)
    {
        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kMouseDoubleClick;
        e.Target = target;
        e.mouseData.button = button;
        e.mouseData.pos = position;
    }

    void Mouse::onMouseMove(const Pos &position, TkWindow *target)
    {
        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kMouseMove;
        e.Target = target;
        e.mouseData.pos = position;
    }

    void Mouse::onMouseLeave(TkWindow *target)
    {
        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kMouseLeave;
        e.Target = target;
    }

    void Mouse::onMouseWheel(const Pos &position, float delta, TkWindow *target)
    {
        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kMouseWheel;
        e.Target = target;
        e.mouseWheelData.wheelDelta = delta;
        e.mouseWheelData.pos = position;
    }

    void Mouse::resetState()
    {
        InputDevice::resetState();

        m_prevState.clear();
        m_state.clear();
    }

    bool Mouse::update(EventQueue &queue)
    {
        // Move the current state to the previous
        TkMemory::memoryCopy(&m_prevState, &m_state, sizeof(State));

        // Gather new events
        if (updateState())
            return true;

        // Handle events
        m_state.mouseWheelDelta = 0;
        for (i32 i = 0; i < m_queue.count(); i++)
        {
            const InputEvent &e = m_queue[i];
            switch (e.Type)
            {
            case InputEventType::kMouseDown:
            {
                m_state.mouseButtons[static_cast<i32>(e.mouseData.button)] = true;
                break;
            }
            case InputEventType::kMouseUp:
            {
                m_state.mouseButtons[static_cast<i32>(e.mouseData.button)] = false;
                break;
            }
            case InputEventType::kMouseDoubleClick:
            {
                m_state.mouseButtons[static_cast<i32>(e.mouseData.button)] = true;
                break;
            }
            case InputEventType::kMouseWheel:
            {
                m_state.mouseWheelDelta += e.mouseWheelData.wheelDelta;
                break;
            }
            case InputEventType::kMouseMove:
            {
                m_state.mousePos = e.mouseData.pos;
                break;
            }
            case InputEventType::kMouseLeave:
            {
                break;
            }
            case InputEventType::kChar:
            case InputEventType::kKeyDown:
            case InputEventType::kKeyUp:
            case InputEventType::kTouchDown:
            case InputEventType::kTouchMove:
            case InputEventType::kTouchUp:
                LogWarn("Unsupport type.");
                break;
            }
        }

        // Send events further
        queue.add(m_queue);
        m_queue.clear();
        return false;
    }

}
