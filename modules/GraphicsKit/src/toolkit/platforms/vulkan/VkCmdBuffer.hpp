/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include <toolkit/collection/Array.hpp>

namespace toolkit
{

    // Implementation of the command buffer for the Vulkan backend.
    class VulkanCmdBuffer
    {
        friend class VulkanQueue;

    public:
        enum class State
        {
            kReadyForBegin,
            kIsInsideBegin,
            kIsInsideRenderPass,
            kHasEnded,
            kSubmitted,
        };

    private:
        class VulkanDevice *m_device;
        VkCommandBuffer m_commandBuffer;
        State m_state;

        TkArray<VkPipelineStageFlags> m_waitFlags;
        TkArray<class VulkanSemaphore *> m_waitSemaphores;
        TkArray<VulkanSemaphore *> m_submittedWaitSemaphores;

        class VulkanFence *m_fence;
#if GFX_ALLOW_PROFILE_EVENTS
        i32 m_eventsBegin = 0;
#endif

        // The latest value when command buffer was submitted.
        volatile u64 m_submittedFenceCounter;
        // The latest value passed after the fence was signaled.
        volatile u64 m_fenceSignaledCounter;

        class VulkanCmdBufferPool *m_commandBufferPool;
        class VulkanDescriptorPoolSetContainer *m_descriptorPoolSetContainer = nullptr;

    public:
        VulkanCmdBuffer(VulkanDevice *device, VulkanCmdBufferPool *pool);
        ~VulkanCmdBuffer();

    public:
        VulkanCmdBufferPool *getOwner() const
        {
            return m_commandBufferPool;
        }

        State getState() const
        {
            return m_state;
        }

        VulkanFence *getFence() const
        {
            return m_fence;
        }

        inline bool isInsideRenderPass() const
        {
            return m_state == State::kIsInsideRenderPass;
        }

        inline bool isOutsideRenderPass() const
        {
            return m_state == State::kIsInsideBegin;
        }

        inline bool hasBegun() const
        {
            return m_state == State::kIsInsideBegin || m_state == State::kIsInsideRenderPass;
        }

        inline bool hasEnded() const
        {
            return m_state == State::kHasEnded;
        }

        inline bool isSubmitted() const
        {
            return m_state == State::kSubmitted;
        }

        inline VkCommandBuffer getHandle() const
        {
            return m_commandBuffer;
        }

        inline volatile u64 getFenceSignaledCounter() const
        {
            return m_fenceSignaledCounter;
        }

        inline volatile u64 getSubmittedFenceCounter() const
        {
            return m_submittedFenceCounter;
        }

    public:
        void addWaitSemaphore(VkPipelineStageFlags waitFlags, VulkanSemaphore *waitSemaphore);

        void begin();
        void end();

        void beginRenderPass(class VulkanRenderPass *renderPass, class VulkanFramebuffer *framebuffer, u32 clearValueCount, VkClearValue *clearValues);
        void endRenderPass();

        TK_FORCE_INLINE VulkanDescriptorPoolSetContainer *getDescriptorPoolSet() const
        {
            return m_descriptorPoolSetContainer;
        }

#if GFX_ALLOW_PROFILE_EVENTS
        void beginEvent(const char *name);
        void endEvent();
#endif

        void refreshFenceStatus();
    };

    class VulkanCmdBufferPool
    {
        friend class VulkanCmdBufferManager;

    private:
        VulkanDevice *m_device;
        VkCommandPool m_handle;
        TkArray<VulkanCmdBuffer *> m_cmdBuffers;

        VulkanCmdBuffer *create();
        void create(u32 queueFamilyIndex);

    public:
        VulkanCmdBufferPool(VulkanDevice *device);
        ~VulkanCmdBufferPool();

    public:
        inline VkCommandPool getHandle() const
        {
            return m_handle;
        }

        void refreshFenceStatus(const VulkanCmdBuffer *skipCmdBuffer = nullptr);
    };

    class VulkanCmdBufferManager
    {
    private:
        VulkanDevice *m_device;
        VulkanCmdBufferPool m_pool;
        VulkanQueue *m_queue;
        VulkanCmdBuffer *m_activeCmdBuffer;
        TkArray<class VulkanTimerQuery *> m_queriesInProgress;

    public:
        VulkanCmdBufferManager(VulkanDevice *device, class VulkanContext *context);

    public:
        TK_FORCE_INLINE VkCommandPool getHandle() const
        {
            return m_pool.getHandle();
        }

        TK_FORCE_INLINE VulkanCmdBuffer *getActiveCmdBuffer() const
        {
            return m_activeCmdBuffer;
        }

        TK_FORCE_INLINE bool hasPendingActiveCmdBuffer() const
        {
            return m_activeCmdBuffer != nullptr;
        }

        TK_FORCE_INLINE bool hasQueriesInProgress() const
        {
            return m_queriesInProgress.count() != 0;
        }

        TK_FORCE_INLINE VulkanCmdBuffer *getCmdBuffer()
        {
            if (!m_activeCmdBuffer)
                prepareForNewActiveCommandBuffer();
            return m_activeCmdBuffer;
        }

    public:
        void submitActiveCmdBuffer(VulkanSemaphore *signalSemaphore = nullptr);
        void waitForCmdBuffer(VulkanCmdBuffer *cmdBuffer, float timeInSecondsToWait = 1.0f);
        void refreshFenceStatus(VulkanCmdBuffer *skipCmdBuffer = nullptr)
        {
            m_pool.refreshFenceStatus(skipCmdBuffer);
        }
        void prepareForNewActiveCommandBuffer();

        void onQueryBegin(VulkanTimerQuery *query);
        void onQueryEnd(VulkanTimerQuery *query);
    };

}

#endif