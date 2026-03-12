/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ThreadPool.hpp"

#include "Task.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/base/Math.hpp"
#include "Sync.hpp"
#include "Thread.hpp"
#include "toolkit/debug/Logger.hpp"

#define MOODYCAMEL_EXCEPTIONS_ENABLED 0
#include <concurrentqueue.h>

namespace toolkit
{
    struct ConcurrentQueueSettings : public moodycamel::ConcurrentQueueDefaultTraits
    {
        // Use bigger blocks
        static const size_t BLOCK_SIZE = 256;

        // Use default engine memory allocator
        static inline void *malloc(size_t size)
        {
            return TkMemory::allocate(TkMemory::getDefault(), (u64)size);
        }

        // Use default engine memory allocator
        static inline void free(void *ptr)
        {
            return TkMemory::free(TkMemory::getDefault(), ptr);
        }
    };

    template <typename T>
    class ConcurrentQueue : public moodycamel::ConcurrentQueue<T, ConcurrentQueueSettings>
    {
    public:
        typedef moodycamel::ConcurrentQueue<T, ConcurrentQueueSettings> Base;

    public:
        TK_FORCE_INLINE i32 count() const
        {
            return static_cast<i32>(Base::size_approx());
        }

        TK_FORCE_INLINE void add(const T &item)
        {
            enqueue(item);
        }

        TK_FORCE_INLINE void add(T &&item)
        {
            enqueue(item);
        }
    };

    // Lock-free implementation of thread-safe tasks queue.
    template <typename T>
    class ConcurrentTaskQueue : public ConcurrentQueue<T *>
    {
    public:
        // Adds item to the collection (thread-safe).
        TK_FORCE_INLINE void add(T *item)
        {
            ConcurrentQueue<T *>::enqueue(item);
        }

        // Cancels all the tasks from the queue and removes them.
        void cancelAll()
        {
            T *tasks[16];
            std::size_t count;
            while ((count = ConcurrentQueue<T *>::try_dequeue_bulk(tasks, lengthOf(tasks))) != 0)
            {
                for (std::size_t i = 0; i != count; i++)
                {
                    tasks[i]->cancel();
                }
            }
        }
    };

    static volatile i64 s_exitFlag = 0;
    static TkArray<ThreadHandle> s_threads;
    static ConcurrentTaskQueue<ThreadPoolTask> s_jobs;
    static TkCondVar s_jobsSignal;
    static TkMutex s_jobsMutex;

    void ThreadPoolTask::enqueue()
    {
        s_jobs.add(this);
        s_jobsSignal.notifyOne();
    }

    bool TkThreadPool::init()
    {
        // Spawn threads
        const i32 nums_threads = TkMath::clamp<i32>(System::getCPUStats().coreCount - 1, 2, TK_PLATFORM_THREADS_LIMIT / 2);
        LogInfo("[ThreadPool] Spawning %i Thread Pool workers", nums_threads);
        for (i32 i = s_threads.count(); i < nums_threads; i++)
        {
            // Create tread
            auto runnable = TkMemory::newObject<Runnable>(TkMemory::getDefault(), true);
            runnable->onWork.bind(TkThreadPool::threadProc);
            auto thread = TkThread::newThread(runnable, TkString::format("Thread Pool %i", i));
            if (thread == nullptr)
            {
                LogError("[ThreadPool] Failed to spawn %i thread in the Thread Pool", i + 1);
                return false;
            }

            // Add to the list
            s_threads.add(thread);
        }

        return true;
    }

    void TkThreadPool::beforeExit()
    {
        // Set exit flag and wake up threads
        TSync::atomicStore(&s_exitFlag, 1);
        s_jobsSignal.notifyAll();
    }

    void TkThreadPool::dispose()
    {
        // Set exit flag and wake up threads
        TSync::atomicStore(&s_exitFlag, 1);
        s_jobsSignal.notifyAll();

        // Wait some time
        TkThread::sleep(10);

        // Delete threads
        for (i32 i = 0; i < s_threads.count(); i++)
        {
            s_threads[i]->kill(true);
        }
        s_threads.clearDelete();
        s_jobs.cancelAll();
    }

    i32 TkThreadPool::threadProc()
    {
        ThreadPoolTask *task;

        // Work until end
        while (TSync::atomicRead(&s_exitFlag) == 0)
        {
            // Try to get a job
            if (s_jobs.try_dequeue(task))
            {
                task->execute();
            }
            else
            {
                s_jobsMutex.lock();
                s_jobsSignal.wait(s_jobsMutex);
                s_jobsMutex.unlock();
            }
        }

        return 0;
    }
}
