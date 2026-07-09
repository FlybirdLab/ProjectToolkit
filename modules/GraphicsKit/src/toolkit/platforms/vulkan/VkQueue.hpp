/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/threading/Sync.hpp"
#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

namespace toolkit
{
    // Implementation of the command buffer queue for the Vulkan backend.
    class VulkanQueue
    {
    private:
        VkQueue m_queue;
        u32 m_familyIndex;
        u32 m_queueIndex;
        class VulkanDevice *m_device;
        class VulkanCmdBuffer *m_lastSubmittedCmdBuffer;
        mutable TkMutex m_mtx;
        u64 m_lastSubmittedCmdBufferFenceCounter;

    public:
        VulkanQueue(VulkanDevice *device, u32 familyIndex);

        inline u32 getFamilyIndex() const
        {
            return m_familyIndex;
        }

        void submit(VulkanCmdBuffer *cmdBuffer, u32 signalSemaphoresCount = 0, const VkSemaphore *signalSemaphores = nullptr);
        inline void submit(VulkanCmdBuffer *cmdBuffer, VkSemaphore signalSemaphore)
        {
            submit(cmdBuffer, 1, &signalSemaphore);
        }

        inline VkQueue getHandle() const
        {
            return m_queue;
        }
        void getLastSubmittedInfo(VulkanCmdBuffer *&cmdBuffer, u64 &fenceCounter) const;

    private:
        void updateLastSubmittedCommandBuffer(VulkanCmdBuffer *cmdBuffer);
    };

}

#endif
