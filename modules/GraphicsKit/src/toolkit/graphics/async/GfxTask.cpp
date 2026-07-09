/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxTask.hpp"
#include "GfxTaskContext.hpp"
#include "GfxTaskMgr.hpp"
#include "toolkit/graphics/GfxDevice.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    void GfxTask::execute(GfxTaskContext *context)
    {
        TK_ASSERT(isQueued() && m_context == nullptr);
        setState(TaskState::kRunning);

        // Perform an operation
        const auto result = run(context);

        // Process result
        if (isCancelRequested())
        {
            setState(TaskState::kCanceled);
        }
        else if (result != Result::kOk)
        {
            LogWarn("\'%s\' failed with result: %s", toString(), toString(result));
            onFail();
        }
        else
        {
            // Save task completion point (for synchronization)
            m_syncPoint = context->getCurrentSyncPoint();
            m_context = context;
            if (m_syncLatency == 0)
            {
                // No delay on sync
                sync();
            }
        }
    }

    TkString GfxTask::toString() const
    {
        return TkString::format("GPU Async Task %s (%i)", toString(getType()), (i32)getState());
    }

    void GfxTask::enqueue()
    {
        GfxDevice::sInstance->getTaskManager()->m_tasks.add(this);
    }

    void GfxTask::onCancel()
    {
        // Check if task is waiting for sync (very likely situation)
        if (isSyncing())
        {
            // Task has been performed but is waiting for a CPU/GPU sync so we have to cancel that
            m_context->onCancelSync(this);
            m_context = nullptr;
            setState(TaskState::kCanceled);
        }

        // Base
        TkTask::onCancel();
    }

}