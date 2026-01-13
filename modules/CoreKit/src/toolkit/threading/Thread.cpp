/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Thread.hpp"

#if TK_WINDOWS
#include <toolkit/platforms/win32/Win32Thread.hpp>
#elif TK_POSIX
#include <toolkit/platforms/unix/UnixThread.hpp>
#endif

namespace toolkit {
    
    static constexpr const char* s_ThreadPriority[] {
        "Normal",
        "Highest",
        "AboveNormal",
        "BelowNormal",
        "Lowest"
    };
    const char* toString(ThreadPriority s) {
        return s_ThreadPriority[(i32)s];
    }

    TkThread::TkThread(ThreadImpl* impl)
        : m_impl {impl}
    {}
}