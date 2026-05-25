/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX11

namespace toolkit
{
    // Interface for objects that can be bound to the shader slots in DirectX 11.
    class IShaderResourceDX11
    {
    public:
        IShaderResourceDX11() = default;

    public:
        // Get handle to the shader resource view object.
        virtual ID3D11ShaderResourceView *srv() const = 0;
        // Get CPU to the unordered access view object.
        virtual ID3D11UnorderedAccessView *uav() const = 0;
    };

}

#endif
