/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "toolkit/types/Span.hpp"
#include "Dx12Device.hpp"
#include "toolkit/collection/DataCollection.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"

namespace toolkit
{
    // Shader base class for DirectX 12 backend.
    template <typename BaseType>
    class DX12ShaderProgram : public BaseType
    {
    public:
        DX12ShaderProgram(const GfxShaderProgramInitializer &initializer, const DxShaderHeader *header, TkSpan<byte> bytecode)
            : header(*header)
        {
            BaseType::init(initializer);
            this->bytecode.copy(bytecode);
        }

    public:
        ByteCollection bytecode;
        DxShaderHeader header;

    public:
        void *getBufferHandle() const override
        {
            return (void *)bytecode.get();
        }
        u32 getBufferSize() const override
        {
            return bytecode.length();
        }
    };

    // Vertex Shader for DirectX 12 backend.
    class DX12ShaderProgramVS : public DX12ShaderProgram<GfxShaderProgramVS>
    {
    public:
        DX12ShaderProgramVS(const GfxShaderProgramInitializer &initializer, const DxShaderHeader *header, TkSpan<byte> bytecode, GfxVertexLayout *inputLayout, GfxVertexLayout *vertexLayout)
            : DX12ShaderProgram(initializer, header, bytecode)
        {
            this->inputLayout = inputLayout;
            layout = vertexLayout;
        }
    };

#if GFX_ALLOW_TESSELLATION_SHADERS
    // Hull Shader for DirectX 12 backend.
    class DX12ShaderProgramHS : public DX12ShaderProgram<GfxShaderProgramHS>
    {
    public:
        DX12ShaderProgramHS(const GfxShaderProgramInitializer &initializer, const DxShaderHeader *header, TkSpan<byte> bytecode, i32 controlPointsCount)
            : DX12ShaderProgram(initializer, header, bytecode)
        {
            m_controlPointsCount = controlPointsCount;
        }
    };

    // Domain Shader for DirectX 12 backend.
    class DX12ShaderProgramDS : public DX12ShaderProgram<GfxShaderProgramDS>
    {
    public:
        DX12ShaderProgramDS(const GfxShaderProgramInitializer &initializer, const DxShaderHeader *header, TkSpan<byte> bytecode)
            : DX12ShaderProgram(initializer, header, bytecode)
        {
        }
    };
#endif

#if GFX_ALLOW_GEOMETRY_SHADERS
    // Geometry Shader for DirectX 12 backend.
    class DX12ShaderProgramGS : public DX12ShaderProgram<GfxShaderProgramGS>
    {
    public:
        DX12ShaderProgramGS(const GfxShaderProgramInitializer &initializer, const DxShaderHeader *header, TkSpan<byte> bytecode)
            : DX12ShaderProgram(initializer, header, bytecode)
        {
        }
    };
#endif

    // Pixel Shader for DirectX 12 backend.
    class DX12ShaderProgramPS : public DX12ShaderProgram<GfxShaderProgramPS>
    {
    public:
        DX12ShaderProgramPS(const GfxShaderProgramInitializer &initializer, const DxShaderHeader *header, TkSpan<byte> bytecode)
            : DX12ShaderProgram(initializer, header, bytecode)
        {
        }
    };

    // Compute Shader for DirectX 12 backend.
    class DX12ShaderProgramCS : public DX12ShaderProgram<GfxShaderProgramCS>
    {
    public:
        DX12ShaderProgramCS(DX12Device *device, const GfxShaderProgramInitializer &initializer, const DxShaderHeader *header, TkSpan<byte> bytecode)
            : DX12ShaderProgram(initializer, header, bytecode), m_device(device), m_state(nullptr)
        {
        }

        ~DX12ShaderProgramCS()
        {
            m_device->addResourceToLateRelease(m_state);
        }

    public:
        // Get DirectX 12 compute pipeline state object
        TK_FORCE_INLINE ID3D12PipelineState *getState() const
        {
            return m_state;
        }

        // Get or creates compute pipeline state for that compute shader.
        ID3D12PipelineState *getOrCreateState();

    private:
        DX12Device *m_device;
        TkArray<byte> m_data;
        ID3D12PipelineState *m_state;
    };

}

#endif
