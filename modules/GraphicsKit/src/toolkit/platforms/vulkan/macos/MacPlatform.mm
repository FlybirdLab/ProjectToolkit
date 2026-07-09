/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/vulkan/VkPlatform.hpp>

#if GFX_API_VULKAN && TK_MACOS

#include "MacPlatform.h"
#include "../VkUtils.hpp"
#import <Cocoa/Cocoa.h>

namespace toolkit
{

void MacVulkanPlatform::getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers)
{
    extensions.add(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.add(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
}

void MacVulkanPlatform::createSurface(void *windowHandle, VkInstance instance, VkSurfaceKHR *surface)
{
    NSWindow *window = (NSWindow *)windowHandle;
    VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
    VkUtils::zeroStruct(surfaceCreateInfo, VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK);
    surfaceCreateInfo.pView = (void *)window.contentView;
    VALIDATE_VK_RESULT(vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, surface));
}

} // namespace toolkit

#endif
