/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/graphics/GfxSampler.hpp"
#include "Dx11Device.hpp"

namespace toolkit
{
    // Sampler object for DirectX 11 backend.
    class DX11Sampler : public GfxResourceBase<DX11Device, GfxSampler>
    {
    public:
        DX11Sampler(DX11Device *device)
            : GfxResourceBase<DX11Device, GfxSampler>(device, TkStringView::Empty)
        {
        }

    public:
        ID3D11SamplerState *samplerState = nullptr;

    protected:
        bool onInit() override;
        void onReleaseData() override;
    };

}

#endif
