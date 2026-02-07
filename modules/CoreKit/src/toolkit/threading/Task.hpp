/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target : ThreadPoolTest.cpp
 */

#pragma once

#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/base/Function.hpp>

namespace toolkit
{

    // Represents the current stage in the lifecycle of a Task.
    enum class TaskState : i64
    {
        kCreated = 0,
        kFailed,
        kCanceled,
        kQueued,
        kRunning,
        kFinished
    };

    // Represents an asynchronous operation.
    class TkTask : public NonCopyable
    {
    public:
        // Gets the task state.
        TK_FORCE_INLINE TaskState getState() const
        {
            return (TaskState)TSync::atomicRead((i64 const volatile *)&m_state);
        }

        TK_FORCE_INLINE TkTask *getContinueWithTask() const
        {
            return m_continueWith;
        }
        TK_FORCE_INLINE bool isFailed() const
        {
            return getState() == TaskState::kFailed;
        }
        TK_FORCE_INLINE bool isCanceled() const
        {
            return getState() == TaskState::kCanceled;
        }
        TK_FORCE_INLINE bool isQueued() const
        {
            return getState() == TaskState::kQueued;
        }
        TK_FORCE_INLINE bool isRunning() const
        {
            return getState() == TaskState::kRunning;
        }
        TK_FORCE_INLINE bool isFinished() const
        {
            return getState() == TaskState::kFinished;
        }

        // Checks if operation has been ended (via cancel, fail or finish).
        bool isEnded() const
        {
            auto state = getState();
            return state == TaskState::kFailed || state == TaskState::kCanceled || state == TaskState::kFinished;
        }

        // Returns true if task has been requested to cancel it's operation.
        TK_FORCE_INLINE bool isCancelRequested()
        {
            return TSync::atomicRead(&m_cancelFlag) != 0;
        }

    public:
        // Starts this task execution (and will continue with all children).
        void start();
        // Cancels this task (and all child tasks).
        void cancel();

        // Waits the specified timeout for the task to be finished.
        TK_FORCE_INLINE bool wait(const TkTimeSpan &timeout) const
        {
            return wait(timeout.getTotalMilliseconds());
        }

        // Waits the specified timeout for the task to be finished.
        bool wait(double timeoutMilliseconds = -1) const;

        // Waits for all the tasks from the list.
        template <class T = TkTask>
        static bool waitAll(TkArray<T *> &tasks, double timeoutMilliseconds = -1)
        {
            for (i32 i = 0; i < tasks.count(); i++)
            {
                if (tasks[i]->wait())
                    return true;
            }
            return false;
        }

    public:
        // Continues that task execution with a given task (will call Start on given task after finishing that one).
        TkTask *continueWith(TkTask *task);
        // Continues that task execution with a given action (will spawn new async action).
        TkTask *continueWith(const TkAction<> &action, void *target = nullptr);
        // Continues that task execution with a given action (will spawn new async action).
        TkTask *continueWith(const TkFunc<void()> &action, void *target = nullptr);
        // Continues that task execution with a given action (will spawn new async action).
        TkTask *continueWithFailable(const TkFunc<bool()> &action, void *target = nullptr);

    public:
        static TkTask *startNew(TkTask *task);
        static TkTask *startNew(const TkFunc<void()> &action, void *target = nullptr);
        template <class T, void (T::*Method)()>
        static TkTask *startNew(T *callee)
        {
            TkFunc<void()> action;
            action.bind<T, Method>(callee);
            return startNew(action, dynamic_cast<void *>(callee));
        }
        static TkTask *startNewFailable(const TkFunc<bool()> &action, void *target = nullptr);
        template <class T, bool (T::*Method)()>
        static TkTask *startNewFailable(T *callee)
        {
            TkFunc<bool()> action;
            action.bind<T, Method>(callee);
            return startNewFailable(action, dynamic_cast<void *>(callee));
        }

        // Cancels all the tasks from the list and clears it.
        template <class T = TkTask>
        static void cancelAll(TkArray<T *> &tasks)
        {
            for (i32 i = 0; i < tasks.count(); i++)
                tasks[i]->cancel();
            tasks.clear();
        }

