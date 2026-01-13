/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <toolkit/platforms/unix/UnixThread.hpp>

#if TK_LINUX

namespace toolkit {
    
    void ThreadImpl::killInternal(bool waitForJoin) {
        if (waitForJoin)
            pthread_join(m_thread, nullptr);
        pthread_kill(m_thread, SIGKILL);
    }


    u32 ThreadImpl::getStackSize(u32 s) {
        return s;
    }


    i32 ThreadImpl::start(pthread_attr_t& attr) {
        const int result = pthread_create(&m_thread, &attr, threadProc, t);
        if (result == 0)
            pthread_setname_np(m_thread, m_name.toStringAnsi().get());
        return result;
    }


    // Linux & Android version
    i32 ThreadImpl::convertType(ThreadPriority t)
    {
        switch (t)
        {
        case ThreadPriority::kHighest:
            return 30;
        case ThreadPriority::kAboveNormal:
            return 25;
        case ThreadPriority::kNormal:
            return 15;
        case ThreadPriority::kBelowNormal:
            return 5;
        case ThreadPriority::kLowest:
            return 1;
        }
        return 0;
    }
}

#endif