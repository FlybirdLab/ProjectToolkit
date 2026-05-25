/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12PipelineState.hpp"

#if GFX_API_DIRECTX12

#include "DX12ShaderProgram.hpp"
#include "DX12Texture.hpp"
#include "Dx12VertexLayout.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/memory/Memory.hpp"

namespace toolkit
{

    static D3D12_STENCIL_OP toStencilOp(StencilOperation value)
    {
        switch (value)
        {
        case StencilOperation::kKeep:
            return D3D12_STENCIL_OP_KEEP;
        case StencilOperation::kZero:
            return D3D12_STENCIL_OP_ZERO;
        case StencilOperation::kReplace:
            return D3D12_STENCIL_OP_REPLACE;
        case StencilOperation::kIncrementSaturated:
            return D3D12_STENCIL_OP_INCR_SAT;
        case StencilOperation::kDecrementSaturated:
            return D3D12_STENCIL_OP_DECR_SAT;
        case StencilOperation::kInvert:
            return D3D12_STENCIL_OP_INVERT;
        case StencilOperation::kIncrement:
            return D3D12_STENCIL_OP_INCR;
        case StencilOperation::kDecrement:
            return D3D12_STENCIL_OP_DECR;
        default:
            return D3D12_STENCIL_OP_KEEP;
        }
    }

    DX12PipelineState::DX12PipelineState(DX12Device *device)
        : DX12Resource<GfxPipelineState>(device, TkStringView())
    {
        TkMemory::memoryClear(&m_desc, sizeof(m_desc));
    }

    bool DX12PipelineState::isValid() const
    {
        return !!m_memoryUsage;
    }

    ID3D12PipelineState *DX12PipelineState::getState(DX12TextureView *depth, i32 rtCount, DX12TextureView **rtHandles, DX12VertexLayout *vertexLayout)
    {
        TK_ASSERT(depth || rtCount);

        // Prepare key
        DX12PipelineStateKey key;
        TkMemory::memoryClear(&key, sizeof(key));
        key.rtsCount = rtCount;
        key.vertexLayout = reinterpret_cast<GfxVertexLayout *>(vertexLayout);
        key.depthFormat = depth ? depth->getFormat() : PixelFormat::kUnknown;
        key.msaa = depth ? depth->getMSAA() : (rtCount ? rtHandles[0]->getMSAA() : GfxMSAALevel::kNone);
        for (i32 i = 0; i < rtCount; i++)
            key.rtvsFormats[i] = rtHandles[i]->getFormat();
        for (i32 i = rtCount; i < GFX_MAX_RT_BINDED; i++)
            key.rtvsFormats[i] = PixelFormat::kUnknown;

        // Try reuse cached version
        auto it = m_states.find(key);
        if (it.isNotEnd())
        {
            return it->value;
        }

        // Update description to match the pipeline
        m_desc.NumRenderTargets = key.rtsCount;
        for (i32 i = 0; i < GFX_MAX_RT_BINDED; i++)
            m_desc.RTVFormats[i] = DxUtils::toDxgiFormat(key.rtvsFormats[i]);
        m_desc.SampleDesc.Count = static_cast<UINT>(key.msaa);
        m_desc.SampleDesc.Quality = key.msaa == GfxMSAALevel::kNone ? 0 : DX12Device::getMaxMSAAQuality(static_cast<u32>(key.msaa));
        m_desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
        m_desc.DSVFormat = DxUtils::toDxgiFormat(GfxPixelFormat::findDepthStencilFormat(key.depthFormat));

        GfxVertexLayout *effectiveLayout = reinterpret_cast<GfxVertexLayout *>(vertexLayout);
        if (!effectiveLayout)
            effectiveLayout = reinterpret_cast<GfxVertexLayout *>(vertexBufferLayout); // Fallback to shader-specified layout (if using old APIs)
        if (effectiveLayout)
        {
            i32 missingSlotOverride = GFX_MAX_VB_BINDED; // Use additional slot with empty VB
            if (vertexInputLayout)
                effectiveLayout = GfxVertexLayout::merge(effectiveLayout, reinterpret_cast<GfxVertexLayout *>(vertexInputLayout), false, true, missingSlotOverride);
            auto *dxLayout = static_cast<GPUVertexLayoutDX12 *>(effectiveLayout);
            m_desc.InputLayout.pInputElementDescs = dxLayout->inputElements;
            m_desc.InputLayout.NumElements = dxLayout->inputElementsCount;
        }
        else
        {
            m_desc.InputLayout.pInputElementDescs = nullptr;
            m_desc.InputLayout.NumElements = 0;
        }

        // Create object
        ID3D12PipelineState *state = nullptr;
        const HRESULT result = m_device->getDevice()->CreateGraphicsPipelineState(&m_desc, IID_PPV_ARGS(&state));
        LOG_DIRECTX_RESULT(result);
        if (FAILED(result))
            return nullptr;

        // Cache it
        m_states.add(key, state);

        return state;
    }

