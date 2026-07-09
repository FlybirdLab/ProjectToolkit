/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {

    // Holds all created threads (except the main thread)
    class ThreadRegistry
    {
    public:
        static TkThread* getThread(u64 id);
        static i32 count();
        static void killEmAll();

        static void add(TkThread* thread);
        static void remove(TkThread* thread);
    };

}