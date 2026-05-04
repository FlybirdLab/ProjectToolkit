/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Win32Platform.hpp"

#if GFX_API_VULKAN && TK_WINDOWS

#include "toolkit/collection/Array.hpp"
#include "../VkUtils.hpp"

#include <vulkan/vulkan_win32.h>

namespace toolkit
{
    VKAPI_ATTR VkResult VKAPI_CALL vkCreateWin32SurfaceKHR(
        VkInstance instance,
        const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkSurfaceKHR *pSurface);

    void Win32VulkanPlatform::getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers)
    {
        extensions.add(VK_KHR_SURFACE_EXTENSION_NAME);
        extensions.add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }

    void Win32VulkanPlatform::createSurface(void *windowHandle, VkInstance instance, VkSurfaceKHR *surface)
    {
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
        VkUtils::zeroStruct(surfaceCreateInfo, VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR);
        surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
        surfaceCreateInfo.hwnd = static_cast<HWND>(windowHandle);
        VALIDATE_VK_RESULT(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, surface));
    }
}

#endif
