/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "JobSys.hpp"

#include "toolkit/base/TypeTraits.hpp"
#include "toolkit/base/Function.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/collection/RingBuffer.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/threading/Runnable.hpp"
#include "toolkit/threading/Thread.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/types/Span.hpp"

namespace toolkit
{
    class JobsAllocator;

    struct JobData
    {
        i32 index;
        i64 JobKey;
    };

    template <>
    struct TIsPODType<JobData>
    {
        enum
        {
            Value = true
        };
    };

    struct JobContext
    {
        volatile i64 left;
        i32 depsLeft;
        TkFunc<void(i32)> job;
        TkArray<i64> deps;
    };

    template <>
    struct TIsPODType<JobContext>
    {
        enum
        {
            Value = false
        };
    };

    class JobSysThread : public IRunnable
    {
    public:
        u64 index;

    public:
        i32 onRun() override;

        void afterWork(bool wasKilled) override
        {
            TkMemory::deletePtr(TkMemory::getDefault(), this);
        }
    };

    static ThreadHandle s_thds[TK_PLATFORM_THREADS_LIMIT / 2] = {};
    static i32 s_thdsCount = 0;
    static TkAtomic<bool> s_jobStartOnDispatch{true};
    static volatile i64 s_exitFlag = 0;
    static volatile i64 s_jobLabel = 0;
    static TkHashMap<i64, JobContext> s_jobCtxs;
    static TkCondVar s_jobsSig;
    static TkMutex s_jobsMtx;
    static TkCondVar s_waitSig;
    static TkMutex s_waitMtx;
    static TkMutex s_JobsMtx;
    static TkRingBuffer<JobData> s_jobs;

    bool TkJobSys::init()
    {
        s_thdsCount = TkMath::min<i32>(System::getCPUStats().logicalCoreCount, lengthOf(s_thds));
        for (i32 i = 0; i < s_thdsCount; i++)
        {
            auto runnable = TkMemory::newObject<JobSysThread>(TkMemory::getDefault());
            runnable->index = (u64)i;
            auto thread = TkThread::newThread(runnable, TkString::format("Job System %i", i), TkMemory::getDefault(), ThreadPriority::kAboveNormal, 0);
            if (thread == nullptr)
            {
                for (i32 j = 0; j < i; j++)
                {
                    if (s_thds[j])
                    {
                        s_thds[j]->kill(true);
                        TkMemory::deletePtr(TkMemory::getDefault(), s_thds[j]);
                        s_thds[j] = nullptr;
                    }
                }
                s_thdsCount = 0;
                return false;
            }
            s_thds[i] = thread;
        }
        return true;
    }

    void TkJobSys::beforeExit()
    {
        TSync::atomicStore(&s_exitFlag, 1);
        s_jobsSig.notifyAll();
    }

    void TkJobSys::dispose()
    {
        TSync::atomicStore(&s_exitFlag, 1);
        s_jobsSig.notifyAll();
        TkThread::sleep(1);

        for (i32 i = 0; i < s_thdsCount; i++)
        {
            if (s_thds[i])
            {
                s_thds[i]->kill(true);
                TkMemory::deletePtr(TkMemory::getDefault(), s_thds[i]);
                s_thds[i] = nullptr;
            }
        }

        s_jobCtxs.setCapacity(0);
        s_jobs.release();
    }

    i32 JobSysThread::onRun()
    {
        TkThread::setThreadAffinityMask(1ull << index);

        JobData data;
        TkFunc<void(i32)> job;
        bool attachCSharpThread = true;
        while (TSync::atomicRead(&s_exitFlag) == 0)
        {
            // FIX: 
            if (!s_jobStartOnDispatch.load(AtomicMemOrder::kAcquire))
            {
                s_jobsMtx.lock();
                s_jobsSig.wait(s_jobsMtx);
                s_jobsMtx.unlock();
                continue;
            }

            // Try to get a job
            s_JobsMtx.lock();
            if (s_jobs.count() != 0)
            {
                data = s_jobs.peekFront();
                s_jobs.popFront();
                const JobContext &context = ((const TkHashMap<i64, JobContext> &)s_jobCtxs).at(data.JobKey);
                job = context.job;
            }
            s_JobsMtx.unlock();

            if (job.isBinded())
            {
                // Run job
                job(data.index);

                // Move forward with the job queue
                bool notifyWaiting = false;
                s_JobsMtx.lock();
                JobContext &context = s_jobCtxs.at(data.JobKey);
                if (TSync::interlockedDecrement(&context.left) <= 0)
                {
                    // Update any dependant jobs
                    for (i64 dependant : context.deps)
                    {
                        JobContext &dependantContext = s_jobCtxs.at(dependant);
                        if (--dependantContext.depsLeft <= 0)
                        {
                            // Dispatch dependency when it's ready
                            JobData dependantData;
                            dependantData.JobKey = dependant;
                            for (dependantData.index = 0; dependantData.index < dependantContext.left; dependantData.index++)
                                s_jobs.pushBack(dependantData);
                        }
                    }

                    // Remove completed context
                    s_jobCtxs.remove(data.JobKey);
                    notifyWaiting = true;
                }
                s_JobsMtx.unlock();
                if (notifyWaiting)
                    s_waitSig.notifyAll();

                job.unbind();
            }
            else
            {
                // Wait for signal
                s_jobsMtx.lock();
                s_jobsSig.wait(s_jobsMtx);
                s_jobsMtx.unlock();
            }
        }
        return 0;
    }

