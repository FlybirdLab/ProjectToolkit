/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxTaskExecutor.hpp"
#include "GfxTaskContext.hpp"
#include "GfxTaskMgr.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/graphics/GfxDevice.hpp"

namespace toolkit
{
    GfxTaskExecutor::~GfxTaskExecutor()
    {
        // Stats
        i32 totalJobsDone = 0;
        for (i32 i = 0; i < m_contextList.count(); i++)
            totalJobsDone += m_contextList[i]->getTotalTasksDoneCount();
        LogInfo("Total GPU tasks done: %i", totalJobsDone);

        m_contextList.clearDelete();
    }

    GfxTaskContext *GfxTaskExecutor::createContext()
    {
        auto context = GfxDevice::sInstance->createTasksContext();
        if (context == nullptr)
        {
            LogError("Cannot create new GPU Tasks Context");
        }
        m_contextList.add(context);
        return context;
    }

    DefaultGfxTasksExecutor::DefaultGfxTasksExecutor()
        : m_context(nullptr)
    {
    }

    TkString DefaultGfxTasksExecutor::toString() const
    {
        return "Default GPU Async Executor";
    }

    void DefaultGfxTasksExecutor::frameBegin()
    {
        PROFILE();

        // Ensure to have valid async context
        if (m_context == nullptr)
            m_context = createContext();

        m_context->onFrameBegin();

        // Default implementation performs async operations on start of the frame which is synchronized with a rendering thread
        GfxTask *buffer[32];
        const i32 count = GfxDevice::sInstance->getTaskManager()->requestWork(buffer, 32);
        for (i32 i = 0; i < count; i++)
        {
            m_context->run(buffer[i]);
        }
    }

    void DefaultGfxTasksExecutor::frameEnd()
    {
        PROFILE();
        TK_ASSERT(m_context != nullptr);
        m_context->onFrameEnd();
    }

}