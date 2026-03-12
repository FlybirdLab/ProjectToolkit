/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Profiler.hpp"

#include "toolkit/collection/Array.hpp"
#include "toolkit/threading/Thread.hpp"
#include "toolkit/threading/ThreadRegistry.hpp"
#include "toolkit/platforms/System.hpp"

namespace toolkit {
    
    TK_THREAD_LOCAL DebugThread* DebugThread::Current = nullptr;
    TkStaticArray<DebugThread*, 64> TkProfiler::Threads;
    bool TkProfiler::Enabled = false;

    DebugEventBuffer::DebugEventBuffer()
    {
        m_capacity = 8192;
        m_capacityMask = m_capacity - 1;
        m_data = TkMemory::newArray<DebugEvent>(m_capacity);
        m_head = 0;
        m_count = 0;
    }


    DebugEventBuffer::~DebugEventBuffer()
    {
        TkMemory::deleteArray(TkMemory::getDefault(), m_data, m_capacity);
    }


    void DebugEventBuffer::extract(TkArray<DebugEvent>& data, bool withRemoval)
    {
        data.clear();

        // Peek ring buffer state
        i32 count = m_count;
        const i32 capacity = m_capacity;

        // Skip if empty
        if (count == 0)
            return;

        // Fix iterators when buffer is full (begin == end)
        if (count == capacity)
        {
            m_count--;
            count--;
        }

        // Find the first item (skip non-root events)
        Iterator firstEvent = begin();
        for (auto i = firstEvent; i.isNotEnd(); ++i)
        {
            if (i.event().depth == 0)
            {
                firstEvent = i;
                break;
            }
        }

        // Skip if no root event found inside the buffer
        if (firstEvent.isEnd())
            return;

        // Find the last item (last event in ended root event)
        Iterator lastEndedRoot = end();
        for (auto i = last(); i != firstEvent; --i)
        {
            if (i.event().depth == 0 && i.event().end > 0)
            {
                lastEndedRoot = i;
                break;
            }
        }

        // Skip if no finished root event found inside the buffer
        if (lastEndedRoot.isEnd())
            return;

        // Find the last non-root event in last root event
        Iterator lastEvent = lastEndedRoot;
        const double lastRootEventEndTime = lastEndedRoot.event().end;
        for (auto i = --end(); i != lastEndedRoot; --i)
        {
            if (i.event().end > 0 && i.event().end <= lastRootEventEndTime)
            {
                lastEvent = i;
                break;
            }
        }

        if (withRemoval)
        {
            // Remove all the events between [begin(), lastEvent]
            m_count -= (lastEvent.index() - begin().index()) & m_capacityMask;
        }

        // Extract all the events between [firstEvent, lastEvent]
        const i32 head = (lastEvent.index() + 1) & m_capacityMask;
        count = (lastEvent.index() - firstEvent.index() + 1) & m_capacityMask;
        data.resize(count, false);
        const i32 tail = (head - count) & m_capacityMask;
        const i32 spaceLeft = capacity - tail;
        const i32 spaceLeftCount = TkMath::min(spaceLeft, count);
        const i32 overflow = count - spaceLeft;
        TkMemory::memoryCopy(data.get(), &m_data[tail], spaceLeftCount * sizeof(DebugEvent));
        if (overflow > 0)
            TkMemory::memoryCopy(data.get() + spaceLeftCount, &m_data[0], overflow * sizeof(DebugEvent));
    }


    i32 DebugThread::beginEvent()
    {
        const double time = System::getTimeSeconds() * 1000.0;
        const auto index = Buffer.add();
        DebugEvent& e = Buffer.get(index);
        e.start = time;
        e.end = 0;
        e.depth = m_depth++;
        e.heapMemAlloc = 0;
        return index;
    }


    void DebugThread::endEvent(i32 index)
    {
        const double time = System::getTimeSeconds() * 1000.0;
        m_depth--;
        DebugEvent& e = Buffer.get(index);
        e.end = time;
    }


    void DebugThread::endEvent()
    {
        const double time = System::getTimeSeconds() * 1000.0;
        m_depth--;
        for (auto i = Buffer.last(); i != Buffer.begin(); --i)
        {
            DebugEvent& e = i.event();
            if (e.end <= 0)
            {
                e.end = time;
                break;
            }
        }
    }


    bool TkProfiler::isProfilingCurrentThread()
    {
        return Enabled && DebugThread::Current != nullptr;
    }


    DebugThread* TkProfiler::getCurrentThread()
    {
        return Enabled ? DebugThread::Current : nullptr;
    }


    i32 TkProfiler::beginEvent()
    {
        if (!Enabled)
            return -1;
        auto thread = DebugThread::Current;
        if (thread == nullptr)
        {
            const auto id = TkThread::getCurrentThreadID();
            const auto t = ThreadRegistry::getThread(id);
            if (t)
                thread = TkMemory::newObject<DebugThread>(TkMemory::getDefault(), t->getName());
            else if (id == TkThread::getMainThreadId())
                thread = TkMemory::newObject<DebugThread>(TkMemory::getDefault(), "Main");
            else
                thread = TkMemory::newObject<DebugThread>(TkMemory::getDefault(), "Thread");

            DebugThread::Current = thread;
            Threads.add(thread);
        }
        return thread->beginEvent();
    }

    
    i32 TkProfiler::beginEvent(const char* name)
    {
        if (!Enabled)
            return -1;
        const auto index = beginEvent();
        const auto thread = DebugThread::Current;
        auto& e = thread->Buffer.get(index);
        auto dst = e.name;
        auto src = name;
        if (src)
        {
            const auto end = dst + lengthOf(e.name) - 1;
            while (*src && dst != end)
                *dst++ = *src++;
        }
        *dst = 0;
        return index;
    }


    void TkProfiler::endEvent(i32 index)
    {
        if (index != -1 && DebugThread::Current)
            DebugThread::Current->endEvent(index);
    }


    void TkProfiler::endEvent()
    {
        if (Enabled && DebugThread::Current)
            DebugThread::Current->endEvent();
    }


    void TkProfiler::dispose()
    {
        Enabled = false;
        for (i32 i = 0; i < Threads.count(); i++)
        {
            TkMemory::deletePtr(TkMemory::getDefault(), Threads[i]);
        }
        Threads.clear();
        DebugThread::Current = nullptr;

        // Cleanup memory, note: calls to profiler after this point will end up with a crash (DebugThread::Current is invalid)
    }

}
