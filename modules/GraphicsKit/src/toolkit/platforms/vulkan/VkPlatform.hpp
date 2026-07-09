/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

namespace toolkit
{

    enum class VulkanValidationLevel
    {
        Disabled = 0,
        ErrorsOnly = 1,
        ErrorsAndWarnings = 2,
        ErrorsAndWarningsPerf = 3,
        ErrorsAndWarningsPerfInfo = 4,
        All = 5,
    };

    // Vulkan API platform support.
    class VulkanPlatformBase
    {
    public:
        static void getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers);
        static void getDeviceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers)
        {
        }
        static void createSurface(VkSurfaceKHR *outSurface);
        static void restrictEnabledPhysicalDeviceFeatures(const VkPhysicalDeviceFeatures &deviceFeatures, VkPhysicalDeviceFeatures &featuresToEnable)
        {
            featuresToEnable = deviceFeatures;
            featuresToEnable.shaderResourceResidency = VK_FALSE;
            featuresToEnable.shaderResourceMinLod = VK_FALSE;
            featuresToEnable.sparseBinding = VK_FALSE;
            featuresToEnable.sparseResidencyBuffer = VK_FALSE;
            featuresToEnable.sparseResidencyImage2D = VK_FALSE;
            featuresToEnable.sparseResidencyImage3D = VK_FALSE;
            featuresToEnable.sparseResidency2Samples = VK_FALSE;
            featuresToEnable.sparseResidency4Samples = VK_FALSE;
            featuresToEnable.sparseResidency8Samples = VK_FALSE;
            featuresToEnable.sparseResidencyAliased = VK_FALSE;
        }
    };

}

#if TK_WINDOWS
#include <toolkit/platforms/vulkan/windows/Win32Platform.hpp>
#elif TK_MACOS
#include <toolkit/platforms/vulkan/macos/MacPlatform.h>
#elif TK_IOS
#include <toolkit/platforms/vulkan/ios/iOSPlatform.h>
#elif TK_LINUX
#include <toolkit/platforms/vulkan/linux/LinuxPlatform.hpp>
#elif TK_ANDROID
#include <toolkit/platforms/vulkan/android/AndroidPlatform.hpp>
#endif

#endif
