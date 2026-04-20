/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/windowing/Window.hpp>
#include <toolkit/input/InputSystem.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit
{
    TkStringView TkWindow::getInputText() const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getInputText() : TkStringView::Empty;
    }

    bool TkWindow::getKey(KeyboardKeys key) const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getKey(key) : false;
    }

    bool TkWindow::getKeyDown(KeyboardKeys key) const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getKeyDown(key) : false;
    }

    bool TkWindow::getKeyUp(KeyboardKeys key) const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getKeyUp(key) : false;
    }

    Pos TkWindow::getMousePos() const
    {
        return m_params.allowInput && m_state.focused ? screenToClient(InputSystem::getMouseScreenPosition()) : Pos::Minimum;
    }

    void TkWindow::setMousePos(const Pos &position) const
    {
        if (m_params.allowInput && m_state.focused)
            InputSystem::setMouseScreenPosition(clientToScreen(position));
    }

    Pos TkWindow::getMousePosDelta() const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getMousePositionDelta() : Pos{0, 0};
    }

    f32 TkWindow::getMouseScrollDelta() const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getMouseScrollDelta() : 0.0f;
    }

    bool TkWindow::getMouseButton(MouseButton button) const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getMouseButton(button) : false;
    }

    bool TkWindow::getMouseButtonDown(MouseButton button) const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getMouseButtonDown(button) : false;
    }

    bool TkWindow::getMouseButtonUp(MouseButton button) const
    {
        return m_params.allowInput && m_state.focused ? InputSystem::getMouseButtonUp(button) : false;
    }
}
