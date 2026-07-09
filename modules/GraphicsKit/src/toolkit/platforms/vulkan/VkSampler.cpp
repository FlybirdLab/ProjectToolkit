/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkSampler.hpp"

#if GFX_API_VULKAN

#include "VkUtils.hpp"

namespace toolkit
{
    bool VulkanSampler::onInit()
    {
        VkSamplerCreateInfo createInfo;
        VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
        createInfo.magFilter = VkUtils::toVulkanMagFilterMode(m_desc.filter);
        createInfo.minFilter = VkUtils::toVulkanMinFilterMode(m_desc.filter);
        createInfo.mipmapMode = VkUtils::toVulkanMipFilterMode(m_desc.filter);
        const bool supportsMirrorClampToEdge = VulkanDevice::optionalDeviceExtensions.hasMirrorClampToEdge;
        createInfo.addressModeU = VkUtils::toVulkanWrapMode(m_desc.addressU, supportsMirrorClampToEdge);
        createInfo.addressModeV = VkUtils::toVulkanWrapMode(m_desc.addressV, supportsMirrorClampToEdge);
        createInfo.addressModeW = VkUtils::toVulkanWrapMode(m_desc.addressW, supportsMirrorClampToEdge);
        createInfo.mipLodBias = m_desc.mipBias;
        createInfo.anisotropyEnable = m_desc.filter == GfxSamplerFilter::kAnisotropic ? VK_TRUE : VK_FALSE;
        createInfo.maxAnisotropy = (float)m_desc.maxAnisotropy;
        createInfo.compareEnable = m_desc.comparisonFunction != GfxSamplerCompareFunction::kNever ? VK_TRUE : VK_FALSE;
        createInfo.compareOp = VkUtils::toVulkanSamplerCompareFunction(m_desc.comparisonFunction);
        createInfo.minLod = m_desc.minMipLevel;
        createInfo.maxLod = m_desc.maxMipLevel;
        switch (m_desc.borderColor)
        {
        case GfxSamplerBorderColor::kTransparentBlack:
            createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            break;
        case GfxSamplerBorderColor::kOpaqueBlack:
            createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            break;
        case GfxSamplerBorderColor::kOpaqueWhite:
            createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            break;
        default:
            return false;
        }
        VALIDATE_VK_RESULT(vkCreateSampler(m_device->device, &createInfo, nullptr, &sampler));
        return true;
    }

    void VulkanSampler::onReleaseData()
    {
        if (sampler != VK_NULL_HANDLE)
        {
            m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::kSampler, sampler);
            sampler = VK_NULL_HANDLE;
        }

        // Base
        GfxSampler::onReleaseData();
    }

}

#endif
