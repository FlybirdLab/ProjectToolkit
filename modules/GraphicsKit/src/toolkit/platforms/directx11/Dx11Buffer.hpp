/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/graphics/GfxBuffer.hpp"
#include "Dx11Device.hpp"
#include "Dx11IShaderRes.hpp"

namespace toolkit
{
    // The buffer view for DirectX 11 backend.
    class DX11BufferView : public GfxBufferView, public IShaderResourceDX11
    {
    public:
        DX11BufferView() = default;
        ~DX11BufferView()
        {
            release();
        }

    public:
        void setParnet(GfxBuffer *parent)
        {
            m_parent = parent;
        }

        // Release the view.
        void release()
        {
            DX_DEL(m_srv, 0);
            DX_DEL(m_uav, 0);
        }

    public:
        // Set new shader resource view.
        void setSRV(ID3D11ShaderResourceView *srv)
        {
            DX_DEL(m_srv, 0);
            m_srv = srv;
        }

        // Set new unordered access view.
        void setUAV(ID3D11UnorderedAccessView *uav)
        {
            DX_DEL(m_uav, 0);
            m_uav = uav;
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
        ID3D11ShaderResourceView *m_srv = nullptr;
        ID3D11UnorderedAccessView *m_uav = nullptr;
    };

    // GPU buffer for DirectX 11 backend
    class DX11Buffer : public DX11Resource<GfxBuffer>
    {
    public:
        DX11Buffer(DX11Device *device, const TkStringView &name);

    public:
        // Gets DirectX 11 buffer object handle
        TK_FORCE_INLINE ID3D11Buffer *getBuffer() const
        {
            return m_resource;
        }

    public:
        // [GfxBuffer]
        GfxBufferView *view() const override;
        void *map(GfxResourceMapMode mode) override;
        void unmap() override;

        ID3D11Resource *getResource() override
        {
            return m_resource;
        }

    protected:
        bool onInit() override;
        void onReleaseData() override;

    private:
        ID3D11Buffer *m_resource = nullptr;
        DX11BufferView m_view;
        bool m_mapped = false;
    };

}

#endif
