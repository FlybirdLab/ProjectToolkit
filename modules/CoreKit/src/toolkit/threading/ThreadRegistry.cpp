/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ThreadRegistry.hpp"

#include "Sync.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "Thread.hpp"

namespace toolkit {
    
    TkHashMap<u64, TkThread*> s_registry(64);
    TkMutex mtx;

    TkThread* ThreadRegistry::getThread(u64 id)
    {
        TkThread* result = nullptr;

        mtx.lock();
        s_registry.tryGet(id, result);
        mtx.unlock();

        return result;
    }


    i32 ThreadRegistry::count()
    {
        mtx.lock();
        i32 count = s_registry.count();
        mtx.unlock();

        return count;
    }


    void ThreadRegistry::killEmAll()
    {
        mtx.lock();
        for (auto i = s_registry.Begin(); i.isNotEnd(); ++i)
        {
            i->Value->kill(false);
        }
        mtx.unlock();

        // Now album Kill'Em All from Metallica...
    }


    void ThreadRegistry::add(TkThread* thread)
    {
        TK_ASSERT(thread && thread->getID() != 0);
        mtx.lock();
        TK_ASSERT(!s_registry.containsKey(thread->getID()) && !s_registry.containsValue(thread));
        s_registry.add(thread->getID(), thread);
        mtx.unlock();
    }


    void ThreadRegistry::remove(TkThread* thread)
    {
        TK_ASSERT(thread && thread->getID() != 0);
        mtx.lock();
#if TK_ENABLE_ASSERTION_LOW_LAYERS
        TkThread** currentValue = s_registry.tryGet(thread->getID());
        TK_ASSERT_LOW_LAYER(!currentValue || *currentValue == thread);
#endif
        s_registry.remove(thread->getID());
        mtx.unlock();
    }
}