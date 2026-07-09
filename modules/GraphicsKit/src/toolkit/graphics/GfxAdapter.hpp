/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>

namespace toolkit
{

// GPU vendors IDs
#define GPU_VENDOR_ID_AMD 0x1002
#define GPU_VENDOR_ID_INTEL 0x8086
#define GPU_VENDOR_ID_NVIDIA 0x10DE
#define GPU_VENDOR_ID_MICROSOFT 0x1414
#define GPU_VENDOR_ID_APPLE 0x106B

    // <summary>
    // Interface for GPU device adapter.
    class TK_API GfxAdapter
    {
    public:
        GfxAdapter() = default;
        GfxAdapter(const GfxAdapter &other)
            : GfxAdapter{}
        {
            *this = other;
        }

        GfxAdapter &operator=(const GfxAdapter &other)
        {
            return *this;
        }

    public:
        // Check if adapter is valid and returns true if it is.
        virtual bool isValid() const = 0;
        // Get the native pointer to the underlying graphics device adapter. It's a low-level platform-specific handle.
        virtual void *getNativePtr() const = 0;
        // Get the GPU vendor identifier.
        virtual u32 getVendorId() const = 0;
        // Get a string that contains the adapter description. Used for presentation to the user.
        virtual TkString getDescription() const = 0;
        // Get the GPU driver version.
        virtual TkVersion getDriverVersion() const = 0;

    public:
        // Returns true if adapter's vendor is AMD.
        TK_FORCE_INLINE bool isAMD() const
        {
            return getVendorId() == GPU_VENDOR_ID_AMD;
        }

        // Returns true if adapter's vendor is Intel.
        TK_FORCE_INLINE bool isIntel() const
        {
            return getVendorId() == GPU_VENDOR_ID_INTEL;
        }

        // Returns true if adapter's vendor is Nvidia.
        TK_FORCE_INLINE bool isNVIDIA() const
        {
            return getVendorId() == GPU_VENDOR_ID_NVIDIA;
        }

        // Returns true if adapter's vendor is Microsoft.
        TK_FORCE_INLINE bool isMicrosoft() const
        {
            return getVendorId() == GPU_VENDOR_ID_MICROSOFT;
        }
    };

}