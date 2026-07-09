/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Gamepad.hpp"

#include "InputDevice.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{

    void GamepadLayout::init()
    {
        for (i32 i = 0; i < (i32)GamepadButton::kMax; i++)
            buttons[i] = (GamepadButton)i;
        for (i32 i = 0; i < (i32)GamepadAxis::kMax; i++)
            axis[i] = (GamepadAxis)i;
        for (i32 i = 0; i < (i32)GamepadAxis::kMax; i++)
            axisMap[i] = Pos::UnitX;
    }

    Gamepad::Gamepad(const TkUuid &productId, const TkString &name)
        : InputDevice(name), m_productId(productId)
    {
        m_state.clear();
        m_mappedState.clear();
        m_mappedPrevState.clear();
        layout.init();
    }

    void Gamepad::resetState()
    {
        InputDevice::resetState();

        m_state.clear();
        m_mappedState.clear();
        m_mappedPrevState.clear();
    }

    bool Gamepad::isAnyButtonDown() const
    {
        // TODO: optimize with SIMD
        bool result = false;
        for (auto e : m_state.buttons)
            result |= e;
        return result;
    }

    bool Gamepad::update(EventQueue &queue)
    {
        // Copy state
        TkMemory::memoryCopy(&m_mappedPrevState, &m_mappedState, sizeof(State));
        m_mappedState.clear();

        // Gather current state
        if (updateState())
            return true;

        // Map state
        for (i32 i = 0; i < (i32)GamepadButton::kMax; i++)
        {
            auto e = layout.buttons[i];
            m_mappedState.buttons[static_cast<i32>(e)] = m_state.buttons[i];
        }
        for (i32 i = 0; i < (i32)GamepadAxis::kMax; i++)
        {
            auto e = layout.axis[i];
            float value = m_state.axis[i];
            value = layout.axisMap[i].x * value + layout.axisMap[i].y;
            m_mappedState.axis[static_cast<i32>(e)] = value;
        }

        return false;
    }

}