    void DX12PipelineState::onReleaseData()
    {
        for (auto it = m_states.Begin(); it.isNotEnd(); ++it)
        {
            m_device->addResourceToLateRelease(it->value);
        }
        m_states.clear();
    }

    bool DX12PipelineState::init(const Description &desc)
    {
        TK_ASSERT(!isValid());

        // Create description
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc;
        TkMemory::memoryClear(&psDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
        psDesc.pRootSignature = m_device->getRootSignature();

        // Shaders
        TkMemory::memoryClear(&header, sizeof(header));
#define INIT_SHADER_STAGE(stage_lower, stage_upper, type)                                 \
    if (desc.stage_lower)                                                                 \
    {                                                                                     \
        auto *shader = static_cast<type *>(desc.stage_lower);                             \
        psDesc.stage_upper = {shader->bytecode.get(), (size_t)shader->bytecode.length()}; \
        const u32 srCount = TkMath::floorLog2(shader->getBindings().usedSRsMask) + 1;     \
        for (u32 i = 0; i < srCount; i++)                                                 \
            if (shader->header.srDimensions[i])                                           \
                header.srDimensions[i] = shader->header.srDimensions[i];                  \
        const u32 uaCount = TkMath::floorLog2(shader->getBindings().usedUAsMask) + 1;     \
        for (u32 i = 0; i < uaCount && i < sizeof(header.uaDimensions); i++)              \
            if (shader->header.uaDimensions[i])                                           \
                header.uaDimensions[i] = shader->header.uaDimensions[i];                  \
    }

#if GFX_ALLOW_TESSELLATION_SHADERS
        INIT_SHADER_STAGE(hs, HS, DX12ShaderProgramHS)
        INIT_SHADER_STAGE(ds, DS, DX12ShaderProgramDS)
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        INIT_SHADER_STAGE(gs, GS, DX12ShaderProgramGS)
#endif
        INIT_SHADER_STAGE(vs, VS, DX12ShaderProgramVS)
        INIT_SHADER_STAGE(ps, PS, DX12ShaderProgramPS)

#undef INIT_SHADER_STAGE

        // Input Assembly
        if (desc.vs)
        {
            vertexBufferLayout = reinterpret_cast<DX12VertexLayout *>(desc.vs->layout);
            vertexInputLayout = reinterpret_cast<DX12VertexLayout *>(desc.vs->inputLayout);
        }
        const D3D12_PRIMITIVE_TOPOLOGY_TYPE primTypes1[] =
            {
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            };
        const D3D_PRIMITIVE_TOPOLOGY primTypes2[] =
            {
                D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
                D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
                D3D_PRIMITIVE_TOPOLOGY_LINELIST,
                D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
            };
        psDesc.PrimitiveTopologyType = primTypes1[static_cast<i32>(desc.primitiveTopology)];
        primitiveTopology = primTypes2[static_cast<i32>(desc.primitiveTopology)];
#if GFX_ALLOW_TESSELLATION_SHADERS
        if (desc.hs)
        {
            psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            primitiveTopology = static_cast<D3D_PRIMITIVE_TOPOLOGY>(static_cast<i32>(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) + (desc.hs->getControlPointsCount() - 1));
        }
#endif

        // Depth State
        psDesc.DepthStencilState.DepthEnable = !!desc.depthEnable;
        psDesc.DepthStencilState.DepthWriteMask = desc.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        psDesc.DepthStencilState.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(desc.depthFunc);
        psDesc.DepthStencilState.StencilEnable = !!desc.stencilEnable;
        psDesc.DepthStencilState.StencilReadMask = desc.stencilReadMask;
        psDesc.DepthStencilState.StencilWriteMask = desc.stencilWriteMask;
        psDesc.DepthStencilState.FrontFace.StencilFailOp = toStencilOp(desc.stencilFailOp);
        psDesc.DepthStencilState.FrontFace.StencilDepthFailOp = toStencilOp(desc.stencilDepthFailOp);
        psDesc.DepthStencilState.FrontFace.StencilPassOp = toStencilOp(desc.stencilPassOp);
        psDesc.DepthStencilState.FrontFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(desc.stencilFunc);
        psDesc.DepthStencilState.BackFace = psDesc.DepthStencilState.FrontFace;

        // Rasterizer State
        psDesc.RasterizerState.FillMode = desc.wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
        D3D12_CULL_MODE dxCullMode = D3D12_CULL_MODE_BACK;
        switch (desc.cullMode)
        {
        case GfxCullMode::kNormal:
            dxCullMode = D3D12_CULL_MODE_BACK;
            break;
        case GfxCullMode::kInverted:
            dxCullMode = D3D12_CULL_MODE_FRONT;
            break;
        case GfxCullMode::kTwoSided:
            dxCullMode = D3D12_CULL_MODE_NONE;
            break;
        }
        psDesc.RasterizerState.CullMode = dxCullMode;
        psDesc.RasterizerState.FrontCounterClockwise = FALSE;
        psDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        psDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        psDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        psDesc.RasterizerState.DepthClipEnable = !!desc.depthClipEnable;
        psDesc.RasterizerState.MultisampleEnable = TRUE;
        psDesc.RasterizerState.AntialiasedLineEnable = !!desc.wireframe;
        psDesc.RasterizerState.ForcedSampleCount = 0;
        psDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // Blend State
        psDesc.BlendState.AlphaToCoverageEnable = desc.blendMode.alphaToCoverageEnable ? TRUE : FALSE;
        psDesc.BlendState.IndependentBlendEnable = FALSE;
        psDesc.BlendState.RenderTarget[0].BlendEnable = desc.blendMode.blendEnable ? TRUE : FALSE;
        psDesc.BlendState.RenderTarget[0].SrcBlend = static_cast<D3D12_BLEND>(desc.blendMode.srcBlend);
        psDesc.BlendState.RenderTarget[0].DestBlend = static_cast<D3D12_BLEND>(desc.blendMode.destBlend);
        psDesc.BlendState.RenderTarget[0].BlendOp = static_cast<D3D12_BLEND_OP>(desc.blendMode.blendOp);
        psDesc.BlendState.RenderTarget[0].SrcBlendAlpha = static_cast<D3D12_BLEND>(desc.blendMode.srcBlendAlpha);
        psDesc.BlendState.RenderTarget[0].DestBlendAlpha = static_cast<D3D12_BLEND>(desc.blendMode.destBlendAlpha);
        psDesc.BlendState.RenderTarget[0].BlendOpAlpha = static_cast<D3D12_BLEND_OP>(desc.blendMode.blendOpAlpha);
        psDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = static_cast<UINT8>(desc.blendMode.renderTargetWriteMask);
#if TK_DEBUG
        for (byte i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
            psDesc.BlendState.RenderTarget[i] = psDesc.BlendState.RenderTarget[0];
#endif

        // Cache description
        m_desc = psDesc;

        // Set non-zero memory usage
        m_memoryUsage = sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC);

        return GfxPipelineState::init(desc);
    }

}

#endif
