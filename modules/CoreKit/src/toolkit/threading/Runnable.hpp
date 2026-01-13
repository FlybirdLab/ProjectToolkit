/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {

    class IRunnable
    {
    public:
        virtual ~IRunnable() = default;

        virtual bool onInit()
        {
            return true;
        }

        virtual i32 onRun() = 0;
        
        virtual void onStop()
        {
        }
        
        virtual void onExit()
        {
        }
        
        virtual void afterWork(bool wasKilled)
        {
        }
    };

}