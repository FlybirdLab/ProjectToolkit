/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "toolkit/base/Function.hpp"
#include "toolkit/graphics/GfxResource.hpp"

namespace toolkit
{
    class GfxResource;
    class DX12Context;
    class DX12AsyncContext;

// Default amount of frames to wait until resource delete.
#define DX12_RESOURCE_DELETE_SAFE_FRAMES_COUNT 100
// Custom resource state used to indicate invalid state (useful for debugging resource tracking issues).
#define D3D12_RESOURCE_STATE_CORRUPT (D3D12_RESOURCE_STATES) - 1

    // Tracking of per-resource or per-subresource state for D3D12 resources that require to issue resource access barriers during rendering.
    class DX12ResourceState : public GfxResourceState<D3D12_RESOURCE_STATES, D3D12_RESOURCE_STATE_CORRUPT>
    {
    public:
        // Returns true if resource state transition is needed in order to use resource in given state.
        TK_FORCE_INLINE static bool isTransitionNeeded(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES &after)
        {
            if (before == D3D12_RESOURCE_STATE_DEPTH_WRITE && after == D3D12_RESOURCE_STATE_DEPTH_READ)
                return false;
            if (after == D3D12_RESOURCE_STATE_COMMON)
                return before != D3D12_RESOURCE_STATE_COMMON;
            if (after == D3D12_RESOURCE_STATE_DEPTH_READ)
                return ~(before & (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)) == 0;
            const D3D12_RESOURCE_STATES combined = before | after;
            if ((combined & (D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)) == combined)
                after = combined;
            return before != after;
        }
    };

    // Base class for objects in DirectX 12 layer that can own a resource
    class DX12ResourceOwner
    {
        friend DX12Context;
        friend DX12AsyncContext;

    protected:
        ID3D12Resource *m_resource;
        u32 m_subresourcesCount;

        DX12ResourceOwner()
            : m_resource(nullptr), m_subresourcesCount(0)
        {
        }
        ~DX12ResourceOwner() = default;

    public:
        // Action called on resource release event.
        TkAction<DX12ResourceOwner *> onRelease;
        // The resource state tracking helper. Used for resource barriers.
        DX12ResourceState state;

    public:
        // Gets the subresources count.
        TK_FORCE_INLINE u32 getSubresourcesCount() const
        {
            return m_subresourcesCount;
        }

        // Gets DirectX 12 resource object handle
        TK_FORCE_INLINE ID3D12Resource *getResource() const
        {
            return m_resource;
        }

        // Gets resource owner object as a GfxResource type or returns null if cannot perform cast.
        virtual GfxResource *asResource() const = 0;

    protected:
        TK_FORCE_INLINE void initResource(ID3D12Resource *resource, const D3D12_RESOURCE_STATES initialState, const D3D12_RESOURCE_DESC &desc, bool usePerSubresourceTracking = false)
        {
            initResource(resource, initialState, desc.DepthOrArraySize * desc.MipLevels, usePerSubresourceTracking);
        }

        void initResource(ID3D12Resource *resource, const D3D12_RESOURCE_STATES initialState, const u32 subresourceCount, bool usePerSubresourceTracking = false);
        void initResource(const D3D12_RESOURCE_STATES initialState, const u32 subresourceCount, bool usePerSubresourceTracking = false);
        void releaseResource(u32 safeFrameCount = DX12_RESOURCE_DELETE_SAFE_FRAMES_COUNT);
    };

}

#endif
