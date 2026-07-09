/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ThreadRegistry.hpp"

#include "Sync.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "Thread.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{
    static TkHashMap<u64, TkThread *> s_registry{64};
    static TkMutex s_mtx{};

    TkThread *ThreadRegistry::getThread(u64 id)
    {
        TkThread *result = nullptr;

        s_mtx.lock();
        if (s_registry.containsKey(id))
        {
            result = s_registry[id];
        }
        s_mtx.unlock();

        return result;
    }

    i32 ThreadRegistry::count()
    {
        s_mtx.lock();
        i32 count = s_registry.count();
        s_mtx.unlock();

        return count;
    }

    void ThreadRegistry::killEmAll()
    {
        s_mtx.lock();
        for (auto i = s_registry.Begin(); i.isNotEnd(); ++i)
        {
            i->value->kill(false);
        }
        s_mtx.unlock();

        // Now album Kill'Em All from Metallica...
    }

    void ThreadRegistry::add(TkThread *thread)
    {
        if (!thread || thread->getID() == 0)
        {
            LogWarn("[ThreadReg] Empty thread trying to add to reg. %i.", thread != nullptr ? thread->getID() : -1);
            return;
        }
        s_mtx.lock();
        TK_ASSERT(!s_registry.containsKey(thread->getID()) && !s_registry.containsValue(thread));
        s_registry.add(thread->getID(), thread);
        s_mtx.unlock();
    }

    void ThreadRegistry::remove(TkThread *thread)
    {
        TK_ASSERT(thread && thread->getID() != 0);
        s_mtx.lock();
#if TK_ENABLE_ASSERTION_LOW_LAYERS
        TkThread **currentValue = s_registry.tryGet(thread->getID());
        TK_ASSERT_LOW_LAYER(!currentValue || *currentValue == thread);
#endif
        s_registry.remove(thread->getID());
        s_mtx.unlock();
    }
}