/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/threading/Sync.hpp"
#include "toolkit/collection/Array.hpp"

#include <toolkit/graphics/GfxContext.hpp>
#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    class GfxTask;

    // GPU tasks contex
    class GfxTaskContext
    {
    public:
        GfxTaskContext(GfxDevice *device);
        ~GfxTaskContext();

    public:
        // The GPU commands context used for tasks execution (can be only copy/upload without graphics capabilities on some platforms).
        GfxContext *context;

    public:
        // Get graphics device handle
        TK_FORCE_INLINE GfxDevice *getDevice() const
        {
            return context->getDevice();
        }

        // Get current synchronization point of that context (CPU position, GPU has some latency)
        TK_FORCE_INLINE GfxSyncPoint getCurrentSyncPoint() const
        {
            return m_currentSyncPoint;
        }

        // Get total amount of tasks done by this context
        TK_FORCE_INLINE i32 getTotalTasksDoneCount() const
        {
            return m_totalTasksDoneCount;
        }

        // Perform given task
        void run(GfxTask *task);

        void onCancelSync(GfxTask *task);

    public:
        void onFrameBegin();
        void onFrameEnd();

    protected:
        TkMutex m_mtx;
        GfxSyncPoint m_currentSyncPoint;
        i32 m_totalTasksDoneCount = 0;
        TkArray<class GfxTask *> m_tasksSyncing;
    };

}