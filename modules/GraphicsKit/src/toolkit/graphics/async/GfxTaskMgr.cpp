/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxTaskMgr.hpp"
#include "GfxTask.hpp"
#include "GfxTaskExecutor.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{
    GfxTaskManager::GfxTaskManager()
    {
        m_buffers[0].ensureCapacity(64);
        m_buffers[1].ensureCapacity(64);
    }

    void GfxTaskManager::setExecutor(GfxTaskExecutor *value)
    {
        if (value != m_executor && value)
        {
            if (m_executor)
            {
                TK_SAFE_DELETE(m_executor);
            }

            m_executor = value;
        }
    }

    void GfxTaskManager::dispose()
    {
        // Cancel all performed tasks (that are waiting for sync)
        TK_SAFE_DELETE(m_executor);

        // Cleanup
        TkTask::cancelAll(m_buffers[0]);
        TkTask::cancelAll(m_buffers[1]);
        m_bufferIndex = 0;
        m_tasks.cancelAll();
    }

    void GfxTaskManager::frameBegin()
    {
        m_executor->frameBegin();
    }

    void GfxTaskManager::frameEnd()
    {
        m_executor->frameEnd();
    }

    i32 GfxTaskManager::requestWork(GfxTask **buffer, i32 maxCount)
    {
        const auto b1Index = m_bufferIndex;
        const auto b2Index = (m_bufferIndex + 1) % 2;
        auto &b1 = m_buffers[b1Index];
        auto &b2 = m_buffers[b2Index];

        // Take maximum amount of tasks to the buffer at once
        TK_ASSERT(b1.isEmpty());
        const i32 takenTasksCount = (i32)m_tasks.try_dequeue_bulk(b1.get(), maxCount);
        b2.add(b1.get(), takenTasksCount);

        i32 count = 0;

        // Filter taken tasks to keep maxTotalComplexity limit
        b1.clear();
        i32 i = 0;
        for (; i < b2.count() && count < maxCount; i++)
        {
            auto task = b2[i];
            const auto state = task->getState();
            switch (state)
            {
            case TaskState::kFailed:
            case TaskState::kCanceled:
            case TaskState::kFinished:
                // Skip task
                break;
            case TaskState::kQueued:
                // Run queued task
                buffer[count++] = task;
                break;
            case TaskState::kCreated:
            case TaskState::kRunning:
            default:
                // Keep task for the next RequestWork
                b1.add(task);
                break;
            }
        }
        const i32 itemsLeft = b2.count() - i;
        if (itemsLeft > 0)
            b1.add(&b2[i], itemsLeft);
        b2.clear();

        // Swap buffers
        m_bufferIndex = b2Index;

        return count;
    }

    TkString GfxTaskManager::toString() const
    {
        return "GPU Tasks Manager";
    }

}