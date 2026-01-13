/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/platforms/System.hpp>

#if TK_WINDOWS

namespace toolkit {

    struct Win32SystemData {
        u64 mainThreadId;
    };

    class Win32System
    {
    public:
        static Win32SystemData* getData();

    public:
        // Invoke in main function to setup toolkit platform data
        static bool init();
        // Log the platform's information to console
        static void logInfo();
        // Setup platform service before run application
        static void beforeRun();
        // Update platform service in main loop for handling events or others.
        static void update();
        // Invoke before application quit to clean up
        static void beforeQuit();
        // For application quit platform service
        static void quit();
    };

}

#endif