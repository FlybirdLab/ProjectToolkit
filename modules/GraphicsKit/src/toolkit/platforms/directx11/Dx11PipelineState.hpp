/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/graphics/GfxPipelineState.hpp"
#include "Dx11ShaderProgram.hpp"
#include "Dx11Device.hpp"

namespace toolkit
{
    // Graphics pipeline state object for DirectX 11 backend.
    class DX11PipelineState : public DX11Resource<GfxPipelineState>
    {
    public:
        i32 rasterizerStateIndex;
        ID3D11DepthStencilState *depthStencilState = nullptr;
        ID3D11BlendState *blendState = nullptr;
        DX11ShaderProgramVS *vs = nullptr;
#if GFX_ALLOW_TESSELLATION_SHADERS
        DX11ShaderProgramHS *hs = nullptr;
        DX11ShaderProgramDS *ds = nullptr;
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        DX11ShaderProgramGS *gs = nullptr;
#endif
        DX11ShaderProgramPS *ps = nullptr;
        D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;

    public:
        DX11PipelineState(DX11Device *device)
            : DX11Resource<GfxPipelineState>(device, TkStringView::Empty)
        {
        }

    public:
        ID3D11Resource *getResource() final override;

        bool isValid() const override;
        bool init(const Description &desc) override;

    protected:
        void onReleaseData() final override;
    };
}

#endif
