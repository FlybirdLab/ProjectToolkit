/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkSwapChain.hpp"

#if GFX_API_VULKAN

#include "VkCmdBuffer.hpp"
#include "VkContext.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/graphics/GfxManager.hpp"
#include "toolkit/graphics/GfxWindow.hpp"
#include "VkPlatform.hpp"
#include "VkAdapter.hpp"
#include "toolkit/collection/StaticArray.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "VkUtils.hpp"
#include "toolkit/platforms/vulkan/VkQueue.hpp"

namespace toolkit
{
    void VulkanBackBuffer::setup(VulkanSwapChain *window, VkImage backbuffer, PixelFormat format, VkExtent3D extent)
    {
        // Cache handle and set default initial state for the backbuffers
        initResource(VK_IMAGE_LAYOUT_UNDEFINED);

        device = window->getDevice();
        handle.init(window->getDevice(), this, backbuffer, 1, format, GfxMSAALevel::kNone, extent, VK_IMAGE_VIEW_TYPE_2D);
        renderingDoneSemaphore = TkMemory::newObject<VulkanSemaphore>(TkMemory::getDefault(),device);
        imageAcquiredSemaphore = TkMemory::newObject<VulkanSemaphore>(TkMemory::getDefault(), device);
    }

    void VulkanBackBuffer::release()
    {
        handle.release();
        TkMemory::deletePtr(TkMemory::getDefault(), renderingDoneSemaphore);
        TkMemory::deletePtr(TkMemory::getDefault(), imageAcquiredSemaphore);
    }

    VulkanSwapChain::VulkanSwapChain(VulkanDevice *device, GfxWindow *window)
        : VulkanResource(device, TkStringView::Empty), m_surface(VK_NULL_HANDLE), m_swapChain(VK_NULL_HANDLE), m_currentImageIndex(-1), m_semaphoreIndex(0), m_acquiredImageIndex(-1), m_acquiredSemaphore(nullptr)
    {
        m_window = window;
    }

    void VulkanSwapChain::releaseBackBuffer()
    {
        for (i32 i = 0; i < m_backBuffers.count(); i++)
        {
            m_backBuffers[i].release();
        }
        m_backBuffers.clear();
    }

