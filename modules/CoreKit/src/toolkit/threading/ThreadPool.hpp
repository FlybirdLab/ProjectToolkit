/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target : ThreadPoolTest.cpp
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{
    class TkThreadPool
    {
        friend class TkThreadPoolTask;
        friend class TkThreadPoolService;

    public:
        static bool init();
        static void beforeExit();
        static void dispose();

    private:
        static i32 threadProc();
    };

    
}