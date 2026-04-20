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

    static TkNullable<bool> s_fullscreen;
    static TkNullable<Pos> s_size;
    static bool s_cursorVisible = true;
    static CursorLockMode s_cursorLock = CursorLockMode::kNone;


    bool Screen::getIsFullscreen()
    {
        auto win = WindowSystem::getMainWindow();
        return win ? win->isFullscreen() : false;
    }


    void Screen::setIsFullscreen(bool value)
    {
        s_fullscreen = value;
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

        s_size = value;
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
        //TODO if (win && Engine::hasGameViewportFocus())
        // {
        //     setCursor(win, value ? CursorType::kDefault : CursorType::kHidden);
        // }
        s_cursorVisible = value;
    }


    CursorLockMode Screen::getCursorLock()
    {
        return s_cursorLock;
    }


    void Screen::setCursorLock(CursorLockMode mode)
    {
        const auto win = WindowSystem::getMainWindow();
        if (win && mode == CursorLockMode::kClipped)
        {
            Rect bounds = win->getClientBounds();
            win->startClippingCursor(bounds);
        }
        else if (win && s_cursorLock == CursorLockMode::kClipped)
        {
            win->endClippingCursor();
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
