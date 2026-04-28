/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit
{
    class GfxTaskContext;

    // Describe object responsible for GPU jobs execution scheduling.
    class GfxTaskExecutor
    {
    public:
        virtual ~GfxTaskExecutor();

    public:
        // Sync point event called on begin of the frame
        virtual void frameBegin() = 0;
        // Sync point event called on end of the frame
        virtual void frameEnd() = 0;

        // Gets the context list.
        TK_FORCE_INLINE const TkArray<GfxTaskContext *> *getContextList() const
        {
            return &m_contextList;
        }

    protected:
        GfxTaskContext *createContext();

    protected:
        TkArray<GfxTaskContext *> m_contextList;
    };

    // Default implementation for GPU async job execution
    class DefaultGfxTasksExecutor : public GfxTaskExecutor
    {
    public:
        DefaultGfxTasksExecutor();

    public:
        TkString toString() const;
        void frameBegin() override;
        void frameEnd() override;

    protected:
        GfxTaskContext *m_context;
    };

}