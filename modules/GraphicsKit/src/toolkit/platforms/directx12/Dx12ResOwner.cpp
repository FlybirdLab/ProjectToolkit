/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12ResOwner.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Device.hpp"

namespace toolkit
{

    void DX12ResourceOwner::initResource(ID3D12Resource *resource, const D3D12_RESOURCE_STATES initialState, const u32 subresourceCount, bool usePerSubresourceTracking)
    {
        TK_ASSERT(resource);
        m_resource = resource;
        m_subresourcesCount = subresourceCount;
        state.init(subresourceCount, initialState, usePerSubresourceTracking);
    }

    void DX12ResourceOwner::initResource(const D3D12_RESOURCE_STATES initialState, const u32 subresourceCount, bool usePerSubresourceTracking)
    {
        // Note: this is used by the dynamic buffers (which don't own resource but just part of other one)
        m_subresourcesCount = subresourceCount;
        state.init(subresourceCount, initialState, usePerSubresourceTracking);
    }

    void DX12ResourceOwner::releaseResource(u32 safeFrameCount)
    {
        if (m_resource)
        {
            onRelease(this);

            auto resource = m_resource;
            m_resource = nullptr;
            m_subresourcesCount = 0;
            state.release();

            ((DX12Device *)GfxDevice::sInstance)->addResourceToLateRelease(resource, safeFrameCount);
        }
    }

}

#endif
