/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11Buffer.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    DX11Buffer::DX11Buffer(DX11Device *device, const TkStringView &name)
        : DX11Resource<GfxBuffer>(device, name)
    {
        m_view.setParnet(this);
    }

    GfxBufferView *DX11Buffer::view() const
    {
        return (GfxBufferView *)&m_view;
    }

    void *DX11Buffer::map(GfxResourceMapMode mode)
    {
        TK_ASSERT(!m_mapped);

        D3D11_MAPPED_SUBRESOURCE map;
        map.pData = nullptr;
        D3D11_MAP mapType;
        UINT mapFlags = 0;
        switch (mode)
        {
        case GfxResourceMapMode::kRead:
            mapType = D3D11_MAP_READ;
            if (m_desc.usage == GfxResourceUsage::kStagingReadback)
                mapFlags = D3D11_MAP_FLAG_DO_NOT_WAIT;
            break;
        case GfxResourceMapMode::kWrite:
            mapType = D3D11_MAP_WRITE_DISCARD;
            break;
        case GfxResourceMapMode::kReadWrite:
            mapType = D3D11_MAP_READ_WRITE;
            break;
        default:
            return nullptr;
        }

        const HRESULT result = m_device->getIM()->Map(m_resource, 0, mapType, mapFlags, &map);
        if (result != DXGI_ERROR_WAS_STILL_DRAWING)
            LOG_DIRECTX_RESULT(result);

        m_mapped = map.pData != nullptr;

        return map.pData;
    }

    void DX11Buffer::unmap()
    {
        TK_ASSERT(m_mapped);
        m_mapped = false;
        m_device->getIM()->Unmap(m_resource, 0);
    }

    bool DX11Buffer::onInit()
    {
        bool useSRV = isShaderResource();
        bool useUAV = isUnorderedAccess();

        // Create buffer description
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = m_desc.size;
        bufferDesc.Usage = DxUtils::toD3D11Usage(m_desc.usage);
        bufferDesc.BindFlags = 0;
        bufferDesc.CPUAccessFlags = DxUtils::getDX11CpuAccessFlagsFromUsage(m_desc.usage);
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        if ((m_desc.flags & GfxBufferFlags::kVertexBuffer) != GfxBufferFlags::kNone)
            bufferDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
        if ((m_desc.flags & GfxBufferFlags::kIndexBuffer) != GfxBufferFlags::kNone)
            bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
        if (useSRV)
            bufferDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        if (useUAV)
            bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

        if ((m_desc.flags & GfxBufferFlags::kArgument) != GfxBufferFlags::kNone)
            bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
        if ((m_desc.flags & GfxBufferFlags::kRawBuffer) != GfxBufferFlags::kNone)
            bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        if ((m_desc.flags & GfxBufferFlags::kStructured) != GfxBufferFlags::kNone)
        {
            bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            bufferDesc.StructureByteStride = m_desc.stride;
        }

        // Create resource
        D3D11_SUBRESOURCE_DATA data;
        if (m_desc.initData)
        {
            data.pSysMem = m_desc.initData;
            data.SysMemPitch = bufferDesc.ByteWidth;
            data.SysMemSlicePitch = 0;
        }
        VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateBuffer(&bufferDesc, m_desc.initData ? &data : nullptr, &m_resource));

        // Set state
        DX_SET_DEBUG_NAME(m_resource, getName());
        m_memoryUsage = m_desc.size;
        i32 numElements = m_desc.getElementsCount();

        // Create views
        if (useSRV)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            if ((m_desc.flags & GfxBufferFlags::kRawBuffer) != GfxBufferFlags::kNone)
            {
                srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
                srvDesc.BufferEx.FirstElement = 0;
                srvDesc.BufferEx.NumElements = numElements;
                srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
            }
            else
            {
                if ((m_desc.flags & GfxBufferFlags::kStructured) != GfxBufferFlags::kNone)
                    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
                else
                    srvDesc.Format = DxUtils::toDxgiFormat(GfxPixelFormat::findShaderResourceFormat(m_desc.format, false));
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
                srvDesc.Buffer.FirstElement = 0;
                srvDesc.Buffer.NumElements = numElements;
            }
            ID3D11ShaderResourceView *srv = nullptr;
            VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateShaderResourceView(m_resource, &srvDesc, &srv));
            m_view.setSRV(srv);
        }
        if (useUAV)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = numElements;
            uavDesc.Buffer.Flags = 0;
            if ((m_desc.flags & GfxBufferFlags::kRawBuffer) != GfxBufferFlags::kNone)
                uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
            if ((m_desc.flags & GfxBufferFlags::kAppend) != GfxBufferFlags::kNone)
                uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
            if ((m_desc.flags & GfxBufferFlags::kCounter) != GfxBufferFlags::kNone)
                uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;
            if ((m_desc.flags & GfxBufferFlags::kStructured) != GfxBufferFlags::kNone)
                uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            else
                uavDesc.Format = DxUtils::toDxgiFormat(GfxPixelFormat::findUnorderedAccessFormat(m_desc.format));
            ID3D11UnorderedAccessView *uav = nullptr;
            VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateUnorderedAccessView(m_resource, &uavDesc, &uav));
            m_view.setUAV(uav);
        }

        return true;
    }

    void DX11Buffer::onReleaseData()
    {
        m_view.release();
        DX_DEL(m_resource, 0);

        // Base
        GfxBuffer::onReleaseData();
    }

}

#endif
