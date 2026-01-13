/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Singleton.hpp"

namespace toolkit {
    
#if TK_DEBUG
    i32 g_singletonsAllocated = 0;

    class SingletonsAllocatedChecker
    {
    public:
        SingletonsAllocatedChecker()
        {
            g_singletonsAllocated = 0;
        }

        ~SingletonsAllocatedChecker()
        {
            TK_ASSERT(g_singletonsAllocated == 0);
        }
    };

    SingletonsAllocatedChecker g_singletonsAllocatedChecker;
#endif

}