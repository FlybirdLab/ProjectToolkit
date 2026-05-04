/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/vulkan/ios/iOSPlatform.h>

#if GFX_API_VULKAN && TK_IOS

#include "toolkit/base/Function.hpp"
#include "../VkUtils.hpp"
#import <UIKit/UIKit.h>

namespace toolkit
{

void iOSVulkanPlatform::getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers)
{
    extensions.add(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.add(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
}

void iOSVulkanPlatform::createSurface(void *windowHandle, VkInstance instance, VkSurfaceKHR *surface)
{
    // Create surface on a main UI Thread
    TkFunc<void()> func = [&windowHandle, &instance, &surface]() {
        VkIOSSurfaceCreateInfoMVK surfaceCreateInfo;
        VkUtils::zeroStruct(surfaceCreateInfo, VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK);
        surfaceCreateInfo.pView = windowHandle; // UIView or CAMetalLayer
        VALIDATE_VK_RESULT(vkCreateIOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, surface));
    };
    iOSPlatform::runOnUIThread(func, true);
}

} // namespace toolkit

#endif
