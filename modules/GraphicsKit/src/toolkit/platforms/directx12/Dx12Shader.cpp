/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12Shader.hpp"

#if GFX_API_DIRECTX12

#include "toolkit/io/MemoryStream.hpp"
#include "toolkit/memory/Memory.hpp"
#include "Dx12ShaderProgram.hpp"

namespace toolkit
{

    GfxShaderProgram *DX12Shader::createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream)
    {
        // Extract the DX shader header from the cache
        DxShaderHeader *header = (DxShaderHeader *)bytecode.get();
        bytecode = bytecode.slice(sizeof(DxShaderHeader));

        GfxShaderProgram *shader = nullptr;
        switch (type)
        {
        case ShaderStage::kVertex:
        {
            GfxVertexLayout *inputLayout, *vertexLayout;
            readVertexLayout(stream, inputLayout, vertexLayout);
            shader = TkMemory::newObject<DX12ShaderProgramVS>(TkMemory::getDefault(), initializer, header, bytecode, inputLayout, vertexLayout);
            break;
        }
#if GFX_ALLOW_TESSELLATION_SHADERS
        case ShaderStage::kHull:
        {
            i32 controlPointsCount;
            stream.readI32(&controlPointsCount);
            shader = TkMemory::newObject<DX12ShaderProgramHS>(TkMemory::getDefault(), initializer, header, bytecode, controlPointsCount);
            break;
        }
        case ShaderStage::kDomain:
        {
            shader = TkMemory::newObject<DX12ShaderProgramDS>(TkMemory::getDefault(), initializer, header, bytecode);
            break;
        }
#else
        case ShaderStage::kHull:
        {
            i32 controlPointsCount;
            stream.readI32(&controlPointsCount);
            break;
        }
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        case ShaderStage::kGeometry:
        {
            shader = TkMemory::newObject<DX12ShaderProgramGS>(TkMemory::getDefault(), initializer, header, bytecode);
            break;
        }
#endif
        case ShaderStage::kPixel:
        {
            shader = TkMemory::newObject<DX12ShaderProgramPS>(TkMemory::getDefault(), initializer, header, bytecode);
            break;
        }
        case ShaderStage::kCompute:
        {
            shader = TkMemory::newObject<DX12ShaderProgramCS>(TkMemory::getDefault(), m_device, initializer, header, bytecode);
            break;
        }
        }
        return shader;
    }

    ID3D12PipelineState *DX12ShaderProgramCS::getOrCreateState()
    {
        if (m_state)
            return m_state;

        // Create description
        D3D12_COMPUTE_PIPELINE_STATE_DESC psDesc;
        TkMemory::memoryClear(&psDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
        psDesc.pRootSignature = m_device->getRootSignature();

        // Shader
        psDesc.CS = {getBufferHandle(), getBufferSize()};

        // Create object
        const HRESULT result = m_device->getDevice()->CreateComputePipelineState(&psDesc, IID_PPV_ARGS(&m_state));
        LOG_DIRECTX_RESULT(result);

        return m_state;
    }
}

#endif
