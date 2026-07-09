/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "toolkit/graphics/GfxResource.hpp"

namespace toolkit
{
// Vulkan resource state used to indicate invalid state (useful for debugging resource tracking issues).
#define VK_IMAGE_LAYOUT_CORRUPT VK_IMAGE_LAYOUT_MAX_ENUM

    // Tracking of per-resource or per-subresource state for Vulkan resources that require to issue resource access barriers during rendering.
    class VulkanResourceState : public GfxResourceState<VkImageLayout, VK_IMAGE_LAYOUT_CORRUPT>
    {
    };

    // Base class for objects in Vulkan backend that can own a resource.
    class VulkanResourceOwner
    {
        friend class VulkanContext;

    public:
        virtual ~VulkanResourceOwner() = default;

    public:
        // The resource state tracking helper. Used for resource barriers.
        VulkanResourceState state;
        // The array size (for textures).
        i32 arraySlices;

    public:
        // Get resource owner object as a GPUResource type or returns null if cannot perform cast.
        virtual GfxResource *asResource() const = 0;

    protected:
        void initResource(VkImageLayout initialState, i32 mipLevels = 1, i32 arraySize = 1, bool usePerSubresourceTracking = false)
        {
            state.init(mipLevels * arraySize, initialState, usePerSubresourceTracking);
            arraySlices = arraySize;
        }
    };

}

#endif