/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */
#pragma once

#include <toolkit/input/InputDevice.hpp>

namespace toolkit
{
    class Screen
    {
    public:
        // Gets the fullscreen mode.
        static bool getIsFullscreen();
        // Sets the fullscreen mode.
        static void setIsFullscreen(bool value);
        // Gets the window size.
        static Pos getSize();
        // Converts the screen-space position to the game viewport position.
        static Pos screenToGameViewport(const Pos &screenPos);
        // Converts the game viewport position to the screen-space position.
        static Pos gameViewportToScreen(const Pos &viewportPos);
        // Sets the window size.
        static void setSize(const Pos &value);
        // Gets the cursor visible flag.
        static bool getCursorVisible();
        // Sets the cursor visible flag.
        static void setCursorVisible(const bool value);
        // Gets the cursor lock mode.
        static CursorLockMode getCursorLock();
        // Sets the cursor lock mode.
        static void setCursorLock(CursorLockMode mode);

    public:
        static void updateScreen(float dt);
        static void drawScreen();
    };

}