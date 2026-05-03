/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkTexture.hpp"

#if GFX_API_VULKAN

#include "VkUtils.hpp"
#include "VkBuffer.hpp"
#include "VkContext.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "toolkit/graphics/utils/TextureData.hpp"

namespace toolkit
{
    void VulkanTextureView::init(VulkanDevice *device, VulkanResourceOwner *owner, VkImage image, i32 totalMipLevels, PixelFormat format, GfxMSAALevel msaa, VkExtent3D extent, VkImageViewType viewType, i32 mipLevels, i32 firstMipIndex, i32 arraySize, i32 firstArraySlice, bool readOnlyDepth)
    {
        TK_ASSERT(view == VK_NULL_HANDLE);

        GfxTextureView::init(owner->asResource(), format, msaa);

        this->device = device;
        this->owner = owner;
        this->image = image;
        this->extent.width = TkMath::max<u32>(1, extent.width >> firstMipIndex);
        this->extent.height = TkMath::max<u32>(1, extent.height >> firstMipIndex);
        this->extent.depth = TkMath::max<u32>(1, extent.depth >> firstMipIndex);
        this->layers = arraySize;
#if VK_USE_DEBUG_DATA
        this->format = format;
        this->readOnlyDepth = readOnlyDepth;
#endif

        VkUtils::zeroStruct(info, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
        info.image = image;
        info.viewType = viewType;
        info.format = VkUtils::toVulkanFormat(format);
        info.components =
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A};

        auto &range = info.subresourceRange;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = firstMipIndex;
        range.levelCount = mipLevels;
        range.baseArrayLayer = firstArraySlice;
        range.layerCount = arraySize;

        if (mipLevels != 1 || arraySize != 1)
        {
            subresourceIndex = -1;
        }
        else
        {
            subresourceIndex = GfxUtils::calcSubresourceIndex(firstMipIndex, firstArraySlice, totalMipLevels);
        }

        if (GfxPixelFormat::isDepthStencil(format))
        {
            range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
#if 0
        // TODO: enable extension and use separateDepthStencilLayouts from Vulkan 1.2
        if (GfxPixelFormat::hasStencil(format))
        {
            range.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            layoutRTV = readOnlyDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            layoutSRV = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }
        else
        {
            layoutRTV = readOnlyDepth ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            layoutSRV = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        }
#else

            if (GfxPixelFormat::hasStencil(format))
                range.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            layoutRTV = readOnlyDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            layoutSRV = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
#endif
        }
        else
        {
            layoutRTV = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            layoutSRV = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        VALIDATE_VK_RESULT(vkCreateImageView(device->device, &info, nullptr, &view));
    }

    VkImageView VulkanTextureView::getFramebufferView()
    {
        if (viewFramebuffer)
            return viewFramebuffer;

        if (info.viewType == VK_IMAGE_VIEW_TYPE_3D)
        {
            // Special case:
            // Render Target Handle to a 3D Volume texture.
            // Use it as Texture2D Array with layers.
            VkImageViewCreateInfo createInfo = info;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            createInfo.subresourceRange.layerCount = extent.depth;
            layers = extent.depth;
            VALIDATE_VK_RESULT(vkCreateImageView(device->device, &createInfo, nullptr, &viewFramebuffer));
        }
        else if (info.subresourceRange.levelCount != 1)
        {
            // Special case:
            // Render Target Handle can be created for full texture including its mip maps but framebuffer image view can use only a single surface
            // Use an additional view for that case with modified level count to 1.
            VkImageViewCreateInfo createInfo = info;
            createInfo.subresourceRange.levelCount = 1;
            VALIDATE_VK_RESULT(vkCreateImageView(device->device, &createInfo, nullptr, &viewFramebuffer));
        }
        else
        {
            viewFramebuffer = view;
        }

        return viewFramebuffer;
    }

    void VulkanTextureView::release()
    {
        if (view != VK_NULL_HANDLE)
        {
            if (viewFramebuffer != view && viewFramebuffer != VK_NULL_HANDLE)
            {
                device->onImageViewDestroy(viewFramebuffer);
                device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kImageView, viewFramebuffer);
            }
            viewFramebuffer = VK_NULL_HANDLE;
            if (viewSRV != view && viewSRV != VK_NULL_HANDLE)
            {
                device->onImageViewDestroy(viewSRV);
                device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kImageView, viewSRV);
            }
            viewSRV = VK_NULL_HANDLE;

            device->onImageViewDestroy(view);
            device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kImageView, view);
            view = VK_NULL_HANDLE;

#if BUILD_DEBUG
            Device = nullptr;
            Owner = nullptr;
            Image = VK_NULL_HANDLE;
#endif
        }
    }

    void VulkanTextureView::descriptorAsImage(VulkanContext *context, VkImageView &imageView, VkImageLayout &layout)
    {
        imageView = view;
        layout = layoutSRV;
        const VkImageAspectFlags aspectMask = info.subresourceRange.aspectMask;
        if (aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
        {
            // Transition depth-only when binding depth buffer with stencil
            if (viewSRV == VK_NULL_HANDLE)
            {
                VkImageViewCreateInfo createInfo = info;
                createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                VALIDATE_VK_RESULT(vkCreateImageView(device->device, &createInfo, nullptr, &viewSRV));
            }
            imageView = viewSRV;
        }
        context->addImageBarrier(this, layoutSRV);
        info.subresourceRange.aspectMask = aspectMask;
    }

    void VulkanTextureView::descriptorAsStorageImage(VulkanContext *context, VkImageView &imageView, VkImageLayout &layout)
    {
        imageView = view;
        layout = VK_IMAGE_LAYOUT_GENERAL;
        context->addImageBarrier(this, VK_IMAGE_LAYOUT_GENERAL);
    }

    bool VulkanTexture::getData(i32 arrayIndex, i32 mipMapIndex, TextureMipData &data, u32 mipRowPitch)
    {
        if (!isStaging())
        {
            LogWarn("Texture::getData is valid only for staging resources.");
            return false;
        }
        GfxDeviceLock lock(m_device);

        // Internally it's a buffer, so adapt resource index and offset
        const u32 subresource = mipMapIndex + arrayIndex * mipLevels();
        // TODO: rowAlign/sliceAlign on Vulkan texture ???
        i32 offsetInBytes = computeBufferOffset(subresource, 1, 1);
        i32 lengthInBytes = computeSubresourceSize(subresource, 1, 1);
        i32 rowPitch = computeRowPitch(mipMapIndex, 1);
        i32 depthPitch = computeSlicePitch(mipMapIndex, 1);

        // Map the staging resource mip map for reading
        auto allocation = stagingBuffer->getAllocation();
        void *mapped;
        const VkResult result = vmaMapMemory(m_device->allocator, allocation, (void **)&mapped);
        LOG_VK_RESULT_WITH_RETURN(result);

        // Shift mapped buffer to the beginning of the mip data start
        mapped = (void *)((byte *)mapped + offsetInBytes);

        data.copy(mapped, rowPitch, depthPitch, depth(), mipRowPitch);

        // Unmap resource
        vmaUnmapMemory(m_device->allocator, allocation);

        return true;
    }

    void VulkanTexture::descriptorAsStorageImage(VulkanContext *context, VkImageView &imageView, VkImageLayout &layout)
    {
        TK_ASSERT(m_handleUAV.owner == this);
        imageView = m_handleUAV.view;
        layout = VK_IMAGE_LAYOUT_GENERAL;
        context->addImageBarrier(this, VK_IMAGE_LAYOUT_GENERAL);
    }

    bool VulkanTexture::onInit()
    {
        // Check if texture should have optimal CPU read/write access
        if (isStaging())
        {
            // TODO: rowAlign/sliceAlign on Vulkan texture ???
            const i32 totalSize = computeBufferTotalSize(1, 1);
            stagingBuffer = (VulkanBuffer *)m_device->createBuffer("Texture.stagingBuffer");
            if (!stagingBuffer->init(GfxBufferDescription::buffer(totalSize, GfxBufferFlags::kNone, PixelFormat::kUnknown, nullptr, 0, m_desc.usage)))
            {
                TkMemory::deletePtr(stagingBuffer);
                return false;
            }
            m_memoryUsage = 1;
            initResource(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_desc.mipLevels, m_desc.arraySize, false);
            return true;
        }

        const bool useSRV = isShaderResource();
        const bool useDSV = isDepthStencil();
        const bool useRTV = isRenderTarget();
        const bool useUAV = isUnorderedAccess();

        const bool optimalTiling = true;
        PixelFormat format = m_desc.format;
        if (useDSV)
            format = GfxPixelFormat::findDepthStencilFormat(format);
        m_desc.format = m_device->getClosestSupportedPixelFormat(format, m_desc.flags, optimalTiling);
        if (m_desc.format == PixelFormat::kUnknown)
        {
            LogError("Unsupported texture format %i.", format);
            return false;
        }

        // Setup texture description
        VkImageCreateInfo imageInfo;
        VkUtils::zeroStruct(imageInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
        imageInfo.imageType = isVolume() ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
        imageInfo.format = VkUtils::toVulkanFormat(this->format());
        imageInfo.mipLevels = mipLevels();
        imageInfo.arrayLayers = arraySize();
        imageInfo.extent.width = width();
        imageInfo.extent.height = height();
        imageInfo.extent.depth = depth();
        imageInfo.flags = isCubeMap() ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
        if (isSRGB())
            imageInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
#if VK_KHR_maintenance1
        if (m_device->optionalDeviceExtensions.hasKHRMaintenance1 && imageInfo.imageType == VK_IMAGE_TYPE_3D)
            imageInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;
#endif
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (useSRV)
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (useDSV)
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (useRTV)
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (useUAV)
            imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
#if TK_MACOS || TK_IOS
        // MoltenVK: VK_ERROR_FEATURE_NOT_PRESENT: vkCreateImageView(): 2D views on 3D images can only be used as color attachments.
        if (isVolume() && m_desc.hasPerSliceViews())
            imageInfo.usage &= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
#endif
        imageInfo.tiling = optimalTiling ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
        imageInfo.samples = (VkSampleCountFlagBits)multiSampleLevel();
        // TODO: set initialLayout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL for IsRegularTexture() ???

        // Create texture
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        const VkResult result = vmaCreateImage(m_device->allocator, &imageInfo, &allocInfo, &m_image, &m_allocation, nullptr);
        LOG_VK_RESULT_WITH_RETURN(result);
#if GFX_ENABLE_RESOURCE_NAMING
        VK_SET_DEBUG_NAME(m_device, m_image, VK_OBJECT_TYPE_IMAGE, getName());
#endif
        // LOG(Warning, "VULKAN TEXTURE -> 0x{0:x} - {1}", (int)m_image, GetName());

        // Set state
        initResource(VK_IMAGE_LAYOUT_UNDEFINED, m_desc.mipLevels, m_desc.arraySize, true);
        m_memoryUsage = calculateMemoryUsage();
        if (GfxPixelFormat::isDepthStencil(format))
        {
            defaultAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (GfxPixelFormat::hasStencil(format))
                defaultAspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        else
        {
            defaultAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        // Initialize handles to the resource
        if (isRegularTexture())
        {
            // 'Regular' texture is using only one handle (texture/cubemap)
            m_handlesPerSlice.resize(1, false);
        }
        else
        {
            // Create all handles
            initHandles();
        }

        return true;
    }

    void VulkanTexture::initHandles()
    {
        // Cache properties
        const i32 arraySize = this->arraySize();
        const i32 mipLevels = this->mipLevels();
        const bool isArray = arraySize > 1;
        const bool isCubeMap = this->isCubeMap();
        const bool isVolume = this->isVolume();
        const auto format = this->format();
        const auto msaa = this->multiSampleLevel();
        VkExtent3D extent;
        extent.width = width();
        extent.height = height();
        extent.depth = depth();

        // Create resource views
        if (isVolume)
        {
            // Create handle for whole 3d texture
            m_handleVolume.init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_3D, mipLevels, 0);

            // Init per slice views
            m_handlesPerSlice.resize(depth(), false);
            if (m_desc.hasPerSliceViews())
            {
                for (i32 sliceIndex = 0; sliceIndex < depth(); sliceIndex++)
                {
                    m_handlesPerSlice[sliceIndex].init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_2D, mipLevels, 0, 1, sliceIndex);
                }
            }
        }
        else if (isArray)
        {
            // Resize handles
            m_handlesPerSlice.resize(this->arraySize(), false);

            // Create per array slice handles
            for (i32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
            {
                m_handlesPerSlice[arrayIndex].init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_2D, mipLevels, 0, 1, arrayIndex);
            }

            // Create whole array handle
            if (isCubeMap)
            {
                m_handleArray.init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_CUBE, mipLevels, 0, arraySize, 0);
            }
            else
            {
                m_handleArray.init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_2D_ARRAY, mipLevels, 0, arraySize, 0);
            }
        }
        else
        {
            // Create single handle for the whole texture
            m_handlesPerSlice.resize(1, false);
            if (isCubeMap)
            {
                m_handlesPerSlice[0].init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_CUBE, mipLevels, 0, arraySize);
            }
            else
            {
                m_handlesPerSlice[0].init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_2D, mipLevels);
            }
        }

        // Init per mip map handles
        if (hasPerMipViews())
        {
            // Init handles
            m_handlesPerMip.resize(arraySize, false);
            for (i32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
            {
                auto &slice = m_handlesPerMip[arrayIndex];
                slice.resize(mipLevels, false);

                for (i32 mipIndex = 0; mipIndex < mipLevels; mipIndex++)
                {
                    slice[mipIndex].init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_2D, 1, mipIndex, 1, arrayIndex);
                }
            }
        }

        // UAV texture
        if (isUnorderedAccess())
        {
            if (isVolume)
            {
                m_handleUAV.init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_3D, 1, 0, 1, 0);
            }
            else if (isArray)
            {
                m_handleUAV.init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_2D_ARRAY, 1, 0, arraySize, 0);
            }
            else
            {
                m_handleUAV.init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_2D, 1, 0, 1, 0);
            }
        }

        // Read-only depth-stencil
        if (enumHasAnyFlags(m_desc.flags, GfxTextureFlags::kReadOnlyDepthView))
        {
            m_handleReadOnlyDepth.init(m_device, this, m_image, mipLevels, format, msaa, extent, VK_IMAGE_VIEW_TYPE_2D, mipLevels, 0, 1, 0, true);
        }
    }

    void VulkanTexture::onResidentMipsChange()
    {
        // Update view
        VkExtent3D extent;
        extent.width = width();
        extent.height = height();
        extent.depth = depth();
        const i32 firstMipIndex = mipLevels() - residentMipLevels();
        const i32 mipLevels = residentMipLevels();
        const VkImageViewType viewType = isVolume() ? VK_IMAGE_VIEW_TYPE_3D : (isCubeMap() ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D);
        VulkanTextureView &view = isVolume() ? m_handleVolume : m_handlesPerSlice[0];
        view.release();
        view.init(m_device, this, m_image, mipLevels, format(), multiSampleLevel(), extent, viewType, mipLevels, firstMipIndex, arraySize());
    }

    void VulkanTexture::onReleaseData()
    {
        m_handleArray.release();
        m_handleVolume.release();
        m_handleUAV.release();
        m_handleReadOnlyDepth.release();
        for (i32 i = 0; i < m_handlesPerMip.count(); i++)
        {
            for (i32 j = 0; j < m_handlesPerMip[i].count(); j++)
            {
                m_handlesPerMip[i][j].release();
            }
        }
        for (i32 i = 0; i < m_handlesPerSlice.count(); i++)
        {
            m_handlesPerSlice[i].release();
        }
        m_handlesPerMip.resize(0, false);
        m_handlesPerSlice.resize(0, false);
        if (m_image != VK_NULL_HANDLE)
        {
            m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kImage, m_image, m_allocation);
            m_image = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
        }
        GFX_DEL_RESOURCE(stagingBuffer);
        state.release();

        // Base
        GfxTexture::onReleaseData();
    }

}

#endif