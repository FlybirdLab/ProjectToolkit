/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11Shader.hpp"

#if GFX_API_DIRECTX11

#include "Dx11ShaderProgram.hpp"
#include "Dx11VertexLayout.hpp"
#include "toolkit/io/MemoryStream.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    DX11ShaderProgramVS::~DX11ShaderProgramVS()
    {
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
        {
            if (it->value)
                it->value->Release();
        }
    }

    ID3D11InputLayout *DX11ShaderProgramVS::getInputLayout(DX11VertexLayout *vertexLayout)
    {
        ID3D11InputLayout *inputLayout = nullptr;
        if (!vertexLayout)
            vertexLayout = (DX11VertexLayout *)this->layout;
        const bool isContain = m_cache.containsKey(vertexLayout);
        if (isContain)
            inputLayout = m_cache[vertexLayout];
        else
        {
            if (vertexLayout && vertexLayout->inputElementsCount)
            {
                DX11VertexLayout *mergedVertexLayout = vertexLayout;
                if (!mergedVertexLayout)
                    mergedVertexLayout = (DX11VertexLayout *)this->layout; // Fallback to shader-specified layout (if using old APIs)
                i32 missingSlotOverride = GFX_MAX_VB_BINDED;               // Use additional slot with empty VB
                if (this->inputLayout)
                    mergedVertexLayout = (DX11VertexLayout *)GfxVertexLayout::merge(mergedVertexLayout, this->inputLayout, false, true, missingSlotOverride);
                LOG_DIRECTX_RESULT(vertexLayout->getDevice()->getDevice()->CreateInputLayout(mergedVertexLayout->inputElements, mergedVertexLayout->inputElementsCount, bytecode.get(), bytecode.length(), &inputLayout));
            }
            m_cache.add(vertexLayout, inputLayout);
        }
        return inputLayout;
    }

    GfxShaderProgram *DX11Shader::createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream)
    {
        GfxShaderProgram *shader = nullptr;
        HRESULT result;
        switch (type)
        {
        case ShaderStage::kVertex:
        {
            GfxVertexLayout *inputLayout = nullptr;
            GfxVertexLayout *vertexLayout = nullptr;
            readVertexLayout(stream, inputLayout, vertexLayout);
            ID3D11VertexShader *buffer = nullptr;
            result = m_device->getDevice()->CreateVertexShader(bytecode.get(), bytecode.length(), nullptr, &buffer);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, nullptr);
            shader = TkMemory::newObject<DX11ShaderProgramVS>(TkMemory::getDefault(), initializer, buffer, inputLayout, vertexLayout, bytecode);
            break;
        }
#if GFX_ALLOW_TESSELLATION_SHADERS
        case ShaderStage::kHull:
        {
            i32 controlPointsCount;
            stream.readI32(&controlPointsCount);
            ID3D11HullShader *buffer = nullptr;
            result = m_device->getDevice()->CreateHullShader(bytecode.get(), bytecode.length(), nullptr, &buffer);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, nullptr);
            shader = TkMemory::newObject<DX11ShaderProgramHS>(TkMemory::getDefault(), initializer, buffer, controlPointsCount);
            break;
        }
        case ShaderStage::kDomain:
        {
            ID3D11DomainShader *buffer = nullptr;
            result = m_device->getDevice()->CreateDomainShader(bytecode.get(), bytecode.length(), nullptr, &buffer);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, nullptr);
            shader = TkMemory::newObject<DX11ShaderProgramDS>(TkMemory::getDefault(), initializer, buffer);
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
            ID3D11GeometryShader *buffer = nullptr;
            result = m_device->getDevice()->CreateGeometryShader(bytecode.get(), bytecode.length(), nullptr, &buffer);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, nullptr);
            shader = TkMemory::newObject<DX11ShaderProgramGS>(TkMemory::getDefault(), initializer, buffer);
            break;
        }
#endif
        case ShaderStage::kPixel:
        {
            ID3D11PixelShader *buffer = nullptr;
            result = m_device->getDevice()->CreatePixelShader(bytecode.get(), bytecode.length(), nullptr, &buffer);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, nullptr);
            shader = TkMemory::newObject<DX11ShaderProgramPS>(TkMemory::getDefault(), initializer, buffer);
            break;
        }
        case ShaderStage::kCompute:
        {
            ID3D11ComputeShader *buffer = nullptr;
            result = m_device->getDevice()->CreateComputeShader(bytecode.get(), bytecode.length(), nullptr, &buffer);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, nullptr);
            shader = TkMemory::newObject<DX11ShaderProgramCS>(TkMemory::getDefault(), initializer, buffer);
            break;
        }
        }
        return shader;
    }

    void DX11Shader::onReleaseData()
    {
        GfxShader::onReleaseData();
    }

}

#endif
