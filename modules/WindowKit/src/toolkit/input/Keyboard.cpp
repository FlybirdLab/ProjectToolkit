/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Keyboard.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    void Keyboard::State::clear()
    {
        TkMemory::memoryClear(this, sizeof(State));
    }

    Keyboard::Keyboard()
        : InputDevice("Keyboard")
    {
        m_state.clear();
        m_prevState.clear();
    }

    TkStringView Keyboard::getInputText() const
    {
        const auto utf8 = StringAsUTF8<>(m_state.text, m_state.textLen);
        m_inputTextUtf8 = utf8.get();
        return TkStringView(m_inputTextUtf8);
    }

    bool Keyboard::isAnyKeyDown() const
    {
        // TODO: optimize with SIMD
        bool result = false;
        for (auto e : m_state.keys)
            result |= e;
        return result;
    }

    void Keyboard::onCharInput(wchar c, TkWindow *target)
    {
        // Skip control characters
        if (c < 32)
            return;

        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kChar;
        e.Target = target;
        e.charData.c = c;
    }

    void Keyboard::onKeyUp(KeyboardKeys key, TkWindow *target)
    {
        if (key >= KeyboardKeys::kMax)
            return;
        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kKeyUp;
        e.Target = target;
        e.keyData.key = key;
    }

    void Keyboard::onKeyDown(KeyboardKeys key, TkWindow *target)
    {
        if (key >= KeyboardKeys::kMax)
            return;
        InputEvent &e = m_queue.addOne();
        e.Type = InputEventType::kKeyDown;
        e.Target = target;
        e.keyData.key = key;
    }

    void Keyboard::resetState()
    {
        InputDevice::resetState();

        m_prevState.clear();
        m_state.clear();
    }

    bool Keyboard::update(EventQueue &queue)
    {
        // Move the current state to the previous
        TkMemory::memoryCopy(&m_prevState, &m_state, sizeof(State));

        // Gather new events
        if (updateState())
            return true;

        // Handle events
        m_state.textLen = 0;
        for (i32 i = 0; i < m_queue.count(); i++)
        {
            const InputEvent &e = m_queue[i];
            switch (e.Type)
            {
            case InputEventType::kChar:
            {
                if (m_state.textLen < lengthOf(m_state.text) - 1)
                    m_state.text[m_state.textLen++] = e.charData.c;
                break;
            }
            case InputEventType::kKeyDown:
            {
                m_state.keys[static_cast<i32>(e.keyData.key)] = true;
                break;
            }
            case InputEventType::kKeyUp:
            {
                m_state.keys[static_cast<i32>(e.keyData.key)] = false;
                break;
            }
            case InputEventType::kMouseDown:
            case InputEventType::kMouseUp:
            case InputEventType::kMouseDoubleClick:
            case InputEventType::kMouseWheel:
            case InputEventType::kMouseMove:
            case InputEventType::kMouseLeave:
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
