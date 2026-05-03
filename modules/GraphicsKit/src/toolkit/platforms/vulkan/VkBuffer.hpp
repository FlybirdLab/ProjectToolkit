/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "toolkit/graphics/GfxBuffer.hpp"
#include "VkDevice.hpp"

namespace toolkit
{

    // The buffer view for Vulkan backend.
    class VulkanBufferView : public GfxBufferView, public VulkanDescriptorOwnerResource
    {
    public:
        VulkanBufferView() = default;

#if TK_DEBUG
        ~VulkanBufferView()
        {
            TK_ASSERT(view == VK_NULL_HANDLE);
        }
#endif

    public:
        VulkanDevice *device = nullptr;
        VulkanBuffer *owner = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkBufferView view = VK_NULL_HANDLE;
        VkDeviceSize size = 0;

    public:
        void init(VulkanDevice *device, VulkanBuffer *owner, VkBuffer buffer, VkDeviceSize size, VkBufferUsageFlags usage, PixelFormat format);
        void release();

    public:
        void *getNativePtr() const override
        {
            return (void *)(VulkanDescriptorOwnerResource *)this;
        }

        void descriptorAsUniformTexelBuffer(VulkanContext *context, VkBufferView &bufferView) override;
        void descriptorAsStorageBuffer(VulkanContext *context, VkBuffer &buffer, VkDeviceSize &offset, VkDeviceSize &range) override;
        void descriptorAsStorageTexelBuffer(VulkanContext *context, VkBufferView &bufferView) override;
#if TK_DEBUG
        bool hasSRV() const override { return ((GfxBuffer *)m_parent)->isShaderResource(); }
        bool hasUAV() const override { return ((GfxBuffer *)m_parent)->isUnorderedAccess(); }
#endif
    };

    // GPU buffer for Vulkan backend.
    class VulkanBuffer : public VulkanResource<GfxBuffer>
    {
    private:
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        VulkanBufferView m_view;

    public:
        VulkanBuffer(VulkanDevice *device, const TkStringView &name)
            : VulkanResource<GfxBuffer>(device, name)
        {
        }

    public:
        // Gets the Vulkan buffer handle.
        TK_FORCE_INLINE VkBuffer getHandle() const
        {
            return m_buffer;
        }

        // Gets the Vulkan memory allocation handle.
        TK_FORCE_INLINE VmaAllocation getAllocation() const
        {
            return m_allocation;
        }

        // The current buffer access flags.
        VkAccessFlags access;
        // The counter buffer attached to the Append/Counter buffers.
        VulkanBuffer *counter = nullptr;

    public:
        GfxBufferView *view() const override;
        void *map(GfxResourceMapMode mode) override;
        void unmap() override;

    protected:
        bool onInit() override;
        void onReleaseData() override;
    };

}
#endif