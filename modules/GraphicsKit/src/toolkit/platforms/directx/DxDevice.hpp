/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxDevice.hpp>

#if GFX_API_DIRECTX11 || GFX_API_DIRECTX12

#include "DxAdapter.hpp"
#include <toolkit/platforms/System.hpp>
#include "DxUtils.hpp"

namespace toolkit
{
    struct VideoOutputDX
    {
    public:
        ComPtr<IDXGIOutput> output;
        u32 refreshRateNumerator;
        u32 refreshRateDenominator;
        DXGI_OUTPUT_DESC desc;
        DXGI_MODE_DESC desktopViewMode;
        TkArray<DXGI_MODE_DESC> videoModes;
    };

    // Base for all DirectX graphics devices.
    class DirectXDevice : public GfxDevice
    {
    protected:
        DirectXDevice(GfxBackendType type, GfxShaderProfile profile, DirectXAdapter *adapter)
            : GfxDevice(type, profile), m_adapter(adapter)
        {
            adapter->getDriverVersion();
        }

    public:
        // The video outputs.
        TkArray<VideoOutputDX> outputs;

    protected:
        void updateOutputs(IDXGIAdapter *adapter);

        static GfxBackendType getRendererType(DirectXAdapter *adapter)
        {
            switch (adapter->maxFeatureLevel)
            {
            case D3D_FEATURE_LEVEL_10_0:
                return GfxBackendType::kDirectX10;
            case D3D_FEATURE_LEVEL_10_1:
                return GfxBackendType::kDirectX10_1;
            case D3D_FEATURE_LEVEL_11_0:
            case D3D_FEATURE_LEVEL_11_1:
                return GfxBackendType::kDirectX11;
#if GFX_API_DIRECTX12
            case D3D_FEATURE_LEVEL_12_0:
            case D3D_FEATURE_LEVEL_12_1:
                return GfxBackendType::kDirectX12;
#endif
            default:
                return GfxBackendType::kUnknown;
            }
        }

        static GfxShaderProfile getGfxShaderProfile(DirectXAdapter *adapter)
        {
            switch (adapter->maxFeatureLevel)
            {
            case D3D_FEATURE_LEVEL_10_0:
            case D3D_FEATURE_LEVEL_10_1:
                return GfxShaderProfile::kDirectX_SM4;
            case D3D_FEATURE_LEVEL_11_0:
            case D3D_FEATURE_LEVEL_11_1:
                return GfxShaderProfile::kDirectX_SM5;
#if GFX_API_DIRECTX12
            case D3D_FEATURE_LEVEL_12_0:
            case D3D_FEATURE_LEVEL_12_1:
                return GfxShaderProfile::kDirectX_SM5;
#endif
            default:
                return GfxShaderProfile::kUnknown;
            }
        }

    public:
        GfxAdapter *getAdapter() const override
        {
            return m_adapter;
        }

    protected:
        bool init() override
        {
            const DXGI_ADAPTER_DESC &adapterDesc = m_adapter->description;
            const u64 dedicatedVideoMemory = static_cast<u64>(adapterDesc.DedicatedVideoMemory);
            const u64 dedicatedSystemMemory = static_cast<u64>(adapterDesc.DedicatedSystemMemory);
            const u64 sharedSystemMemory = static_cast<u64>(adapterDesc.SharedSystemMemory);

            // Calculate total GPU memory
            const u64 totalPhysicalMemory = TkMath::min(System::getMemoryStats().physicalMemSize, 16Ull * 1024Ull * 1024Ull * 1024Ull);
            const u64 totalSystemMemory = TkMath::min(sharedSystemMemory / 2Ull, totalPhysicalMemory / 4Ull);
            totalMemory = 0;
            if (m_adapter->isIntel())
            {
                totalMemory = dedicatedVideoMemory + dedicatedSystemMemory + totalSystemMemory;
            }
            else if (dedicatedVideoMemory >= 200 * 1024 * 1024)
            {
                totalMemory = dedicatedVideoMemory;
            }
            else if (dedicatedSystemMemory >= 200 * 1024 * 1024)
            {
                totalMemory = dedicatedSystemMemory;
            }
            else if (sharedSystemMemory >= 400 * 1024 * 1024)
            {
                totalMemory = totalSystemMemory;
            }
            else
            {
                totalMemory = totalPhysicalMemory / 4Ull;
            }

            // Base
            return GfxDevice::init();
        }

        void dispose() override
        {
            outputs.resize(0);

            GfxDevice::dispose();
        }

    protected:
        DirectXAdapter *m_adapter;
    };

}

#endif
