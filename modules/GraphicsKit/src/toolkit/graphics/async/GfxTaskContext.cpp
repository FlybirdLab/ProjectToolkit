/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxTaskContext.hpp"

#include "GfxTask.hpp"
#include "toolkit/graphics/GfxDevice.hpp"
#include "toolkit/threading/Thread.hpp"

namespace toolkit
{

#define GPU_TASKS_USE_DEDICATED_CONTEXT 0

    GfxTaskContext::GfxTaskContext(GfxDevice *device)
    {
        // Bump it up to prevent initial state problems with frame index comparison
        m_currentSyncPoint = 10;

#if GPU_TASKS_USE_DEDICATED_CONTEXT
        // Create graphics context
        context = device->createContext(true);
#else
        // Reuse Graphics Device context
        context = device->getMainContext();
#endif
    }

    GfxTaskContext::~GfxTaskContext()
    {
        TK_ASSERT(TkThread::isInMainThread());

        // Cancel jobs to sync
        auto tasks = m_tasksSyncing;
        m_tasksSyncing.clear();
        for (i32 i = 0; i < tasks.count(); i++)
        {
            auto task = tasks[i];
            if (task->getSyncPoint() <= m_currentSyncPoint && task->getState() != TaskState::kFinished)
            {
                task->cancelSync();
            }
        }

#if GPU_TASKS_USE_DEDICATED_CONTEXT
        GFX_DEL_RESOURCE(context);
#endif
    }

    void GfxTaskContext::run(GfxTask *task)
    {
        TK_ASSERT(task != nullptr);

        task->execute(this);
        // if (task->GetSyncStart() != 0)
        m_tasksSyncing.add(task);
    }

    void GfxTaskContext::onCancelSync(GfxTask *task)
    {
        m_tasksSyncing.remove(task);
    }

    void GfxTaskContext::onFrameBegin()
    {
#if GPU_TASKS_USE_DEDICATED_CONTEXT
        context->frameBegin();
#endif

        // Move forward one frame
        ++m_currentSyncPoint;

        // Try to flush done jobs
        for (i32 i = 0; i < m_tasksSyncing.count(); i++)
        {
            auto task = m_tasksSyncing[i];
            auto state = task->getState();
            if (task->getSyncPoint() <= m_currentSyncPoint && state != TaskState::kFinished)
            {
                // TODO: add stats counter and count performed jobs, print to log on exit.
                task->sync();
            }
            if (state == TaskState::kFailed || state == TaskState::kCanceled)
            {
                m_tasksSyncing.removeAt(i);
                i--;
            }
            if (state == TaskState::kFinished)
            {
                m_tasksSyncing.removeAt(i);
                i--;
                m_totalTasksDoneCount++;
            }
        }
    }

    void GfxTaskContext::onFrameEnd()
    {
#if GPU_TASKS_USE_DEDICATED_CONTEXT
        context->frameEnd();
#endif
    }

}
