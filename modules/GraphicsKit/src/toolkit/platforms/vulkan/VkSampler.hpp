/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "VkDevice.hpp"
#include "toolkit/graphics/GfxSampler.hpp"

namespace toolkit
{

    // Sampler object for Vulkan backend.
    class VulkanSampler : public VulkanResource<GfxSampler>
    {
    public:
        VulkanSampler(VulkanDevice *device)
            : VulkanResource<GfxSampler>(device, TkStringView::Empty)
        {
        }

        VkSampler sampler = VK_NULL_HANDLE;

    protected:
        bool onInit() override;
        void onReleaseData() override;
    };

}

#endif