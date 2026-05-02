/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include "toolkit/types/Version.hpp"
#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "toolkit/graphics/GfxAdapter.hpp"

namespace toolkit
{
    // Graphics Device adapter implementation for Vulkan backend
    class VulkanAdapter : public GfxAdapter
    {
    public:
        VulkanAdapter()
            : device(VK_NULL_HANDLE)
        {
        }

        VulkanAdapter(const VulkanAdapter &other)
            : VulkanAdapter()
        {
            *this = other;
        }

        VulkanAdapter &operator=(const VulkanAdapter &other)
        {
            device = other.device;
            deviceProps = other.deviceProps;
            desc = other.desc;
            return *this;
        }

    public:
        // The GPU device handle.
        VkPhysicalDevice device;
        // The GPU device properties.
        VkPhysicalDeviceProperties deviceProps;
        // The GPU description.
        TkString desc;

    public:
        bool isValid() const override
        {
            return device != VK_NULL_HANDLE;
        }
        void *getNativePtr() const override
        {
            return (void *)device;
        }
        u32 getVendorId() const override
        {
            return deviceProps.vendorID;
        }
        TkString getDescription() const override
        {
            return desc;
        }
        TkVersion getDriverVersion() const override
        {
            TkVersion version(VK_VERSION_MAJOR(deviceProps.driverVersion), VK_VERSION_MINOR(deviceProps.driverVersion), VK_VERSION_PATCH(deviceProps.driverVersion));
            if (isNVIDIA())
            {
                union NvidiaDriverVersion
                {
                    struct
                    {
                        u32 tertiary : 6;
                        u32 secondary : 8;
                        u32 minor : 8;
                        u32 major : 10;
                    };
                    u32 Packed;
                } NvidiaVersion;
                NvidiaVersion.Packed = deviceProps.driverVersion;
                version = TkVersion(NvidiaVersion.major, NvidiaVersion.minor);
            }
            return version;
        }
    };

}

#endif