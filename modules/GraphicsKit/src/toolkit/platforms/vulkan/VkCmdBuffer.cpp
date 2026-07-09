/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkCmdBuffer.hpp"
#include "VkTimerQuery.hpp"

#if GFX_API_VULKAN

#include "VkUtils.hpp"
#include "VkContext.hpp"
#include "VkQueue.hpp"
#include "VkDescriptorSet.hpp"
#include "VkDevice.hpp"
#include "toolkit/debug/Profiler.hpp"

namespace toolkit
{
    void VulkanCmdBuffer::addWaitSemaphore(VkPipelineStageFlags waitFlags, VulkanSemaphore *waitSemaphore)
    {
        m_waitFlags.add(waitFlags);
        TK_ASSERT(!m_waitSemaphores.contains(waitSemaphore));
        m_waitSemaphores.add(waitSemaphore);
    }

    void VulkanCmdBuffer::begin()
    {
        PROFILE();
        TK_ASSERT(m_state == State::kReadyForBegin);

        VkCommandBufferBeginInfo beginInfo;
        VkUtils::zeroStruct(beginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VALIDATE_VK_RESULT(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));

        // Acquire a descriptor pool set on
        if (m_descriptorPoolSetContainer == nullptr)
        {
            m_descriptorPoolSetContainer = m_device->descriptorPoolsManager->acquirePoolSetContainer();
        }

        m_state = State::kIsInsideBegin;

#if GFX_ALLOW_PROFILE_EVENTS
        // Reset events counter
        m_eventsBegin = 0;
#endif
    }

    void VulkanCmdBuffer::end()
    {
        PROFILE();
        TK_ASSERT(isOutsideRenderPass());

#if GFX_ALLOW_PROFILE_EVENTS && VK_EXT_debug_utils
        // End remaining events
        while (m_eventsBegin--)
            vkCmdEndDebugUtilsLabelEXT(getHandle());
#endif

        VALIDATE_VK_RESULT(vkEndCommandBuffer(getHandle()));
        m_state = State::kHasEnded;
    }

