/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN && TK_WINDOWS

#include <toolkit/platforms/vulkan/VkPlatform.hpp>

namespace toolkit
{
    class Win32VulkanPlatform : public VulkanPlatformBase
    {
    public:
        static void getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers);
        static void createSurface(void* windowHandle, VkInstance instance, VkSurfaceKHR* outSurface);
    };
    typedef Win32VulkanPlatform VulkanPlatform;

}

#endif
