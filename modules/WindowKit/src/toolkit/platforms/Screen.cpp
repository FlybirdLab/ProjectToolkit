/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Screen.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/Nullable.hpp"
#include "toolkit/windowing/WindowSystem.hpp"
#include "toolkit/windowing/Window.hpp"

namespace toolkit {

    static bool s_cursorVisible = true;
    static CursorLockMode s_cursorLock = CursorLockMode::kNone;
    static CursorType s_lastVisibleCursor = CursorType::kDefault;


    bool Screen::getIsFullscreen()
    {
        auto win = WindowSystem::getMainWindow();
        return win ? win->isFullscreen() : false;
    }


    void Screen::setIsFullscreen(bool value)
    {
        if (auto win = WindowSystem::getMainWindow())
            win->setFullscreen(value);
    }


    Pos Screen::getSize()
    {
        auto win = WindowSystem::getMainWindow();
        return win ? win->getClientSize() : Pos {0,0};
    }


    void Screen::setSize(const Pos& value)
    {
        if (value.x <= 0 || value.y <= 0)
        {
            LogError("Invalid Screen size to set.");
            return;
        }

        if (auto win = WindowSystem::getMainWindow())
            win->setClientSize(value);
    }


    Pos Screen::screenToGameViewport(const Pos& screenPos)
    {
        auto win = WindowSystem::getMainWindow();
        return win ? win->screenToClient(screenPos) : Pos{0,0};
    }


    Pos Screen::gameViewportToScreen(const Pos& viewportPos)
    {
        auto win = WindowSystem::getMainWindow();
        return win ? win->clientToScreen(viewportPos) : Pos{0,0};
    }


    bool Screen::getCursorVisible()
    {
        return s_cursorVisible;
    }


    void Screen::setCursorVisible(const bool value)
    {
        const auto win = WindowSystem::getMainWindow();
        if (win != nullptr)
        {
            if (!value)
            {
                if (win->getCursor() != CursorType::kHidden)
                    s_lastVisibleCursor = win->getCursor();
                win->setCursor(CursorType::kHidden);
            }
            else
            {
                win->setCursor(s_lastVisibleCursor == CursorType::kHidden ? CursorType::kDefault : s_lastVisibleCursor);
            }
        }
        s_cursorVisible = value;
    }


    CursorLockMode Screen::getCursorLock()
    {
        return s_cursorLock;
    }


    void Screen::setCursorLock(CursorLockMode mode)
    {
        const auto win = WindowSystem::getMainWindow();
        if (win != nullptr)
        {
            if (s_cursorLock == CursorLockMode::kClipped)
                win->endClippingCursor();

            if (mode == CursorLockMode::kNone)
            {
                win->endClippingCursor();
                win->endTrackingMouse();
            }
            else if (mode == CursorLockMode::kClipped)
            {
                win->startClippingCursor(win->getClientBounds());
            }
            else if (mode == CursorLockMode::kLocked)
            {
                // Locked mode recenters the mouse in InputSystem::update and does not use ClipCursor.
                win->endClippingCursor();
            }
        }

        s_cursorLock = mode;
    }

    void Screen::updateScreen(float)
    {
    }

    void Screen::drawScreen()
    {
    }

}