    void VulkanCmdBuffer::beginRenderPass(VulkanRenderPass *renderPass, VulkanFramebuffer *framebuffer, u32 clearValueCount, VkClearValue *clearValues)
    {
        TK_ASSERT(isOutsideRenderPass());
        VkRenderPassBeginInfo info;
        VkUtils::zeroStruct(info, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        info.renderPass = renderPass->handle;
        info.framebuffer = framebuffer->handle;
        info.renderArea.offset.x = 0;
        info.renderArea.offset.y = 0;
        info.renderArea.extent.width = framebuffer->extent.width;
        info.renderArea.extent.height = framebuffer->extent.height;
        info.clearValueCount = clearValueCount;
        info.pClearValues = clearValues;
        vkCmdBeginRenderPass(m_commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        m_state = State::kIsInsideRenderPass;
    }

    void VulkanCmdBuffer::endRenderPass()
    {
        TK_ASSERT(isInsideRenderPass());
        vkCmdEndRenderPass(m_commandBuffer);
        m_state = State::kIsInsideBegin;
    }

#if GFX_ALLOW_PROFILE_EVENTS

    void VulkanCmdBuffer::beginEvent(const char *name)
    {
#if VK_EXT_debug_utils
        if (!vkCmdBeginDebugUtilsLabelEXT)
            return;

        m_eventsBegin++;

        // Convert to ANSI
        char buffer[101];
        i32 i = 0;
        while (i < 100 && name[i])
        {
            buffer[i] = (char)name[i];
            i++;
        }
        buffer[i] = 0;

        VkDebugUtilsLabelEXT label;
        VkUtils::zeroStruct(label, VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT);
        label.pLabelName = buffer;
        vkCmdBeginDebugUtilsLabelEXT(getHandle(), &label);
#endif
    }

    void VulkanCmdBuffer::endEvent()
    {
#if VK_EXT_debug_utils
        if (m_eventsBegin == 0 || !vkCmdEndDebugUtilsLabelEXT)
            return;
        m_eventsBegin--;

        vkCmdEndDebugUtilsLabelEXT(getHandle());
#endif
    }

#endif

    void VulkanCmdBuffer::refreshFenceStatus()
    {
        if (m_state == State::kSubmitted)
        {
            PROFILE();
            if (m_device->fenceManager.isFenceSignaled(m_fence))
            {
                m_state = State::kReadyForBegin;
                m_submittedWaitSemaphores.clear();

                vkResetCommandBuffer(m_commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
                m_device->fenceManager.resetFence(m_fence);
                m_fenceSignaledCounter++;

                if (m_descriptorPoolSetContainer)
                {
                    // Keep descriptor pool lifetime tied to GraphicsKit's own frame counter.
                    // The Vulkan backend must not depend on the higher-level Engine module.
                    m_descriptorPoolSetContainer->lastFrameUsed = m_device->getFrameIndex();
                    m_descriptorPoolSetContainer = nullptr;
                }
            }
        }
        else
        {
            TK_ASSERT(!m_fence->isSignaled);
        }
    }

    VulkanCmdBuffer::VulkanCmdBuffer(VulkanDevice *device, VulkanCmdBufferPool *pool)
        : m_device(device), m_commandBuffer(VK_NULL_HANDLE), m_state(State::kReadyForBegin), m_fence(nullptr), m_submittedFenceCounter(0), m_fenceSignaledCounter(0), m_commandBufferPool(pool)
    {
        VkCommandBufferAllocateInfo createCmdBufInfo;
        VkUtils::zeroStruct(createCmdBufInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
        createCmdBufInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        createCmdBufInfo.commandBufferCount = 1;
        createCmdBufInfo.commandPool = m_commandBufferPool->getHandle();
        VALIDATE_VK_RESULT(vkAllocateCommandBuffers(m_device->device, &createCmdBufInfo, &m_commandBuffer));
        m_fence = m_device->fenceManager.allocateFence();
    }

    VulkanCmdBuffer::~VulkanCmdBuffer()
    {
        auto &fenceManager = m_device->fenceManager;
        if (m_state == State::kSubmitted)
        {
            // Wait 60ms
            const u64 waitForCmdBufferInNanoSeconds = 60 * 1000 * 1000LL;
            fenceManager.waitAndReleaseFence(m_fence, waitForCmdBufferInNanoSeconds);
        }
        else
        {
            // Just free the fence, command buffer was not submitted
            fenceManager.releaseFence(m_fence);
        }

        vkFreeCommandBuffers(m_device->device, m_commandBufferPool->getHandle(), 1, &m_commandBuffer);
    }

    VulkanCmdBuffer *VulkanCmdBufferPool::create()
    {
        const auto cmdBuffer = TkMemory::newObject<VulkanCmdBuffer>(TkMemory::getDefault(), m_device, this);
        m_cmdBuffers.add(cmdBuffer);
        return cmdBuffer;
    }

    void VulkanCmdBufferPool::create(u32 queueFamilyIndex)
    {
        VkCommandPoolCreateInfo poolInfo;
        VkUtils::zeroStruct(poolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
        poolInfo.queueFamilyIndex = queueFamilyIndex;
        // TODO: use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT?
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VALIDATE_VK_RESULT(vkCreateCommandPool(m_device->device, &poolInfo, nullptr, &m_handle));
    }

    VulkanCmdBufferPool::VulkanCmdBufferPool(VulkanDevice *device)
        : m_device(device), m_handle(VK_NULL_HANDLE)
    {
    }

    VulkanCmdBufferPool::~VulkanCmdBufferPool()
    {
        for (i32 i = 0; i < m_cmdBuffers.count(); i++)
            TkMemory::deletePtr(m_cmdBuffers[i]);
        vkDestroyCommandPool(m_device->device, m_handle, nullptr);
    }

    void VulkanCmdBufferPool::refreshFenceStatus(const VulkanCmdBuffer *skipCmdBuffer)
    {
        for (i32 i = 0; i < m_cmdBuffers.count(); i++)
        {
            auto cmdBuffer = m_cmdBuffers[i];
            if (cmdBuffer != skipCmdBuffer)
            {
                cmdBuffer->refreshFenceStatus();
            }
        }
    }

    VulkanCmdBufferManager::VulkanCmdBufferManager(VulkanDevice *device, VulkanContext *context)
        : m_device(device), m_pool(device), m_queue(context->getQueue()), m_activeCmdBuffer(nullptr)
    {
        m_pool.create(m_queue->getFamilyIndex());
    }

    void VulkanCmdBufferManager::submitActiveCmdBuffer(VulkanSemaphore *signalSemaphore)
    {
        PROFILE();
        TK_ASSERT(m_activeCmdBuffer);
        if (!m_activeCmdBuffer->isSubmitted() && m_activeCmdBuffer->hasBegun())
        {
            if (m_activeCmdBuffer->isInsideRenderPass())
                m_activeCmdBuffer->endRenderPass();

#if VK_USE_QUERIES
            // Pause all active queries
            for (i32 i = 0; i < m_queriesInProgress.count(); i++)
            {
                m_queriesInProgress.get()[i]->interrupt(m_activeCmdBuffer);
            }
#endif

            m_activeCmdBuffer->end();

            if (signalSemaphore)
            {
                m_queue->submit(m_activeCmdBuffer, signalSemaphore->getHandle());
            }
            else
            {
                m_queue->submit(m_activeCmdBuffer);
            }
        }
        m_activeCmdBuffer = nullptr;
    }

    void VulkanCmdBufferManager::waitForCmdBuffer(VulkanCmdBuffer *cmdBuffer, float timeInSecondsToWait)
    {
        PROFILE();
        TK_ASSERT(cmdBuffer->isSubmitted());
        const bool success = m_device->fenceManager.waitForFence(cmdBuffer->getFence(), (u64)(timeInSecondsToWait * 1e9));
        TK_ASSERT(success);
        cmdBuffer->refreshFenceStatus();
    }

    void VulkanCmdBufferManager::prepareForNewActiveCommandBuffer()
    {
        PROFILE();
        TK_ASSERT(m_activeCmdBuffer == nullptr);
        for (i32 i = 0; i < m_pool.m_cmdBuffers.count(); i++)
        {
            auto cmdBuffer = m_pool.m_cmdBuffers.get()[i];
            cmdBuffer->refreshFenceStatus();
            if (cmdBuffer->getState() == VulkanCmdBuffer::State::kReadyForBegin)
            {
                m_activeCmdBuffer = cmdBuffer;
                break;
            }
            else
            {
                TK_ASSERT(cmdBuffer->getState() == VulkanCmdBuffer::State::kSubmitted);
            }
        }

        if (m_activeCmdBuffer == nullptr)
        {
            // Always begin fresh command buffer for rendering
            m_activeCmdBuffer = m_pool.create();
        }

        m_activeCmdBuffer->begin();

#if VK_USE_QUERIES
        // Resume any paused queries with the new command buffer
        for (i32 i = 0; i < m_queriesInProgress.count(); i++)
        {
            m_queriesInProgress.get()[i]->resume(m_activeCmdBuffer);
        }
#endif
    }

    void VulkanCmdBufferManager::onQueryBegin(VulkanTimerQuery *query)
    {
#if VK_USE_QUERIES
        m_queriesInProgress.add(query);
#endif
    }

    void VulkanCmdBufferManager::onQueryEnd(VulkanTimerQuery *query)
    {
#if VK_USE_QUERIES
        m_queriesInProgress.remove(query);
#endif
    }

}

#endif
