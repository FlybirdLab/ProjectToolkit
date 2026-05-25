/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx11/common.hpp>

#if GFX_API_DIRECTX11

#include "toolkit/collection/Array.hpp"
#include "toolkit/graphics/GfxTexture.hpp"
#include "Dx11Device.hpp"
#include "Dx11IShaderRes.hpp"

namespace toolkit
{
    // The texture view for DirectX 11 backend.
    class DX11TextureView : public GfxTextureView, public IShaderResourceDX11
    {
    public:
        DX11TextureView() = default;
        ~DX11TextureView()
        {
            release();
        }

    public:
        void init(GfxResource *parent, ID3D11RenderTargetView *rtv, ID3D11ShaderResourceView *srv, ID3D11DepthStencilView *dsv, ID3D11UnorderedAccessView *uav, PixelFormat format, GfxMSAALevel msaa)
        {
            GfxTextureView::init(parent, format, msaa);
            m_rtv = rtv;
            m_srv = srv;
            m_dsv = dsv;
            m_uav = uav;
        }

        // Release the view.
        void release()
        {
            DX_DEL(m_rtv, 0);
            DX_DEL(m_srv, 0);
            DX_DEL(m_dsv, 0);
            DX_DEL(m_uav, 0);
        }

    public:
        // Set new render target view.
        void setRTV(ID3D11RenderTargetView *rtv)
        {
            DX_DEL(m_rtv, 0);
            m_rtv = rtv;
        }

        // Set new shader resource view.
        void setSRV(ID3D11ShaderResourceView *srv)
        {
            DX_DEL(m_srv, 0);
            m_srv = srv;
        }

        // Set new depth stencil view.
        void setDSV(ID3D11DepthStencilView *dsv)
        {
            DX_DEL(m_dsv, 0);
            m_dsv = dsv;
        }

        // Set new unordered access view.
        void setUAV(ID3D11UnorderedAccessView *uav)
        {
            DX_DEL(m_uav, 0);
            m_uav = uav;
        }

    public:
        // Get the render target view.
        ID3D11RenderTargetView *rtv() const
        {
            return m_rtv;
        }

        // Get the depth stencil view.
        ID3D11DepthStencilView *dsv() const
        {
            return m_dsv;
        }

    public:
        void *getNativePtr() const override
        {
            return (void *)(IShaderResourceDX11 *)this;
        }

        ID3D11ShaderResourceView *srv() const override
        {
            return m_srv;
        }

        ID3D11UnorderedAccessView *uav() const override
        {
            return m_uav;
        }

    private:
        ID3D11RenderTargetView *m_rtv = nullptr;
        ID3D11ShaderResourceView *m_srv = nullptr;
        ID3D11DepthStencilView *m_dsv = nullptr;
        ID3D11UnorderedAccessView *m_uav = nullptr;
    };


    // Texture object for DirectX 11 backend.
    class DX11Texture : public DX11Resource<GfxTexture>
    {
    public:
        DX11Texture(DX11Device *device, const TkStringView &name)
            : DX11Resource<GfxTexture>(device, name)
        {
        }

    public:
        // Get DX11 texture resource.
        TK_FORCE_INLINE ID3D11Resource *getResource() const
        {
            return m_resource;
        }

    private:
        void initHandles();

        ID3D11Texture2D *getTexture2D() const
        {
            TK_ASSERT(m_desc.dimensions == TextureDimensions::kTexture || m_desc.dimensions == TextureDimensions::kCubeTexture);
            return (ID3D11Texture2D *)m_resource;
        }
        ID3D11Texture3D *getTexture3D() const
        {
            TK_ASSERT(m_desc.dimensions == TextureDimensions::kVolumeTexture);
            return (ID3D11Texture3D *)m_resource;
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
            return static_cast<void *>(m_resource);
        }
        bool getData(i32 arrayIndex, i32 mipMapIndex, TextureMipData &data, u32 mipRowPitch) override;

        ID3D11Resource *getResource() override
        {
            return m_resource;
        }

    protected:
        bool onInit() override;
        void onResidentMipsChange() override;
        void onReleaseData() override;

    private:
        ID3D11Resource *m_resource = nullptr;

        DX11TextureView m_handleArray;
        DX11TextureView m_handleVolume;
        DX11TextureView m_handleReadOnlyDepth;
        TkArray<DX11TextureView> m_handlesPerSlice;      // [slice]
        TkArray<TkArray<DX11TextureView>> m_handlesPerMip; // [slice][mip]

        DXGI_FORMAT m_dxgiFormatDSV;
        DXGI_FORMAT m_dxgiFormatSRV;
        DXGI_FORMAT m_dxgiFormatRTV;
        DXGI_FORMAT m_dxgiFormatUAV;
    };
}

#endif
