/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkTimerQuery.hpp"
#include "VkContext.hpp"

#if GFX_API_VULKAN

#include "VkCmdBuffer.hpp"

namespace toolkit
{

    VulkanTimerQuery::VulkanTimerQuery(VulkanDevice *device)
        : VulkanResource<GfxTimerQuery>(device, TkString::Empty)
    {
    }

    void VulkanTimerQuery::interrupt(VulkanCmdBuffer *cmdBuffer)
    {
        if (!m_interrupted)
        {
            m_interrupted = true;
            writeTimestamp(cmdBuffer, m_queries[m_queryIndex].end, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
    }

    void VulkanTimerQuery::resume(VulkanCmdBuffer *cmdBuffer)
    {
        TK_ASSERT(m_interrupted);

        QueryPair e;
        e.end.pool = nullptr;

        m_interrupted = false;
        writeTimestamp(cmdBuffer, e.begin, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

        m_queries.add(e);
        m_queryIndex++;
    }

    bool VulkanTimerQuery::getResult(Query &query)
    {
        if (query.pool)
        {
            const auto context = (VulkanContext *)m_device->getMainContext();
            if (query.pool->getResults(context, query.index, query.result))
            {
                // Success: the timestamp is available, so the pool slot can be reused.
                query.pool->releaseQuery(query.index);
                query.pool = nullptr;
            }
            else
            {
                // Failure/pending: keep the pool slot until a later poll can read it.
                return false;
            }
        }

        // Success: either the result was read above or no timestamp query was allocated.
        return true;
    }

    void VulkanTimerQuery::writeTimestamp(VulkanCmdBuffer *cmdBuffer, Query &query, VkPipelineStageFlagBits stage) const
    {
        auto pool = m_device->findAvailableTimestampQueryPool();
        u32 index;
        if (pool->acquireQuery(cmdBuffer, index))
        {
            vkCmdWriteTimestamp(cmdBuffer->getHandle(), stage, pool->getHandle(), index);
            pool->markQueryAsStarted(index);
            query.pool = pool;
            query.index = index;
            query.result = 0;
        }
        else
        {
            query.pool = nullptr;
            query.index = 0;
            query.result = 0;
        }
    }

    bool VulkanTimerQuery::tryGetResult()
    {
#if VK_USE_QUERIES
        // Success means every begin/end timestamp has been resolved. If any timestamp
        // is still pending, return false and leave the query data intact for retry.
        for (i32 i = 0; i < m_queries.count(); i++)
        {
            auto &e = m_queries[i];

            if (!getResult(e.begin))
                return false;
            if (!getResult(e.end))
                return false;
        }

        // Calculate delta time (accumulated)
        u64 delta = 0;
        for (i32 i = 0; i < m_queries.count(); i++)
        {
            auto &e = m_queries[i];

            if (e.end.result > e.begin.result)
            {
                delta += e.end.result - e.begin.result;
            }
        }

        // Calculate event duration in milliseconds
        const double frequency = double(m_device->physicalDeviceLimits.timestampPeriod) / 1e6;
        m_timeDelta = static_cast<float>((delta * frequency));

        // Clear data for next usage
        m_hasResult = true;
        for (i32 i = 0; i < m_queries.count(); i++)
        {
            auto &e = m_queries[i];
            if (e.begin.pool)
                e.begin.pool->releaseQuery(e.begin.index);
            if (e.end.pool)
                e.end.pool->releaseQuery(e.end.index);
        }
        m_queries.clear();
#else
        m_timeDelta = 0.0f;
        m_hasResult = true;
#endif
        return true;
    }

    bool VulkanTimerQuery::useQueries()
    {
#if VK_USE_QUERIES
        return m_device->physicalDeviceLimits.timestampComputeAndGraphics == VK_TRUE;
#else
        return false;
#endif
    }

    void VulkanTimerQuery::onReleaseData()
    {
        m_hasResult = false;
        m_endCalled = false;
        m_timeDelta = 0.0f;

        for (i32 i = 0; i < m_queries.count(); i++)
        {
            auto &e = m_queries[i];
            if (e.begin.pool)
                e.begin.pool->releaseQuery(e.begin.index);
            if (e.end.pool)
                e.end.pool->releaseQuery(e.end.index);
        }
        m_queries.clear();
    }

    void VulkanTimerQuery::begin()
    {
#if VK_USE_QUERIES
        if (useQueries())
        {
            const auto context = (VulkanContext *)m_device->getMainContext();
            const auto cmdBuffer = context->getCmdBufferManager()->getCmdBuffer();

            QueryPair e;
            e.end.pool = nullptr;

            m_queryIndex = 0;
            m_interrupted = false;
            writeTimestamp(cmdBuffer, e.begin, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
            context->getCmdBufferManager()->onQueryBegin(this);

            TK_ASSERT(m_queries.isEmpty());
            m_queries.add(e);
        }
#endif

        m_hasResult = false;
        m_endCalled = false;
    }

    void VulkanTimerQuery::end()
    {
        if (m_endCalled)
            return;

#if VK_USE_QUERIES
        if (useQueries())
        {
            const auto context = (VulkanContext *)m_device->getMainContext();
            const auto cmdBuffer = context->getCmdBufferManager()->getCmdBuffer();

            if (!m_interrupted)
            {
                writeTimestamp(cmdBuffer, m_queries[m_queryIndex].end, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
            }
            context->getCmdBufferManager()->onQueryEnd(this);
        }
#endif

        m_endCalled = true;
    }

    bool VulkanTimerQuery::hasResult()
    {
        if (!m_endCalled)
            return false;
        if (m_hasResult)
            return true;
        return tryGetResult();
    }

    float VulkanTimerQuery::getResult()
    {
        if (m_hasResult)
            return m_timeDelta;
        tryGetResult();
        return m_timeDelta;
    }

}

#endif
