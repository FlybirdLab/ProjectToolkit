/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit
{
    enum class ThreadPriority : u8
    {
        kNormal = 0,
        kHighest,
        kAboveNormal,
        kBelowNormal,
        kLowest,
    };
    const char *toString(ThreadPriority);

    class IRunnable;
    class ThreadImpl;


    struct TkThread
    {
        friend struct ThreadImpl;
        friend struct WrappedThread;
    public:
        ThreadPriority getPriority();
        void setPriority(ThreadPriority priority);
        IAllocator& getAllocator();
        u64 getID();
        bool isRunning();
        const TkString& getName();

        void kill(bool waitForJoin = false);
        void join();

        struct IRunnable* getRunnable();

    public:
        static TkAction<struct TkThread*> ThreadStarting;
        static TkAction<TkThread*, i32> ThreadExiting;

    public:
        static void setThreadPriority(ThreadPriority priority);
        static void setThreadAffinityMask(u64 affinityMask);
        static u64 getCurrentThreadID();

        static void sleep(i32 milliseconds);
        static bool isInMainThread();
        static u64 getMainThreadId();

    public:
        static TkThread* newThread(struct IRunnable*, const TkString& name, ThreadPriority priority, IAllocator& alloc = TkMemory::getDefault());
        static TkThread* newThread(IRunnable* runnable, const TkString& name, IAllocator& alloc = TkMemory::getDefault(), ThreadPriority priority = ThreadPriority::kNormal, u32 stackSize = 0);
        static void destroy(TkThread*);

    protected:
        TkThread(struct ThreadImpl* impl);

        ThreadImpl* m_impl;
    };


    typedef struct TkThread* ThreadHandle;
    constexpr ThreadHandle INVALID_HANDLE = nullptr;
}