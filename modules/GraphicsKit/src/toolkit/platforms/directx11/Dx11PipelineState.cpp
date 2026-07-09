/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11PipelineState.hpp"

#if GFX_API_DIRECTX11

namespace toolkit
{

    void DX11PipelineState::onReleaseData()
    {
        depthStencilState = nullptr;
        blendState = nullptr;
        vs = nullptr;
#if GFX_ALLOW_TESSELLATION_SHADERS
        hs = nullptr;
        ds = nullptr;
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        gs = nullptr;
#endif
        ps = nullptr;
    }

    ID3D11Resource *DX11PipelineState::getResource()
    {
        return nullptr;
    }

    bool DX11PipelineState::isValid() const
    {
        return !!m_memoryUsage;
    }

    bool DX11PipelineState::init(const Description &desc)
    {
        TK_ASSERT(!isValid());

        // Cache shaders
        vs = (DX11ShaderProgramVS *)desc.vs;
#if GFX_ALLOW_TESSELLATION_SHADERS
        hs = desc.hs ? (DX11ShaderProgramHS *)desc.hs : nullptr;
        ds = desc.ds ? (DX11ShaderProgramDS *)desc.ds : nullptr;
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        gs = desc.gs ? (DX11ShaderProgramGS *)desc.gs : nullptr;
#endif
        ps = desc.ps ? (DX11ShaderProgramPS *)desc.ps : nullptr;

        // Primitive Topology
        const static D3D11_PRIMITIVE_TOPOLOGY D3D11_primTypes[] =
            {
                D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
                D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
                D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
                D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
            };
        primitiveTopology = D3D11_primTypes[static_cast<i32>(desc.primitiveTopology)];
#if GFX_ALLOW_TESSELLATION_SHADERS
        if (hs)
            primitiveTopology = (D3D11_PRIMITIVE_TOPOLOGY)((i32)D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (hs->getControlPointsCount() - 1));
#endif

        // States
        rasterizerStateIndex = static_cast<i32>(desc.cullMode) + (desc.wireframe ? 0 : 3) + (desc.depthClipEnable ? 0 : 6);
        depthStencilState = m_device->getDepthStencilState(&desc);
        blendState = m_device->getBlendState(desc.blendMode);

        // Calculate approx. memory usage (just to set sth)
        m_memoryUsage = sizeof(D3D11_DEPTH_STENCIL_DESC) + sizeof(D3D11_RASTERIZER_DESC) + sizeof(D3D11_BLEND_DESC);

        return GfxPipelineState::init(desc);
    }

}

#endif
