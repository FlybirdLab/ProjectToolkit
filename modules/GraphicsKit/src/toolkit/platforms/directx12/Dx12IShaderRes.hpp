/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "DX12ResOwner.hpp"

#if GFX_API_DIRECTX12

namespace toolkit
{
    // Interface for objects that can be bound to the shader slots in DirectX 12.
    class DX12IShaderResource
    {
    public:
        DX12IShaderResource()
            : subResIdx(-1)
        {
        }

        DX12IShaderResource(i32 subresourceIndex)
            : subResIdx(subresourceIndex)
        {
        }

    public:
        // Affected subresource index or -1 if use whole resource.
        i32 subResIdx;

        D3D12_SRV_DIMENSION srvDimension = D3D12_SRV_DIMENSION_UNKNOWN;
        D3D12_UAV_DIMENSION uavDimension = D3D12_UAV_DIMENSION_UNKNOWN;

    public:
        // Determine whether this resource is depth/stencil buffer.
        virtual bool isDepthStencilResource() const = 0;
        // Get CPU handle to the shader resource view descriptor.
        virtual D3D12_CPU_DESCRIPTOR_HANDLE srv() const = 0;
        // Get CPU handle to the unordered access view descriptor.
        virtual D3D12_CPU_DESCRIPTOR_HANDLE uav() const = 0;
        // Get the resource owner.
        virtual DX12ResourceOwner *getResourceOwner() const = 0;
    };
}

#endif
