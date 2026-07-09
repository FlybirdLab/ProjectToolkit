/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/async/GfxTaskContext.hpp"
#include "toolkit/threading/Task.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    // Describe GPU work object.
    class GfxTask : public TkTask
    {
        friend class GfxTaskContext;

    public:
        // Describe GPU work type
        TK_DECLARE_ENUM_EX_4(Type, byte, 0, Custom, CopyResource, UploadTexture, UploadBuffer);
        // Describe GPU work result value
        TK_DECLARE_ENUM_4(Result, Ok, Failed, MissingResources, MissingData);

    protected:
        GfxTask(const Type type, byte syncLatency = kGfxAsyncLatency)
            : m_type(type), m_syncLatency(syncLatency), m_syncPoint(0), m_context(nullptr)
        {
        }

    public:
        // Gets a task type.
        TK_FORCE_INLINE Type getType() const
        {
            return m_type;
        }

        // Gets work synchronization start point
        TK_FORCE_INLINE GfxSyncPoint getSyncStart() const
        {
            return m_syncPoint;
        }

        // Gets work finish synchronization point
        TK_FORCE_INLINE GfxSyncPoint getSyncPoint() const
        {
            return m_syncPoint + m_syncLatency;
        }

    public:
        // Checks if operation is syncing
        TK_FORCE_INLINE bool isSyncing() const
        {
            return isRunning() && m_syncPoint != 0;
        }

    public:
        // Executes this task.
        void execute(GfxTaskContext *context);

        // Action fired when asynchronous operation has been synchronized with a GPU
        void sync()
        {
            if (m_context != nullptr)
            {
                TK_ASSERT(isSyncing());

                // Sync and finish
                m_context = nullptr;
                onSync();
                onFinish();
            }
        }

        // Cancels the task results synchronization.
        void cancelSync()
        {
            TK_ASSERT(isSyncing());

            // Rollback state and cancel
            m_context = nullptr;
            setState(TaskState::kQueued);
            cancel();
        }
        TkString toString() const;

    protected:
        virtual Result run(GfxTaskContext *context) = 0;

        void enqueue() override;
        void onCancel() override;

        virtual void onSync()
        {
        }

        bool run() override
        {
            return true;
        }

    private:
        // Task type
        Type m_type;
        byte m_syncLatency;
        // Synchronization point when async task has been done
        GfxSyncPoint m_syncPoint;
        // The context that performed this task, it should synchronize it.
        GfxTaskContext *m_context;
    };

}