    void VulkanSwapChain::releaseSwapchainOnly(bool retainHandleForReuse)
    {
        GfxDeviceLock lock(m_device);

        m_device->waitForGPU();

        releaseBackBuffer();

        m_currentImageIndex = -1;
        m_semaphoreIndex = 0;
        m_acquiredImageIndex = -1;
        m_acquiredSemaphore = nullptr;

        if (!retainHandleForReuse && m_swapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_device->device, m_swapChain, nullptr);
            m_swapChain = VK_NULL_HANDLE;
        }
        m_width = m_height = 0;
    }

    void VulkanSwapChain::releaseSurface()
    {
        if (m_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(VulkanDevice::instance, m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }
    }

    void VulkanSwapChain::onReleaseData()
    {
        releaseSwapchainOnly(false);
        releaseSurface();
    }

    bool VulkanSwapChain::isFullscreen()
    {
#if TK_ANDROID || TK_IOS
        // Not supported
        return true;
#else
        return false;
#endif
    }

    void VulkanSwapChain::setFullscreen(bool isFullscreen)
    {
#if TK_ANDROID || TK_IOS
        // Not supported
#else
        if (!m_surface)
            return;
        // TODO: support fullscreen mode on Vulkan
        TK_ASSERT(false && "support fullscreen mode on Vulkan");
#endif
    }

    GfxTextureView *VulkanSwapChain::getBackBufferView()
    {
        if (m_acquiredImageIndex == -1)
        {
            PROFILE();
            if (tryPresent(doAcquireImageIndex) < 0)
            {
                LogError("Swapchain acquire image index failed!");
                return nullptr;
            }
            TK_ASSERT(m_acquiredImageIndex != -1);

            auto context = m_device->mainContext;
            const auto backBuffer = &m_backBuffers[m_acquiredImageIndex].handle;

            auto cmdBufferManager = context->getCmdBufferManager();
            auto cmdBuffer = cmdBufferManager->getCmdBuffer();

            // Transition to render target (typical usage in most cases when calling backbuffer getter)
            context->addImageBarrier(backBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            context->flushBarriers();

            // Submit here so we can add a dependency with the acquired semaphore
            cmdBuffer->addWaitSemaphore(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_acquiredSemaphore);
            cmdBufferManager->submitActiveCmdBuffer();
            cmdBufferManager->prepareForNewActiveCommandBuffer();
            TK_ASSERT(cmdBufferManager->hasPendingActiveCmdBuffer() && cmdBufferManager->getActiveCmdBuffer()->getState() == VulkanCmdBuffer::State::kIsInsideBegin);
        }
        return &m_backBuffers[m_acquiredImageIndex].handle;
    }

    bool VulkanSwapChain::resize(i32 width, i32 height)
    {
        if (width <= 0 || height <= 0)
            return false;

        // Check if size won't change
        if (width == m_width && height == m_height)
            return true;

        // Wait for GPU to flush commands
        m_device->waitForGPU();

        return createSwapChain(width, height);
    }

    void VulkanSwapChain::copyBackbuffer(GfxContext *context, GfxTexture *dst)
    {
        const auto contextVulkan = (VulkanContext *)context;

        const auto dstVulkan = (VulkanTexture *)dst;
        const auto backBuffer = (VulkanTextureView *)getBackBufferView();

        contextVulkan->addImageBarrier(dstVulkan, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        contextVulkan->addImageBarrier(backBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        contextVulkan->flushBarriers();

        TK_ASSERT(dstVulkan->mipLevels() == 1 && dstVulkan->arraySize() == 1 && dstVulkan->format() == m_format);
        VkImageCopy region;
        region.extent = {(u32)dstVulkan->width(), (u32)dstVulkan->height(), 1};
        region.srcOffset = {0, 0, 0};
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.layerCount = 1;
        region.srcSubresource.mipLevel = 0;
        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstOffset = {0, 0, 0};
        region.dstSubresource.baseArrayLayer = 0;
        region.dstSubresource.layerCount = 1;
        region.dstSubresource.mipLevel = 0;
        region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vkCmdCopyImage(contextVulkan->getCmdBufferManager()->getCmdBuffer()->getHandle(), backBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    bool VulkanSwapChain::isReady() const
    {
        return m_swapChain != VK_NULL_HANDLE && m_backBuffers.hasItems() && m_width > 0 && m_height > 0;
    }

    bool VulkanSwapChain::createSwapChain(i32 width, i32 height)
    {
        // Skip if window handle is missing (eg. Android window is not yet visible)
        auto windowHandle = m_window->getWindowPtr();
        if (!windowHandle)
            return false;
        PROFILE();
        GfxDeviceLock lock(m_device);
        const auto device = m_device->device;

        // Recreation path: tear down image views + backbuffers but KEEP the
        // surface and the old VkSwapchainKHR (we'll feed it into the new
        // swapchain's oldSwapchain field so the driver can reuse memory).
        const bool isRecreate = (m_swapChain != VK_NULL_HANDLE);
        VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE;
        if (isRecreate)
        {
            oldSwapChain = m_swapChain;
            releaseSwapchainOnly(true);
            // Make sure stale image views / framebuffers from the previous
            // swapchain are gone before the driver hands us new images that
            // might re-use those handles.
            m_device->deferredDeletionQueue.releaseResources(true);
        }
        // First-time path: create the platform-specific VkSurfaceKHR exactly once.
        if (m_surface == VK_NULL_HANDLE)
        {
            VulkanPlatform::createSurface(windowHandle, VulkanDevice::instance, &m_surface);
            if (m_surface == VK_NULL_HANDLE)
            {
                LogWarn("Failed to create Vulkan surface.");
                return false;
            }
        }
        m_memoryUsage = 1;

        const auto &gpu = m_device->adapter->device;

        // Pick a format for backbuffer
        PixelFormat resultFormat = GFX_BACK_BUFFER_PIXEL_FORMAT;
        VkSurfaceFormatKHR result;
        TkMemory::memoryClear(&result, sizeof(result));
        {
            u32 surfaceFormatsCount;
            VALIDATE_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_surface, &surfaceFormatsCount, nullptr));
            TkStaticArray<VkSurfaceFormatKHR, 16> surfaceFormats;
            // instead: surfaceFormats.addZeroed(surfaceFormatsCount);
            for (auto i = 0; i < surfaceFormatsCount; i ++) {
                VkSurfaceFormatKHR zero {};
                surfaceFormats.add(zero);
            }
            VALIDATE_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_surface, &surfaceFormatsCount, surfaceFormats.get()));

            if (resultFormat != PixelFormat::kUnknown)
            {
                bool found = false;
                if (enumHasAllFlags(m_device->features[(i32)resultFormat].support, FormatSupport::kRenderTarget))
                {
                    const VkFormat requested = VkUtils::toVulkanFormat(resultFormat);
                    for (i32 i = 0; i < surfaceFormats.count(); i++)
                    {
                        if (surfaceFormats[i].format == requested)
                        {
                            result = surfaceFormats[i];
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        LogWarn("Requested pixel format %d not supported by this swapchain. Falling back to supported swapchain formats...", (i32)resultFormat);
                        resultFormat = PixelFormat::kUnknown;
                    }
                }
                else
                {
                    LogWarn("Requested pixel format %d is not supported by this Vulkan implementation", (i32)resultFormat);
                    resultFormat = PixelFormat::kUnknown;
                }
            }

            if (resultFormat == PixelFormat::kUnknown)
            {
                for (i32 i = 0; i < surfaceFormats.count(); i++)
                {
                    // Reverse lookup
                    TK_ASSERT(surfaceFormats[i].format != VK_FORMAT_UNDEFINED);
                    for (i32 pixelFormat = 0; pixelFormat < static_cast<i32>(PixelFormat::MAX); pixelFormat++)
                    {
                        if (surfaceFormats[i].format == VkUtils::toVulkanFormat(static_cast<PixelFormat>(pixelFormat)))
                        {
                            resultFormat = static_cast<PixelFormat>(pixelFormat);
                            result = surfaceFormats[i];
                            LogInfo("No swapchain format requested, picking up format %d (vk=%d)", (i32)resultFormat, (i32)result.format);
                            break;
                        }
                    }
                    if (resultFormat != PixelFormat::kUnknown)
                        break;
                }
            }

            if (resultFormat == PixelFormat::kUnknown)
            {
                LogWarn("Can't find a proper pixel format for the swapchain, trying to pick up the first available");
                const VkFormat format = VkUtils::toVulkanFormat(resultFormat);
                bool supported = false;
                for (i32 i = 0; i < surfaceFormats.count(); i++)
                {
                    if (surfaceFormats[i].format == format)
                    {
                        supported = true;
                        result = surfaceFormats[i];
                        break;
                    }
                }
                TK_ASSERT(supported);
                TkString msg;
                for (i32 index = 0; index < surfaceFormats.count(); index++)
                {
                    msg += index == 0 ? TEXT("(") : TEXT(", ");
                    msg += TkStringUtils::toString((i32)surfaceFormats[index].format);
                }
                if (surfaceFormats.hasItems())
                    msg += TEXT(")");
                LogError("Unable to find a pixel format for the swapchain; swapchain returned %d Vulkan formats %s", surfaceFormats.count(), *msg);
                return false;
            }
        }
        result.format = VkUtils::toVulkanFormat(resultFormat);
        m_format = resultFormat;

        // Prepare present queue
        m_device->setupPresentQueue(m_surface);

        // Calculate the swap chain present mode
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        {
            u32 presentModesCount = 0;
            VALIDATE_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, m_surface, &presentModesCount, nullptr));
            TkStaticArray<VkPresentModeKHR, 8> presentModes; //TODO 4->8
            presentModes.resize(presentModesCount);
            VALIDATE_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, m_surface, &presentModesCount, presentModes.get()));
            bool foundPresentModeMailbox = false;
            bool foundPresentModeImmediate = false;
            bool foundPresentModeFifo = false;
            for (size_t i = 0; i < presentModesCount; i++)
            {
                switch (presentModes[(i32)i])
                {
                case VK_PRESENT_MODE_MAILBOX_KHR:
                    foundPresentModeMailbox = true;
                    break;
                case VK_PRESENT_MODE_IMMEDIATE_KHR:
                    foundPresentModeImmediate = true;
                    break;
                case VK_PRESENT_MODE_FIFO_KHR:
                    foundPresentModeFifo = true;
                    break;
                default: break;
                }
            }
            if (GfxManager::useVSync && foundPresentModeFifo)
            {
                presentMode = VK_PRESENT_MODE_FIFO_KHR;
            }
            else if (foundPresentModeMailbox)
            {
                presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
            else if (foundPresentModeImmediate)
            {
                presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
            else if (foundPresentModeFifo)
            {
                presentMode = VK_PRESENT_MODE_FIFO_KHR;
            }
            else
            {
                LogWarn("Couldn't find desired Vulkan present mode! Using %d", static_cast<i32>(presentModes[0]));
                presentMode = presentModes[0];
            }
        }

        // Check the surface properties and formats
        VkSurfaceCapabilitiesKHR surfProperties;
        VALIDATE_VK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, m_surface, &surfProperties));
        width = TkMath::clamp<i32>(width, surfProperties.minImageExtent.width, surfProperties.maxImageExtent.width);
        height = TkMath::clamp<i32>(height, surfProperties.minImageExtent.height, surfProperties.maxImageExtent.height);
        if (width <= 0 || height <= 0)
        {
            LogError("Vulkan swapchain dimensions are invalid %dx%d (minImageExtent=%ux%u, maxImageExtent=%ux%u)", width, height, surfProperties.minImageExtent.width, surfProperties.minImageExtent.height, surfProperties.maxImageExtent.width, surfProperties.maxImageExtent.height);
            return false;
        }
        TK_ASSERT(surfProperties.minImageCount <= VK_BACK_BUFFERS_COUNT_MAX);
        VkSwapchainCreateInfoKHR swapChainInfo;
        VkUtils::zeroStruct(swapChainInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
        swapChainInfo.surface = m_surface;
        swapChainInfo.minImageCount = surfProperties.maxImageCount > 0 // A value of 0 means that there is no limit on the number of image
                                          ? TkMath::min<u32>(VK_BACK_BUFFERS_COUNT, surfProperties.maxImageCount)
                                          : VK_BACK_BUFFERS_COUNT;
        swapChainInfo.minImageCount = TkMath::max<u32>(swapChainInfo.minImageCount, surfProperties.minImageCount);
        swapChainInfo.minImageCount = TkMath::min<u32>(swapChainInfo.minImageCount, VK_BACK_BUFFERS_COUNT_MAX);
        swapChainInfo.imageFormat = result.format;
        swapChainInfo.imageColorSpace = result.colorSpace;
        swapChainInfo.imageExtent.width = width;
        swapChainInfo.imageExtent.height = height;
        swapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
#if GFX_USE_WINDOW_SRV
        swapChainInfo.imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
#endif
        swapChainInfo.preTransform = surfProperties.currentTransform;
        if (surfProperties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapChainInfo.imageArrayLayers = 1;
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainInfo.presentMode = presentMode;
        swapChainInfo.clipped = VK_TRUE;
        swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        if (surfProperties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
            swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        // Letting the driver reuse memory pages from the previous swapchain
        // avoids ICD-side allocator churn on every window resize.
        swapChainInfo.oldSwapchain = oldSwapChain;

        // Create swap chain
        VkBool32 supportsPresent;
        VALIDATE_VK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, m_device->presentQueue->getFamilyIndex(), m_surface, &supportsPresent));
        TK_ASSERT(supportsPresent);
        VALIDATE_VK_RESULT(vkCreateSwapchainKHR(device, &swapChainInfo, nullptr, &m_swapChain));

        // Spec: after a successful vkCreateSwapchainKHR with oldSwapchain, the
        // old handle is in retired state and safe to destroy. Driver pages
        // referenced by the new chain are kept alive automatically.
        if (oldSwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, oldSwapChain, nullptr);
        }

        // Cache data
        m_width = width;
        m_height = height;

        // Setup back buffers
        {
            u32 imagesCount;
            VALIDATE_VK_RESULT(vkGetSwapchainImagesKHR(device, m_swapChain, &imagesCount, nullptr));
            if (imagesCount < 1 || imagesCount > VK_BACK_BUFFERS_COUNT_MAX)
            {
                LogWarn("Vulkan swapchain got invalid amount of backbuffers %u instead of %u (min %u)", imagesCount, VK_BACK_BUFFERS_COUNT, swapChainInfo.minImageCount);
                return false;
            }
            VkImage images[VK_BACK_BUFFERS_COUNT_MAX];
            VALIDATE_VK_RESULT(vkGetSwapchainImagesKHR(device, m_swapChain, &imagesCount, images));

            m_backBuffers.resize(imagesCount);
            VkExtent3D extent;
            extent.width = m_width;
            extent.height = m_height;
            extent.depth = 1;
            for (u32 i = 0; i < imagesCount; i++)
            {
                m_backBuffers.get()[i].setup(this, images[i], m_format, extent);
            }
        }

        // Estimate memory usage
        m_memoryUsage = 1024 + GfxUtils::calculateTextureMemoryUsage(m_format, m_width, m_height, 1) * m_backBuffers.count();

        return true;
    }

    VulkanSwapChain::Status VulkanSwapChain::present(VulkanQueue *presentQueue, VulkanSemaphore *backBufferrenderingDoneSemaphore)
    {
        if (m_currentImageIndex == -1)
            return Status::kOk;

        VkPresentInfoKHR presentInfo;
        VkUtils::zeroStruct(presentInfo, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
        VkSemaphore semaphore;
        if (backBufferrenderingDoneSemaphore)
        {
            presentInfo.waitSemaphoreCount = 1;
            semaphore = backBufferrenderingDoneSemaphore->getHandle();
            presentInfo.pWaitSemaphores = &semaphore;
        }
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain;
        presentInfo.pImageIndices = (u32 *)&m_currentImageIndex;

        const VkResult presentResult = vkQueuePresentKHR(presentQueue->getHandle(), &presentInfo);
        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            return Status::kOutdated;
        }
        if (presentResult == VK_ERROR_SURFACE_LOST_KHR)
        {
            return Status::kLostSurface;
        }
#if GFX_ENABLE_ASSERTION
        if (presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR)
        {
            VALIDATE_VK_RESULT(presentResult);
            return Status::kFailed;
        }
#else
        if (presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR)
        {
            return Status::kFailed;
        }
#endif

        return Status::kOk;
    }

    i32 VulkanSwapChain::doAcquireImageIndex(VulkanSwapChain *viewport, void *customData)
    {
        return viewport->m_acquiredImageIndex = viewport->acquireNextImage(viewport->m_acquiredSemaphore);
    }

    i32 VulkanSwapChain::doPresent(VulkanSwapChain *viewport, void *customData)
    {
        return (i32)viewport->present((VulkanQueue *)customData, viewport->m_backBuffers[viewport->m_acquiredImageIndex].renderingDoneSemaphore);
    }

    i32 VulkanSwapChain::tryPresent(TkFunc<i32(VulkanSwapChain *, void *)> job, void *customData, bool skipOnOutOfDate)
    {
        i32 attemptsPending = 4;
        i32 status = job(this, customData);
        while (status < 0 && attemptsPending > 0)
        {
            if (status == (i32)Status::kOutdated)
            {
                // LogWarn("Swapchain is out of date");
                if (skipOnOutOfDate)
                    return status;
            }
            else if (status == (i32)Status::kLostSurface)
            {
                LogWarn("Swapchain surface lost");
                // Only this path actually needs a new VkSurfaceKHR. The
                // out-of-date path below reuses the existing surface to
                // avoid the heavy create/destroy cycle in the ICD.
                releaseSurface();
            }
            else
            {
                return status;
            }

            // Recreate swapchain (createSwapChain handles the existing-surface
            // case internally and feeds the old VkSwapchainKHR into oldSwapchain).
            TK_ASSERT(m_swapChain != VK_NULL_HANDLE || m_surface == VK_NULL_HANDLE);
            i32 width = m_width, height = m_height;
            m_device->waitForGPU();
            m_device->deferredDeletionQueue.releaseResources(true);
            if (!createSwapChain(width, height))
                return status;

            // Flush commands
            m_device->getMainContext()->flush();
            m_device->waitForGPU();

            status = job(this, customData);
            attemptsPending--;
        }
        return status;
    }

    i32 VulkanSwapChain::acquireNextImage(VulkanSemaphore *&outSemaphore)
    {
        PROFILE();
        TK_ASSERT(m_swapChain && m_backBuffers.hasItems());

        u32 imageIndex = m_currentImageIndex;
        const i32 prevSemaphoreIndex = m_semaphoreIndex;
        m_semaphoreIndex = (m_semaphoreIndex + 1) % m_backBuffers.count();
        const auto semaphore = m_backBuffers[m_semaphoreIndex].imageAcquiredSemaphore;

        const VkResult result = vkAcquireNextImageKHR(
            m_device->device,
            m_swapChain,
            UINT64_MAX,
            semaphore->getHandle(),
            VK_NULL_HANDLE,
            &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_semaphoreIndex = prevSemaphoreIndex;
            return (i32)Status::kOutdated;
        }
        if (result == VK_ERROR_SURFACE_LOST_KHR)
        {
            m_semaphoreIndex = prevSemaphoreIndex;
            return (i32)Status::kLostSurface;
        }
        outSemaphore = semaphore;
        if (result == VK_ERROR_VALIDATION_FAILED_EXT)
        {
            LogError("vkAcquireNextImageKHR failed with validation error");
            m_semaphoreIndex = prevSemaphoreIndex;
            return (i32)Status::kFailed;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
#if GFX_ENABLE_ASSERTION
            VkUtils::logResult(result, __FILE__, __LINE__);
#endif
            m_semaphoreIndex = prevSemaphoreIndex;
            return (i32)Status::kFailed;
        }
        m_currentImageIndex = (i32)imageIndex;

        return m_currentImageIndex;
    }

    void VulkanSwapChain::present(bool vsync)
    {
        // Skip if there was no rendering to the backbuffer
        if (m_acquiredImageIndex == -1)
            return;
        PROFILE();

        // Ensure that backbuffer has been acquired before presenting it to the window
        const auto backBuffer = (VulkanTextureView *)getBackBufferView();

        // Ensure to keep backbuffer in a proper layout
        auto context = m_device->mainContext;
        context->addImageBarrier(backBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        context->flushBarriers();

        context->getCmdBufferManager()->submitActiveCmdBuffer(m_backBuffers[m_acquiredImageIndex].renderingDoneSemaphore);

        // Present the back buffer to the viewport window
        const auto result = tryPresent(doPresent, m_device->presentQueue, true);
        if (result == (i32)Status::kOutdated)
        {
            // Failed to present, window can be minimized or doesn't want to swap the buffers so just ignore the present
            if (m_window->isMinimized())
            {
                return;
            }

            // Rebuild swapchain for the next present. Surface stays alive —
            // createSwapChain will reuse it and pass the previous handle
            // through oldSwapchain so the driver reuses backbuffer memory.
            i32 width = m_width, height = m_height;
            if (!createSwapChain(width, height))
            {
                LogError("Swapchain recreate failed!");
                return;
            }
            m_device->getMainContext()->flush();
            m_device->waitForGPU();
            return;
        }
        if (result < 0)
        {
            LogError("Swapchain present failed!");
        }

        // Release the back buffer
        m_acquiredImageIndex = -1;

        // Base
        GfxSwapChain::present(vsync);
    }

}

#endif
