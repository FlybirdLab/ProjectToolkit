/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "toolkit/graphics/GfxShader.hpp"
#include "toolkit/graphics/GfxConstantBuffer.hpp"
#include "Dx12Device.hpp"

namespace toolkit
{
    // Constant Buffer for DirectX 12 backend.
    class DX12ConstantBuffer : public DX12Resource<GfxConstantBuffer>
    {
    public:
        DX12ConstantBuffer(DX12Device *device, u32 size, const TkStringView &name) noexcept
            : DX12Resource(device, name), address(0)
        {
            m_size = size;
        }

    public:
        // Last uploaded data address.
        D3D12_GPU_VIRTUAL_ADDRESS address;
    };

    // Shader for DirectX 12 backend.
    class DX12Shader : public DX12Resource<GfxShader>
    {
    public:
        DX12Shader(DX12Device *device, const TkStringView &name)
            : DX12Resource<GfxShader>(device, name)
        {
        }

    protected:
        GfxShaderProgram *createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream) override;
    };

}

#endif
