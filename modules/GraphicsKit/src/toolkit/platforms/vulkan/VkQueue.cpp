/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkQueue.hpp"

#if GFX_API_VULKAN

#include "VkDevice.hpp"
#include "VkCmdBuffer.hpp"
#include "VkUtils.hpp"

namespace toolkit
{

    VulkanQueue::VulkanQueue(VulkanDevice *device, u32 familyIndex)
        : m_queue(VK_NULL_HANDLE), m_familyIndex(familyIndex), m_queueIndex(0), m_device(device), m_lastSubmittedCmdBuffer(nullptr), m_lastSubmittedCmdBufferFenceCounter(0)
    {
        vkGetDeviceQueue(device->device, familyIndex, 0, &m_queue);
    }

    void VulkanQueue::submit(VulkanCmdBuffer *cmdBuffer, u32 signalSemaphoresCount, const VkSemaphore *signalSemaphores)
    {
        TK_ASSERT(cmdBuffer->hasEnded());
        auto fence = cmdBuffer->getFence();
        TK_ASSERT(!fence->isSignaled);

        const VkCommandBuffer cmdBuffers[] = {cmdBuffer->getHandle()};

        VkSubmitInfo submitInfo;
        VkUtils::zeroStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = cmdBuffers;
        submitInfo.signalSemaphoreCount = signalSemaphoresCount;
        submitInfo.pSignalSemaphores = signalSemaphores;

        TkStaticArray<VkSemaphore, 8> waitSemaphores;
        if (cmdBuffer->m_waitSemaphores.hasItems())
        {
            // TODO waitSemaphores.ensureCapacity((u32)cmdBuffer->m_waitSemaphores.count());
            for (auto semaphore : cmdBuffer->m_waitSemaphores)
            {
                waitSemaphores.add(semaphore->getHandle());
            }
            submitInfo.waitSemaphoreCount = (u32)cmdBuffer->m_waitSemaphores.count();
            submitInfo.pWaitSemaphores = waitSemaphores.get();
            submitInfo.pWaitDstStageMask = cmdBuffer->m_waitFlags.get();
        }

        VALIDATE_VK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, fence->handle));

        // Mark semaphores as submitted
        cmdBuffer->m_state = VulkanCmdBuffer::State::kSubmitted;
        cmdBuffer->m_waitFlags.clear();
        cmdBuffer->m_submittedWaitSemaphores = cmdBuffer->m_waitSemaphores;
        cmdBuffer->m_waitSemaphores.clear();
        cmdBuffer->m_submittedFenceCounter = cmdBuffer->m_fenceSignaledCounter;

#if 0
	// Wait for the GPU to be idle on every submit (useful for tracking GPU hangs)
	const bool WaitForIdleOnSubmit = false;
	if (WaitForIdleOnSubmit)
	{
		// Use 200ms timeout
		bool success = m_device->FenceManager.waitForFence(fence, 200 * 1000 * 1000);
		TK_ASSERT(success);
		TK_ASSERT(m_device->FenceManager.isFenceSignaled(fence));
		cmdBuffer->getOwner()->refreshFenceStatus();
	}
#endif

        updateLastSubmittedCommandBuffer(cmdBuffer);

        cmdBuffer->getOwner()->refreshFenceStatus(cmdBuffer);
    }

    void VulkanQueue::getLastSubmittedInfo(VulkanCmdBuffer *&cmdBuffer, u64 &fenceCounter) const
    {
        m_mtx.lock();
        cmdBuffer = m_lastSubmittedCmdBuffer;
        fenceCounter = m_lastSubmittedCmdBufferFenceCounter;
        m_mtx.unlock();
    }

    void VulkanQueue::updateLastSubmittedCommandBuffer(VulkanCmdBuffer *cmdBuffer)
    {
        m_mtx.lock();
        m_lastSubmittedCmdBuffer = cmdBuffer;
        m_lastSubmittedCmdBufferFenceCounter = cmdBuffer->getFenceSignaledCounter();
        m_mtx.unlock();
    }
}

#endif