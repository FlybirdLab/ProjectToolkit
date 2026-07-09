/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "VkDevice.hpp"
#include "toolkit/graphics/GfxVertexLayout.hpp"

namespace toolkit
{
    // Vertex layout object for Vulkan backend.
    class VulkanVertexLayout : public VulkanResource<GfxVertexLayout>
    {
    public:
        VulkanVertexLayout(VulkanDevice *device, const Elements &elements, bool explicitOffsets);
        i32 maxSlot;
    };
}

#endif