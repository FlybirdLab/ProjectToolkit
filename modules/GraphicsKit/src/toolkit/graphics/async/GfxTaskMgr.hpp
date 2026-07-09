/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/async/GfxTask.hpp"
#include <toolkit/graphics/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/threading/ConcurrentTaskQueue.hpp>

namespace toolkit
{
    // Graphics Device work manager.
    class GfxTaskManager : public NonCopyable
    {
        friend class GfxDevice;
        friend class GfxTask;

    private:
        class GfxTaskExecutor *m_executor = nullptr;
        ConcurrentTaskQueue<GfxTask> m_tasks;
        TkArray<GfxTask *> m_buffers[2];
        i32 m_bufferIndex = 0;

    public:
        GfxTaskManager();

        // Get the GPU tasks executor.
        TK_FORCE_INLINE class GfxTaskExecutor *getExecutor() const
        {
            return m_executor;
        }

        // Set the GPU tasks executor.
        void setExecutor(class GfxTaskExecutor *value);

        // Get the amount of enqueued tasks to perform.
        TK_FORCE_INLINE i32 getTaskCount() const
        {
            return m_tasks.count();
        }

    public:
        // Clears asynchronous resources loading queue and cancels all tasks.
        void dispose();

        // On begin rendering frame.
        void frameBegin();
        // On end rendering frame.
        void frameEnd();

        // Request work to do. Should be used only by class GfxTaskExecutor.
        i32 requestWork(GfxTask **buffer, i32 maxCount);

        TkString toString() const;
    };

}