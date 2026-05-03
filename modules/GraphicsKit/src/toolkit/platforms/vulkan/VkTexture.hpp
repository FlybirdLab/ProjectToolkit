/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "toolkit/graphics/GfxTexture.hpp"
#include "VkDevice.hpp"
#include "VkResourceOwner.hpp"

namespace toolkit
{
    // The texture view for Vulkan backend.
    class VulkanTextureView : public GfxTextureView, public VulkanDescriptorOwnerResource
    {
    public:
        VulkanTextureView() = default;

#if TK_DEBUG
        ~VulkanTextureView()
        {
            TK_ASSERT(view == VK_NULL_HANDLE);
        }
#endif

    public:
        VulkanDevice *device = nullptr;
        class VulkanResourceOwner *owner = nullptr;
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkImageView viewFramebuffer = VK_NULL_HANDLE;
        VkImageView viewSRV = VK_NULL_HANDLE;
        VkExtent3D extent;
        u32 layers;
        VkImageViewCreateInfo info;
        i32 subresourceIndex;
        VkImageLayout layoutRTV;
        VkImageLayout layoutSRV;
#if VK_USE_DEBUG_DATA
        PixelFormat format;
        bool readOnlyDepth;
#endif

    public:
        void init(VulkanDevice *device, VulkanResourceOwner *owner, VkImage image, i32 totalMipLevels, PixelFormat format, GfxMSAALevel msaa, VkExtent3D extent, VkImageViewType viewType, i32 mipLevels = 1, i32 firstMipIndex = 0, i32 arraySize = 1, i32 firstArraySlice = 0, bool readOnlyDepth = false);
        VkImageView getFramebufferView();
        void release();

    public:
        void *getNativePtr() const override
        {
            return (void *)(VulkanDescriptorOwnerResource *)this;
        }

        void descriptorAsImage(VulkanContext *context, VkImageView &imageView, VkImageLayout &layout) override;
        void descriptorAsStorageImage(VulkanContext *context, VkImageView &imageView, VkImageLayout &layout) override;
#if TK_DEBUG
        bool hasSRV() const override { return ((GfxTexture *)m_parent)->isShaderResource(); }
        bool hasUAV() const override { return ((GfxTexture *)m_parent)->isUnorderedAccess(); }
#endif
    };

    // Texture object for Vulkan backend.
    class VulkanTexture : public VulkanResource<GfxTexture>, public VulkanResourceOwner, public VulkanDescriptorOwnerResource
    {
    private:
        VkImage m_image = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        VulkanTextureView m_handleArray;
        VulkanTextureView m_handleVolume;
        VulkanTextureView m_handleUAV;
        VulkanTextureView m_handleReadOnlyDepth;
        TkArray<VulkanTextureView> m_handlesPerSlice;      // [slice]
        TkArray<TkArray<VulkanTextureView>> m_handlesPerMip; // [slice][mip]

    public:
        VulkanTexture(VulkanDevice *device, const TkStringView &name)
            : VulkanResource<GfxTexture>(device, name)
        {
        }

    public:
        // Gets the Vulkan image handle.
        TK_FORCE_INLINE VkImage getHandle() const
        {
            return m_image;
        }

        // The Vulkan staging buffer (used by the staging textures for memory transfers).
        VulkanBuffer *stagingBuffer = nullptr;
        // The default aspect mask flags for the texture (all planes).
        VkImageAspectFlags defaultAspectMask;

    private:
        void initHandles();

    public:
        GfxTextureView *view(i32 arrayOrDepthIndex) const override
        {
            return (GfxTextureView *)&m_handlesPerSlice[arrayOrDepthIndex];
        }
        GfxTextureView *view(i32 arrayOrDepthIndex, i32 mipMapIndex) const override
        {
            return (GfxTextureView *)&m_handlesPerMip[arrayOrDepthIndex][mipMapIndex];
        }
        GfxTextureView *viewArray() const override
        {
            TK_ASSERT(arraySize() > 1);
            return (GfxTextureView *)&m_handleArray;
        }
        GfxTextureView *viewVolume() const override
        {
            TK_ASSERT(isVolume());
            return (GfxTextureView *)&m_handleVolume;
        }
        GfxTextureView *viewReadOnlyDepth() const override
        {
            TK_ASSERT(m_desc.flags & GfxTextureFlags::kReadOnlyDepthView);
            return (GfxTextureView *)&m_handleReadOnlyDepth;
        }
        void *getNativePtr() const override
        {
            return (void *)m_image;
        }
        bool getData(i32 arrayIndex, i32 mipMapIndex, TextureMipData &data, u32 mipRowPitch) override;

        GfxResource *asResource() const override
        {
            return (GfxResource *)this;
        }
        void descriptorAsStorageImage(VulkanContext *context, VkImageView &imageView, VkImageLayout &layout) override;

    protected:
        bool onInit() override;
        void onResidentMipsChange() override;
        void onReleaseData() override;
    };

}

#endif