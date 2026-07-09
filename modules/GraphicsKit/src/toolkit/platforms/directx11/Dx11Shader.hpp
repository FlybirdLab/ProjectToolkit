/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/graphics/GfxShader.hpp"
#include "toolkit/graphics/GfxConstantBuffer.hpp"
#include "Dx11Device.hpp"

namespace toolkit
{
    // Constant Buffer for DirectX 11 backend.
    class DX11ConstantBuffer : public DX11Resource<GfxConstantBuffer>
    {
    private:
        ID3D11Buffer *m_resource;

    public:
        DX11ConstantBuffer(DX11Device *device, u32 size, u32 memorySize, ID3D11Buffer *buffer, const TkStringView &name) noexcept
            : DX11Resource(device, name), m_resource(buffer)
        {
            m_size = size;
            m_memoryUsage = memorySize;
        }

        ~DX11ConstantBuffer()
        {
            DX_DEL(m_resource, 0);
            m_memoryUsage = 0;
        }

    public:
        // Get the constant buffer object.
        TK_FORCE_INLINE ID3D11Buffer *getBuffer() const
        {
            return m_resource;
        }

        ID3D11Resource *getResource() override
        {
            return m_resource;
        }

        void onReleaseData() final override
        {
            DX_DEL(m_resource, 0);
        }
    };

    // Shader for DirectX 11 backend.
    class DX11Shader : public DX11Resource<GfxShader>
    {
    public:
        DX11Shader(DX11Device *device, const TkStringView &name)
            : DX11Resource(device, name)
        {
        }

    public:
        ID3D11Resource *getResource() final override
        {
            return nullptr;
        }

    protected:
        GfxShaderProgram *createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream) override;
        void onReleaseData() override;
    };

}

#endif
