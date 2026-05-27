/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "VkResourceOwner.hpp"
#include "VkDevice.hpp"
#include "VkTexture.hpp"
#include "toolkit/graphics/GfxSwapChain.hpp"

namespace toolkit
{
    // Represents a Vulkan swap chain back buffer wrapper object.
    class VulkanBackBuffer : public VulkanResourceOwner
    {
    public:
        // The device.
        VulkanDevice *device;
        // The image acquired semaphore handle.
        VulkanSemaphore *imageAcquiredSemaphore;
        // The rendering done semaphore handle.
        VulkanSemaphore *renderingDoneSemaphore;
        // The render target surface handle.
        VulkanTextureView handle;

    public:
        void setup(VulkanSwapChain *window, VkImage backbuffer, PixelFormat format, VkExtent3D extent);
        void release();

    public:
        GfxResource *asResource() const override
        {
            return nullptr;
        }
    };

    // Window swap chain for Vulkan backend
    class VulkanSwapChain : public VulkanResource<GfxSwapChain>, public VulkanResourceOwner
    {
        friend class GPUContextVulkan;
        friend VulkanDevice;

    private:
        VkSurfaceKHR m_surface;
        VkSwapchainKHR m_swapChain;
        i32 m_currentImageIndex;
        i32 m_semaphoreIndex;
        i32 m_acquiredImageIndex;
        TkStaticArray<VulkanBackBuffer, VK_BACK_BUFFERS_COUNT_MAX> m_backBuffers;
        VulkanSemaphore *m_acquiredSemaphore;

    public:
        VulkanSwapChain(VulkanDevice *device, GfxWindow *window);

    public:
        // Get the Vulkan surface.
        TK_FORCE_INLINE VkSurfaceKHR getSurface() const
        {
            return m_surface;
        }

        // Get the Vulkan surface swap chain.
        TK_FORCE_INLINE VkSwapchainKHR getSwapChain() const
        {
            return m_swapChain;
        }

    public:
        enum class Status
        {
            kOk = 0,
            kOutdated = -1,
            kLostSurface = -2,
            kFailed = -3,
        };

        Status present(VulkanQueue *presentQueue, VulkanSemaphore *backBufferRenderingDoneSemaphore);

        static i32 doAcquireImageIndex(VulkanSwapChain *viewport, void *customData);
        static i32 doPresent(VulkanSwapChain *viewport, void *customData);
        i32 tryPresent(TkFunc<i32(VulkanSwapChain *, void *)> job, void *customData = nullptr, bool skipOnOutOfDate = false);
        i32 acquireNextImage(VulkanSemaphore *&outSemaphore);

    private:
        void releaseBackBuffer();
        // Releases swap chain image views + VkSwapchainKHR but KEEPS the
        // VkSurfaceKHR alive. Used during resize so we can pass the old
        // swap chain into vkCreateSwapchainKHR as oldSwapchain (which lets
        // the driver reuse pages) and so we don't churn the surface object
        // (creating/destroying a Vulkan surface is heavy and several ICDs
        // leak memory across surface cycles).
        void releaseSwapchainOnly(bool retainHandleForReuse);
        // Tears down the surface as well. Only used on full dispose or
        // VK_ERROR_SURFACE_LOST_KHR.
        void releaseSurface();
        bool createSwapChain(i32 width, i32 height);

    public:
        bool isFullscreen() override;
        void setFullscreen(bool isFullscreen) override;
        GfxTextureView *getBackBufferView() override;
        void present(bool vsync) override;
        bool resize(i32 width, i32 height) override;
        void copyBackbuffer(GfxContext *context, GfxTexture *dst) override;
        bool isReady() const override;

        GfxResource *asResource() const override
        {
            return (GfxResource *)this;
        }

    protected:
        void onReleaseData() override;
    };

}

#endif
