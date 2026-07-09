/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "toolkit/collection/Array.hpp"
#include "toolkit/graphics/GfxTexture.hpp"
#include "DX12Device.hpp"
#include "DX12IShaderRes.hpp"

namespace toolkit
{
    // The texture view for DirectX 12 backend.
    class DX12TextureView : public GfxTextureView, public DX12IShaderResource
    {
    private:
        DX12Device *m_device = nullptr;
        DX12ResourceOwner *m_owner = nullptr;
        DX12DescriptorHeapWithSlots::Slot m_rtv, m_srv, m_dsv, m_uav;

    public:
        DX12TextureView() = default;
        ~DX12TextureView()
        {
            release();
        }

    public:
        void init(GfxResource *parent, DX12Device *device, DX12ResourceOwner *owner, PixelFormat format, GfxMSAALevel msaa, i32 subresourceIndex = -1)
        {
            GfxTextureView::init(parent, format, msaa);
            subResIdx = subresourceIndex;
            m_device = device;
            m_owner = owner;
        }

        // Release the view.
        void release();

    public:
        bool readOnlyDepthView = false;
        void setRTV(D3D12_RENDER_TARGET_VIEW_DESC &rtvDesc);
        void setSRV(D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc);
        void setDSV(D3D12_DEPTH_STENCIL_VIEW_DESC &dsvDesc);
        void setUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC &uavDesc, ID3D12Resource *counterResource = nullptr);

    public:
        // Get the CPU handle to the render target view descriptor.
        TK_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE rtv() const
        {
            return m_rtv.cpu();
        }

        // Get the CPU handle to the depth stencil view descriptor.
        TK_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE dsv() const
        {
            return m_dsv.cpu();
        }

    public:
        void *getNativePtr() const override
        {
            return (void *)(DX12IShaderResource *)this;
        }

        bool isDepthStencilResource() const override
        {
            return m_dsv.isValid();
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
    };

    // Texture object for DirectX 12 backend
    class DX12Texture : public DX12Resource<GfxTexture>, public DX12ResourceOwner, public DX12IShaderResource
    {
    public:
        DX12Texture(DX12Device *device, const TkStringView &name)
            : DX12Resource<GfxTexture>(device, name)
        {
        }

    public:
        GfxTextureView *view(i32 arrayOrDepthIndex) const override
        {
            return (GfxTextureView *)&m_handlesPerSlice[arrayOrDepthIndex];
        }
        GfxTextureView *view(i32 arrayOrDepthIndex, i32 mipMapIndex) const override
        {
            return (GfxTextureView *)&m_handlesPerMip[arrayOrDepthIndex][mipMapIndex];
        }
        GfxTextureView *viewArray() const override
        {
            TK_ASSERT(arraySize() > 1);
            return (GfxTextureView *)&m_handleArray;
        }
        GfxTextureView *viewVolume() const override
        {
            TK_ASSERT(isVolume());
            return (GfxTextureView *)&m_handleVolume;
        }
        GfxTextureView *viewReadOnlyDepth() const override
        {
            TK_ASSERT(m_desc.flags & GfxTextureFlags::kReadOnlyDepthView);
            return (GfxTextureView *)&m_handleReadOnlyDepth;
        }
        void *getNativePtr() const override
        {
            return (void *)m_resource;
        }
        bool getData(i32 arrayIndex, i32 mipMapIndex, TextureMipData &data, u32 mipRowPitch) override;

        GfxResource *asResource() const override
        {
            return (GfxResource *)this;
        }

        bool isDepthStencilResource() const override
        {
            return (m_desc.flags & GfxTextureFlags::kDepthStencil) != GfxTextureFlags::kNone;
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
            return (DX12ResourceOwner *)this;
        }

    protected:
        bool onInit() override;
        void onResidentMipsChange() override;
        void onReleaseData() override;

    private:
        void initHandles();

    private:
        DX12TextureView m_handleArray;
        DX12TextureView m_handleVolume;
        DX12TextureView m_handleReadOnlyDepth;
        TkArray<DX12TextureView> m_handlesPerSlice;        // [slice]
        TkArray<TkArray<DX12TextureView>> m_handlesPerMip; // [slice][mip]

        DX12DescriptorHeapWithSlots::Slot m_srv;
        DX12DescriptorHeapWithSlots::Slot m_uav;

        DXGI_FORMAT m_dxgiFormatDSV;
        DXGI_FORMAT m_dxgiFormatSRV;
        DXGI_FORMAT m_dxgiFormatRTV;
        DXGI_FORMAT m_dxgiFormatUAV;
    };

}

#endif
