/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx/common.hpp>

#if GFX_API_DIRECTX11 || GFX_API_DIRECTX12

#include <toolkit/graphics/GfxAdapter.hpp>
#include "toolkit/types/Version.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/utils/Convertor.hpp"

namespace toolkit
{
    // Graphics Device adapter implementation for DirectX backend.
    class DirectXAdapter : public GfxAdapter
    {
    public:
        void getDriverVersion();

        bool isValid() const override
        {
            return index != -1 && maxFeatureLevel != static_cast<D3D_FEATURE_LEVEL>(0);
        }
        void *getNativePtr() const override
        {
            return (void *)(intptr)index;
        }
        u32 getVendorId() const override
        {
            return (u32)description.VendorId;
        }
        TkString getDescription() const override
        {
            return StringAsUTF8<>(description.Description).get();
        }
        TkVersion getDriverVersion() const override
        {
            return driverVersion;
        }

    public:
        i32 index = -1;
        D3D_FEATURE_LEVEL maxFeatureLevel;
        DXGI_ADAPTER_DESC description;
        TkVersion driverVersion = TkVersion(0, 0);

    private:
        void setDriverVersion(TkVersion &ver);
    };
}

#endif
