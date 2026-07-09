/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx12/common.hpp>

#if GFX_API_DIRECTX12

#include "toolkit/graphics/GfxSampler.hpp"
#include "Dx12Device.hpp"

namespace toolkit
{
    // Sampler object for DirectX 12 backend.
    class Dx12Sampler : public DX12Resource<GfxSampler>
    {
    public:
        Dx12Sampler(DX12Device *device)
            : DX12Resource<GfxSampler>(device, TkStringView::Empty)
        {
        }

    public:
        DX12DescriptorHeapWithSlots::Slot slot;
        D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;

    protected:
        bool onInit() override;
        void onReleaseData() override;
    };

}

#endif
