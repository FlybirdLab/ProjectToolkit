/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Task.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/threading/Thread.hpp"
#include "toolkit/base/Math.hpp"

namespace toolkit {
    
    void TkTask::start()
    {
        if (getState() != TaskState::kCreated)
            return;

        onStart();
        setState(TaskState::kQueued);
        enqueue();
    }


    void TkTask::cancel()
    {
        if (TSync::atomicRead(&m_cancelFlag) == 0)
        {
            onCancel();
            if (m_continueWith)
                m_continueWith->cancel();
        }
    }


    bool TkTask::wait(double timeoutMilliseconds) const
    {
        PROFILE();
        double startTime = System::getTimeSeconds() * 1000.0;

        // TODO: no active waiting! use a semaphore!

        do
        {
            auto state = getState();

            // Finished
            if (state == TaskState::kFinished)
            {
                // Wait for child if has
                if (m_continueWith)
                {
                    auto spendTime = System::getTimeSeconds() * 1000.0 - startTime;
                    return m_continueWith->wait(timeoutMilliseconds - spendTime);
                }

                return false;
            }

            // Failed or canceled
            if (state == TaskState::kFailed || state == TaskState::kCanceled)
                return true;

            TkThread::sleep(1);
        } while (timeoutMilliseconds <= 0.0 || System::getTimeSeconds() * 1000.0 - startTime < timeoutMilliseconds);

        // Timeout reached!
        LogWarn("\'%s\' has timed out. Wait time: %i ms", m_name, timeoutMilliseconds);
        return true;
    }


    TkTask* TkTask::continueWith(TkTask* task)
    {
        TK_ASSERT(task != nullptr && task != this);
        if (m_continueWith)
            return m_continueWith->continueWith(task);
        m_continueWith = task;
        return task;
    }


    TkTask* TkTask::continueWith(const TkAction<>& action, void* target)
    {
        TkArray<TkAction<>::TkFuncType> bindings;
        bindings.resize(action.count());
        action.getBindings(bindings.get(), bindings.count());

        TkTask* result = this;
        for (i32 i = 0; i < bindings.count(); i++)
            result = result->continueWith(bindings[i], target);
        return result;
    }


    TkTask* TkTask::continueWith(const TkFunc<void()>& action, void* target)
    {
        TK_ASSERT(action.isBinded());
        return continueWith(TkMemory::newObject<ThreadPoolActionTask>(TkMemory::getDefault(), action, target));
    }


    TkTask* TkTask::continueWithFailable(const TkFunc<bool()>& action, void* target)
    {
        TK_ASSERT(action.isBinded());
        return continueWith(TkMemory::newObject<ThreadPoolActionTask>(TkMemory::getDefault(), action, target));
    }


    TkTask* TkTask::startNew(TkTask* task)
    {
        TK_ASSERT(task);
        task->start();
        return task;
    }

    TkTask* TkTask::startNew(const TkFunc<void()>& action, void* target)
    {
        return startNew(TkMemory::newObject<ThreadPoolActionTask>(TkMemory::getDefault(), action, target));
    }

    TkTask* TkTask::startNew(const TkFunc<bool()>& action, void* target)
    {
        return startNewFailable(action, target);
    }

   
    TkTask* TkTask::startNewFailable(const TkFunc<bool()>& action, void* target)
    {
        return startNew(TkMemory::newObject<ThreadPoolActionTask>(TkMemory::getDefault(), action, target));
    }

    void TkTask::execute()
    {
        if (isCanceled())
            return;

        if (isCancelRequested())
        {
            TK_ASSERT(isQueued());
            setState(TaskState::kCanceled);
            onEnd();
            return;
        }

        TK_ASSERT(isQueued());
        setState(TaskState::kRunning);

        bool failed = run();

        if (isCancelRequested())
        {
            setState(TaskState::kCanceled);
        }
        else if (failed)
        {
            onFail();
        }
        else
        {
            onFinish();
        }
    }

    void TkTask::onStart()
    {
    }

    void TkTask::onFinish()
    {
        TK_ASSERT(isRunning() && !isCancelRequested());
        setState(TaskState::kFinished);

        // Send event further
        if (m_continueWith)
            m_continueWith->start();

        onEnd();
    }

    void TkTask::onFail()
    {
        setState(TaskState::kFailed);

        // Send event further
        if (m_continueWith)
            m_continueWith->onFail();

        onEnd();
    }

    void TkTask::onCancel()
    {
        // Set flag
        TSync::interlockedIncrement(&m_cancelFlag);
        TSync::memoryBarrier();

        // If task is active try to wait for a while
        if (isRunning())
        {
            // Wait for it a little bit
            const double timeout = 2000.0;
            LogWarn("Cannot cancel \'%s\' because it's still running, waiting for end with timeout: %l ms", m_name, timeout);
            wait(timeout);
        }

        const auto state = getState();
        if (state == TaskState::kQueued)
        {
            // The task is still inside the thread-pool queue. Let the worker dequeue it
            // and run onEnd(), otherwise the queue would keep a dangling task pointer.
            return;
        }

        // Don't call OnEnd twice
        if (state != TaskState::kFinished && state != TaskState::kFailed && state != TaskState::kCanceled)
        {
            setState(TaskState::kCanceled);
            onEnd();
        }
    }

    void TkTask::onEnd()
    {
        TK_ASSERT(!isRunning());

        if (m_autoDelete)
            destroy();
    }

    void ThreadPoolTask::cancelAfterDequeue()
    {
        cancel();
        if (isQueued() && isCancelRequested())
            execute();
    }

    static TkMutex s_mtx;
    static TkArray<MainThreadTask*> s_waiting;
    static TkArray<MainThreadTask*> s_queue;

    void TkTask::disposePendingMainThreadTasks()
    {
        s_mtx.lock();
        for (i32 i = 0; i < s_waiting.count(); i++)
        {
            s_waiting[i]->cancel();
        }
        for (i32 i = 0; i < s_queue.count(); i++)
        {
            s_queue[i]->cancel();
        }
        s_waiting.clear();
        s_queue.clear();
        s_mtx.unlock();
    }

    void MainThreadTask::runAll(float dt)
    {
        PROFILE();
        s_mtx.lock();
        for (i32 i = s_waiting.count() - 1; i >= 0; i--)
        {
            auto task = s_waiting[i];
            task->initialDelay -= dt;
            if (task->initialDelay < TkMath::ZeroTolerance)
            {
                s_waiting.removeAt(i);
                s_queue.add(task);
            }
        }
        for (i32 i = 0; i < s_queue.count(); i++)
        {
            s_queue[i]->execute();
        }
        s_queue.clear();
        s_mtx.unlock();
    }

    void MainThreadTask::enqueue()
    {
        s_mtx.lock();
        if (initialDelay <= TkMath::ZeroTolerance)
            s_queue.add(this);
        else
            s_waiting.add(this);
        s_mtx.unlock();
    }

    bool MainThreadActionTask::run()
    {
        if (m_action.isBinded())
        {
            m_action();
            return false;
        }
        return false;
    }

    bool MainThreadBoolActionTask::run()
    {
        if (m_action.isBinded())
        {
            return m_action();
        }
        return false;
    }

}
