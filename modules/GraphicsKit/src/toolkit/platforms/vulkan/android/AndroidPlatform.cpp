/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/vulkan/android/AndroidPlatform.hpp>

#if GFX_API_VULKAN && TK_ANDROID

#include "../VkUtils.hpp"

namespace toolkit
{

    void AndroidVulkanPlatform::getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers)
    {
        extensions.add(VK_KHR_SURFACE_EXTENSION_NAME);
        extensions.add(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    }

    void AndroidVulkanPlatform::getDeviceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers)
    {
        extensions.add(VK_KHR_SURFACE_EXTENSION_NAME);
        extensions.add(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    }

    void AndroidVulkanPlatform::createSurface(void *windowHandle, VkInstance instance, VkSurfaceKHR *surface)
    {
        TK_ASSERT(windowHandle);
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;
        VkUtils::zeroStruct(surfaceCreateInfo, VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR);
        surfaceCreateInfo.window = (struct ANativeWindow *)windowHandle;
        VALIDATE_VK_RESULT(vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, surface));
    }

}

#endif
