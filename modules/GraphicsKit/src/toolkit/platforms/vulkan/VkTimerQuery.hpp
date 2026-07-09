/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "VkDevice.hpp"
#include "toolkit/graphics/GfxTimerQuery.hpp"

namespace toolkit
{
    // GPU timer query object for Vulkan backend.
    class VulkanTimerQuery : public VulkanResource<GfxTimerQuery>
    {
    private:
        struct Query
        {
            VulkanBufferedQueryPool *pool = nullptr;
            u32 index = 0;
            u64 result = 0;
        };

        struct QueryPair
        {
            Query begin;
            Query end;
        };

        bool m_hasResult = false;
        bool m_endCalled = false;
        bool m_interrupted = false;
        f32 m_timeDelta = 0.0f;
        i32 m_queryIndex;
        TkStaticArray<QueryPair, 8> m_queries;

    public:
        VulkanTimerQuery(VulkanDevice *device);

    public:
        // Interrupts an in-progress query, allowing the command buffer to submitted. Interrupted queries must be resumed using Resume().
        void interrupt(VulkanCmdBuffer *cmdBuffer);
        // Resumes an interrupted query, restoring it back to its original in-progress state.
        void resume(VulkanCmdBuffer *cmdBuffer);

    private:
        // Returns true when this timestamp is resolved (or was never allocated);
        // returns false when the GPU result is still pending.
        bool getResult(Query &query);
        void writeTimestamp(VulkanCmdBuffer *cmdBuffer, Query &query, VkPipelineStageFlagBits stage) const;
        // Returns true only after all query timestamps are resolved and m_timeDelta is valid.
        bool tryGetResult();
        bool useQueries();

    public:
        void begin() override;
        void end() override;
        bool hasResult() override;
        f32 getResult() override;

    protected:
        void onReleaseData() override;
    };

}

#endif
