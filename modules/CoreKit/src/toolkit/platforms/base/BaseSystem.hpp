/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/platforms/System.hpp>

namespace toolkit {

    // Common logic for all platform's System, run before platform's custom logic
    class BaseSystem
    {
    public:
        static bool init();
        static void logInfo();
    };

}