        void setName(const char *name)
        {
            m_name = name;
        }
        const char *getName() const
        {
            return m_name;
        }

    protected:
        // Executes this task. It should be called by the task consumer (thread pool or other executor of this task type). It calls run() and handles result).
        void execute();

        // Runs the task specified operations. It does not handle any task related logic, but only performs the actual job.
        virtual bool run() = 0;

    protected:
        virtual void enqueue() = 0;
        virtual void onStart();
        virtual void onFinish();
        virtual void onFail();
        virtual void onCancel();
        virtual void onEnd();

    protected:
        // The cancel flag used to indicate that there is request to cancel task operation.
        volatile i64 m_cancelFlag = 0;
        // The current task state.
        volatile TaskState m_state = TaskState::kCreated;
        const char *m_name;

        // The task to start after finish.
        TkTask *m_continueWith = nullptr;

        void setState(TaskState state)
        {
            TSync::atomicStore((i64 volatile *)&m_state, (u64)state);
        }
    };

    // General purpose task executed using Thread Pool.
    class ThreadPoolTask : public TkTask
    {
        friend class TkThreadPool;

    protected:
        ThreadPoolTask() : TkTask() {}

    public:
        void enqueue() override;
    };

    // General purpose task executing custom action using Thread Pool.
    class ThreadPoolActionTask : public ThreadPoolTask
    {
    protected:
        TkFunc<void()> m_action1;
        TkFunc<bool()> m_action2;
        void *m_target;

    public:
        ThreadPoolActionTask(const TkFunc<void()> &action, void *target = nullptr)
            : ThreadPoolTask(), m_action1(action), m_target(target) {}
        ThreadPoolActionTask(TkFunc<void()>::Signature action, void *target = nullptr)
            : ThreadPoolTask(), m_action1(action), m_target(target) {}
        ThreadPoolActionTask(const TkFunc<bool()> &action, void *target = nullptr)
            : ThreadPoolTask(), m_action2(action), m_target(target) {}
        ThreadPoolActionTask(TkFunc<bool()>::Signature action, void *target = nullptr)
            : ThreadPoolTask(), m_action2(action), m_target(target) {}

    protected:
        bool run() override
        {
            if (m_action1.isBinded())
            {
                m_action1();
                return false;
            }

            if (m_action2.isBinded())
            {
                return m_action2();
            }

            return false;
        }
    };

    // General purpose task executed on Main Thread in the beginning of the next frame.
    class MainThreadTask : public TkTask
    {
    public:
        float initialDelay = 0.0f;

        static void runAll(float dt);

    protected:
        void enqueue() override;
    };

    // General purpose task executing custom action using Main Thread in the beginning of the next frame.
    class MainThreadActionTask : public MainThreadTask
    {
    protected:
        TkFunc<void()> m_action;
        void *m_target;

    public:
        MainThreadActionTask(const TkFunc<void()> &action, void *target = nullptr)
            : MainThreadTask(), m_action(action), m_target(target) {}

    protected:
        bool run() override;
    };

    // General purpose task executing custom action using Main Thread in the beginning of the next frame.
    class MainThreadBoolActionTask  : public MainThreadTask
    {
    protected:
        TkFunc<bool()> m_action;
        void *m_target;

    public:
        MainThreadBoolActionTask(const TkFunc<bool()> &action, void *target = nullptr)
            : MainThreadTask(), m_action(action), m_target(target) {}

    protected:
        bool run() override;
    };

    // Invokes a target method on a main thread (using task or directly if already on main thread)
#define INVOKE_ON_MAIN_THREAD(tp, mtd, obj)                                                                  \
    if (TkThread::isInMainThread())                                                                          \
    {                                                                                                        \
        obj->mtd();                                                                                          \
    }                                                                                                        \
    else                                                                                                     \
    {                                                                                                        \
        TkFunc<void()> action;                                                                               \
        action.bind<tp, &mtd>(obj);                                                                          \
        TkTask::startNew(TkMemory::newObject<MainThreadActionTask>(TkMemory::getDefault(), action))->wait(); \
    }
}