    void TkJobSys::execute(const TkFunc<void(i32)> &job, i32 jobCount)
    {
        // TODO: disable async if called on job thread? or maybe Wait should handle waiting in job thread to do the processing?
        if (jobCount > 1)
        {
            // Async
            const i64 jobWaitHandle = dispatch(job, jobCount);
            wait(jobWaitHandle);
        }
        else
        {
            // Sync
            for (i32 i = 0; i < jobCount; i++)
                job(i);
        }
    }

    i64 TkJobSys::dispatch(const TkFunc<void(i32)> &job, i32 jobCount)
    {
        if (jobCount <= 0)
            return 0;
        PROFILE();
        const auto label = TSync::interlockedAdd(&s_jobLabel, (i64)jobCount) + jobCount;

        JobData data;
        data.JobKey = label;

        JobContext context;
        context.job = job;
        context.left = jobCount;
        context.depsLeft = 0;

        s_JobsMtx.lock();
        s_jobCtxs.add(label, moveTemp(context));
        for (data.index = 0; data.index < jobCount; data.index++)
            s_jobs.pushBack(data);
        s_JobsMtx.unlock();

        if (s_jobStartOnDispatch.load(AtomicMemOrder::kAcquire))
        {
            if (jobCount == 1)
                s_jobsSig.notifyOne();
            else
                s_jobsSig.notifyAll();
        }

        return label;
    }

    i64 TkJobSys::dispatch(const TkFunc<void(i32)> &job, TkSpan<i64> dependencies, i32 jobCount)
    {
        if (jobCount <= 0)
            return 0;
        PROFILE();
        const auto label = TSync::interlockedAdd(&s_jobLabel, (i64)jobCount) + jobCount;

        JobData data;
        data.JobKey = label;

        JobContext context;
        context.job = job;
        context.left = jobCount;
        context.depsLeft = 0;

        s_JobsMtx.lock();
        for (i64 dependency : dependencies)
        {
            JobContext *dependencyContext = nullptr;
            if (s_jobCtxs.containsKey(dependency))
            {
                dependencyContext = &s_jobCtxs[dependency];
                context.depsLeft++;
                dependencyContext->deps.add(label);
            }
        }
        s_jobCtxs.add(label, moveTemp(context));
        if (context.depsLeft == 0)
        {
            // No dependencies left to complete so dispatch now
            for (data.index = 0; data.index < jobCount; data.index++)
                s_jobs.pushBack(data);
        }
        s_JobsMtx.unlock();

        if (context.depsLeft == 0 && s_jobStartOnDispatch.load(AtomicMemOrder::kAcquire))
        {
            if (jobCount == 1)
                s_jobsSig.notifyOne();
            else
                s_jobsSig.notifyAll();
        }

        return label;
    }

    void TkJobSys::wait()
    {
        s_JobsMtx.lock();
        i32 numJobs = s_jobCtxs.count();
        s_JobsMtx.unlock();

        while (numJobs > 0)
        {
            s_waitMtx.lock();
            s_waitSig.wait(s_waitMtx, 1);
            s_waitMtx.unlock();

            s_JobsMtx.lock();
            numJobs = s_jobCtxs.count();
            s_JobsMtx.unlock();
        }
    }

    void TkJobSys::wait(i64 label)
    {
        PROFILE();

        while (TSync::atomicRead(&s_exitFlag) == 0)
        {
            s_JobsMtx.lock();
            JobContext *context = nullptr;
            if (s_jobCtxs.containsKey(label))
            {
                context = &s_jobCtxs[label];
            }
            s_JobsMtx.unlock();

            // Skip if context has been already executed (last job removes it)
            if (!context)
                break;

            // Wait on signal until input label is not yet done
            s_waitMtx.lock();
            s_waitSig.wait(s_waitMtx, 1);
            s_waitMtx.unlock();

            // Wake up any thread to prevent stalling in highly multi-threaded environment
            s_jobsSig.notifyOne();
        }
    }
    void TkJobSys::setJobStartingOnDispatch(bool value)
    {
        bool old = s_jobStartOnDispatch.exchange(value, AtomicMemOrder::kAcquire);

        // FIX: only arouse on manually dispatch
        if (!old && value)
        {
            s_JobsMtx.lock();
            const i32 count = s_jobs.count();
            s_JobsMtx.unlock();

            if (count == 1)
                s_jobsSig.notifyOne();
            else if (count != 0)
                s_jobsSig.notifyAll();
        }
    }

    i32 TkJobSys::getThreadsCount()
    {
        return s_thdsCount;
    }
}
