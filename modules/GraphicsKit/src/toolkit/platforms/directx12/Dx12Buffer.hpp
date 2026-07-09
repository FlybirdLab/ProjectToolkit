/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx12/common.hpp>

#if GFX_API_DIRECTX12

#include "toolkit/graphics/GfxBuffer.hpp"
#include "Dx12Device.hpp"
#include "Dx12IShaderRes.hpp"

namespace toolkit
{
    // The buffer view for DirectX 12 backend.
    class DX12BufferView : public GfxBufferView, public DX12IShaderResource
    {
    public:
        DX12BufferView()
        {
            srvDimension = D3D12_SRV_DIMENSION_BUFFER;
            uavDimension = D3D12_UAV_DIMENSION_BUFFER;
        }

        ~DX12BufferView()
        {
            release();
        }

    public:
        void init(DX12Device *device, DX12ResourceOwner *owner, GfxResource *parent)
        {
            m_device = device;
            m_owner = owner;
            m_parent = parent;
        }

        // Release the view.
        void release()
        {
            m_srv.release();
            m_uav.release();
        }

    public:
        // Set the shader resource view.
        void setSRV(D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc);
        // Set the unordered access view.
        void setUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC &uavDesc, ID3D12Resource *counterResource = nullptr);

    public:
        void *getNativePtr() const override
        {
            return (void *)(DX12IShaderResource *)this;
        }

        bool isDepthStencilResource() const override
        {
            return false;
        }
        D3D12_CPU_DESCRIPTOR_HANDLE srv() const override
        {
            return m_srv.cpu();
        }
        D3D12_CPU_DESCRIPTOR_HANDLE uav() const override
        {
            return m_uav.cpu();
        }
        DX12ResourceOwner *getResourceOwner() const override
        {
            return m_owner;
        }

    private:
        DX12Device *m_device = nullptr;
        DX12ResourceOwner *m_owner = nullptr;
        DX12DescriptorHeapWithSlots::Slot m_srv, m_uav;
    };

    // GPU buffer for DirectX 12 backend
    class DX12Buffer : public DX12Resource<GfxBuffer>, public DX12ResourceOwner
    {
    public:
        DX12Buffer(DX12Device *device, const TkStringView &name)
            : DX12Resource<GfxBuffer>(device, name)
        {
        }

    public:
        // Get vertex buffer view descriptor. Valid only for the vertex buffers.
        TK_FORCE_INLINE void getVBView(D3D12_VERTEX_BUFFER_VIEW &view) const
        {
            view.StrideInBytes = getStride();
            view.SizeInBytes = (UINT)getSizeInBytes();
            view.BufferLocation = getLocation();
        }

        // Get index buffer view descriptor. Valid only for the index buffers.
        TK_FORCE_INLINE void getIBView(D3D12_INDEX_BUFFER_VIEW &view) const
        {
            view.Format = getStride() == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
            view.SizeInBytes = (UINT)getSizeInBytes();
            view.BufferLocation = getLocation();
        }

        // Get buffer size in a GPU memory in bytes.
        u64 getSizeInBytes() const;
        // Get buffer location in a GPU memory.
        D3D12_GPU_VIRTUAL_ADDRESS getLocation() const;
        // Get the counter resource.
        TK_FORCE_INLINE DX12Buffer *getCounter() const
        {
            return m_counter;
        }

    public:
        GfxBufferView *view() const override;
        void *map(GfxResourceMapMode mode) override;
        void unmap() override;

        GfxResource *asResource() const override;

    protected:
        bool onInit() override;
        void onReleaseData() override;

    private:
        DX12BufferView m_view;
        DX12Buffer *m_counter = nullptr;
        GfxResourceMapMode m_lastMapMode = (GfxResourceMapMode)255;
    };

}

#endif
