/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx11/common.hpp>

#if GFX_API_DIRECTX11

#include "toolkit/platforms/directx/DxUtils.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"
#include "toolkit/collection/DataCollection.hpp"
#include "toolkit/collection/HashMap.hpp"

namespace toolkit
{
    // Shaders base class for DirectX 11 backend.
    template <typename BaseType, typename BufferType>
    class DX11ShaderProgram : public BaseType
    {
    public:
        DX11ShaderProgram(const GfxShaderProgramInitializer &initializer, BufferType *buffer)
            : m_buffer(buffer)
        {
            BaseType::init(initializer);
#if GFX_ENABLE_RESOURCE_NAMING
            setDebugObjectName(buffer, initializer.name);
#endif
        }

        ~DX11ShaderProgram()
        {
            DX_DEL(m_buffer, 0);
        }

    public:
        TK_FORCE_INLINE BufferType *getBufferHandleDX11() const
        {
            return m_buffer;
        }

        u32 getBufferSize() const override
        {
            return 0;
        }
        void *getBufferHandle() const override
        {
            return m_buffer;
        }

    protected:
        BufferType *m_buffer;
    };

    // Vertex Shader for DirectX 11 backend.
    class DX11ShaderProgramVS : public DX11ShaderProgram<GfxShaderProgramVS, ID3D11VertexShader>
    {
    public:
        ByteCollection bytecode;

    public:
        DX11ShaderProgramVS(const GfxShaderProgramInitializer &initializer, ID3D11VertexShader *buffer, GfxVertexLayout *inputLayout, GfxVertexLayout *vertexLayout, TkSpan<byte> bytecode)
            : DX11ShaderProgram(initializer, buffer)
        {
            this->inputLayout = inputLayout;
            this->layout = vertexLayout;
            this->bytecode.copy(bytecode);
        }
        ~DX11ShaderProgramVS();

    public:
        ID3D11InputLayout *getInputLayout(class DX11VertexLayout *vertexLayout);

    private:
        TkHashMap<class DX11VertexLayout *, ID3D11InputLayout *> m_cache;
    };

#if GFX_ALLOW_TESSELLATION_SHADERS
    // Hull Shader for DirectX 11 backend.
    class DX11ShaderProgramHS : public DX11ShaderProgram<GfxShaderProgramHS, ID3D11HullShader>
    {
    public:
        DX11ShaderProgramHS(const GfxShaderProgramInitializer &initializer, ID3D11HullShader *buffer, i32 controlPointsCount)
            : DX11ShaderProgram(initializer, buffer)
        {
            m_controlPointsCount = controlPointsCount;
        }
    };

    // Domain Shader for DirectX 11 backend.
    class DX11ShaderProgramDS : public DX11ShaderProgram<GfxShaderProgramDS, ID3D11DomainShader>
    {
    public:
        DX11ShaderProgramDS(const GfxShaderProgramInitializer &initializer, ID3D11DomainShader *buffer)
            : DX11ShaderProgram(initializer, buffer)
        {
        }
    };
#endif

#if GFX_ALLOW_GEOMETRY_SHADERS
    // Geometry Shader for DirectX 11 backend.
    class DX11ShaderProgramGS : public DX11ShaderProgram<GfxShaderProgramGS, ID3D11GeometryShader>
    {
    public:
        DX11ShaderProgramGS(const GfxShaderProgramInitializer &initializer, ID3D11GeometryShader *buffer)
            : DX11ShaderProgram(initializer, buffer)
        {
        }
    };
#endif

    // Pixel Shader for DirectX 11 backend.
    class DX11ShaderProgramPS : public DX11ShaderProgram<GfxShaderProgramPS, ID3D11PixelShader>
    {
    public:
        DX11ShaderProgramPS(const GfxShaderProgramInitializer &initializer, ID3D11PixelShader *buffer)
            : DX11ShaderProgram(initializer, buffer)
        {
        }
    };

    // Compute Shader for DirectX 11 backend.
    class DX11ShaderProgramCS : public DX11ShaderProgram<GfxShaderProgramCS, ID3D11ComputeShader>
    {
    public:
        DX11ShaderProgramCS(const GfxShaderProgramInitializer &initializer, ID3D11ComputeShader *buffer)
            : DX11ShaderProgram(initializer, buffer)
        {
        }
    };

}

#endif
