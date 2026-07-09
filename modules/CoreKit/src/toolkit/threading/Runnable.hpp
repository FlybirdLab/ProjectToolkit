/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Function.hpp>

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

    class Runnable : public IRunnable
    {
    public:
        // Working function
        TkFunc<i32()> onWork;

        Runnable(bool autoDelete)
            : m_autoDelete(autoDelete)
        {
        }
        virtual ~Runnable() = default;

        i32 onRun() override
        {
            i32 result = -1;
            if (onWork.isBinded())
                result = onWork();
            return result;
        }

        void afterWork(bool wasKilled) override
        {
            if (m_autoDelete)
                TkMemory::deletePtr(this);
        }

    private:
        bool m_autoDelete;
    };
}