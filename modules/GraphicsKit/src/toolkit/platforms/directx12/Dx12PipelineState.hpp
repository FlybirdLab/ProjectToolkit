/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "DX12Device.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/base/Hash.hpp"
#include "toolkit/graphics/GfxPipelineState.hpp"

namespace toolkit
{
    class DX12TextureView;
    class DX12VertexLayout;

    struct DX12PipelineStateKey
    {
    public:
        i32 rtsCount;
        GfxMSAALevel msaa;
        GfxVertexLayout *vertexLayout;
        PixelFormat depthFormat;
        PixelFormat rtvsFormats[GFX_MAX_RT_BINDED];

    public:
        bool operator==(const DX12PipelineStateKey &other) const
        {
            return TkMemory::memoryCompare((void *)this, &other, sizeof(DX12PipelineStateKey)) == 0;
        }

        friend inline u32 getHash(const DX12PipelineStateKey &key)
        {
            u32 hash = (i32)key.msaa * 11;
            combineHash(hash, (u32)key.depthFormat * 93473262);
            combineHash(hash, key.rtsCount * 136);
            combineHash(hash, HashFunc<void *>::get(key.vertexLayout));
            combineHash(hash, (u32)key.rtvsFormats[0]);
            combineHash(hash, (u32)key.rtvsFormats[1]);
            combineHash(hash, (u32)key.rtvsFormats[2]);
            combineHash(hash, (u32)key.rtvsFormats[3]);
            combineHash(hash, (u32)key.rtvsFormats[4]);
            combineHash(hash, (u32)key.rtvsFormats[5]);
            static_assert(GFX_MAX_RT_BINDED == 6, "Update hash combine code to match RT count (manually inlined loop).");
            return hash;
        }
    };

    // Graphics pipeline state object for DirectX 12 backend.
    class DX12PipelineState : public DX12Resource<GfxPipelineState>
    {
    private:
        TkHashMap<DX12PipelineStateKey, ID3D12PipelineState *> m_states;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc;

    public:
        DX12PipelineState(DX12Device *device);

    public:
        D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        DxShaderHeader header;
        DX12VertexLayout *vertexBufferLayout = nullptr;
        DX12VertexLayout *vertexInputLayout = nullptr;

    public:
        // Gets DirectX 12 graphics pipeline state object for the given rendering state. Uses depth buffer and render targets formats and multi-sample levels to setup a proper PSO. Uses caching.
        ID3D12PipelineState *getState(DX12TextureView *depth, i32 rtCount, DX12TextureView **rtHandles, DX12VertexLayout *vertexLayout);

        bool isValid() const override;
        bool init(const Description &desc) override;

    protected:
        void onReleaseData() override;
    };

}

#endif
