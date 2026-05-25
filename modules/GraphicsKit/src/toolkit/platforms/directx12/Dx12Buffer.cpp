/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12Buffer.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Context.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    void DX12BufferView::setSRV(D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc)
    {
        m_srv.createSRV(m_device, m_owner->getResource(), &srvDesc);
    }

    void DX12BufferView::setUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC &uavDesc, ID3D12Resource *counterResource)
    {
        m_uav.createUAV(m_device, m_owner->getResource(), &uavDesc, counterResource);
    }

    u64 DX12Buffer::getSizeInBytes() const
    {
        return m_memoryUsage;
    }

    D3D12_GPU_VIRTUAL_ADDRESS DX12Buffer::getLocation() const
    {
        return m_resource->GetGPUVirtualAddress();
    }

    GfxBufferView *DX12Buffer::view() const
    {
        return (GfxBufferView *)&m_view;
    }

    void *DX12Buffer::map(GfxResourceMapMode mode)
    {
        D3D12_RANGE readRange;
        D3D12_RANGE *readRangePtr;
        switch (mode)
        {
        case GfxResourceMapMode::kRead:
            readRangePtr = nullptr;
            break;
        case GfxResourceMapMode::kWrite:
            readRange.Begin = readRange.End = 0;
            readRangePtr = &readRange;
            break;
        case GfxResourceMapMode::kReadWrite:
            readRangePtr = nullptr;
            break;
        default:
            TK_ASSERT(false);
            return nullptr;
        }
        m_lastMapMode = mode;
        void *mapped = nullptr;
        const HRESULT result = m_resource->Map(0, readRangePtr, &mapped);
        LOG_DIRECTX_RESULT(result);
        return mapped;
    }

    void DX12Buffer::unmap()
    {
        D3D12_RANGE writtenRange;
        D3D12_RANGE *writtenRangePtr;
        switch (m_lastMapMode)
        {
        case GfxResourceMapMode::kRead:
            writtenRange.Begin = writtenRange.End = 0;
            writtenRangePtr = &writtenRange;
            break;
        case GfxResourceMapMode::kWrite:
            writtenRangePtr = nullptr;
            break;
        case GfxResourceMapMode::kReadWrite:
            writtenRangePtr = nullptr;
            break;
        default:
            return;
        }
        m_resource->Unmap(0, writtenRangePtr);
        m_lastMapMode = (GfxResourceMapMode)255;
    }

    GfxResource *DX12Buffer::asResource() const
    {
        return (GfxResource *)this;
    }

    bool DX12Buffer::onInit()
    {
        const bool useSRV = isShaderResource();
        const bool useUAV = isUnorderedAccess();

        D3D12_RESOURCE_DESC resourceDesc;
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = m_desc.size;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        if (!useSRV)
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        if (useUAV)
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        D3D12_HEAP_PROPERTIES heapProperties;
        switch (m_desc.usage)
        {
        case GfxResourceUsage::kStagingUpload:
        case GfxResourceUsage::kStaging:
            heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
            break;
        case GfxResourceUsage::kStagingReadback:
            heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
            break;
        default:
            heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        }
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 1;
        heapProperties.VisibleNodeMask = 1;

        ID3D12Resource *resource;
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
        VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&resource)));

        initResource(resource, initialState, 1);
#if GFX_ENABLE_RESOURCE_NAMING
        {
            const TkStringView name = getName();
            if (name.length() > 0)
                DX_SET_DEBUG_NAME(m_resource, TkString(name));
        }
#endif
        m_memoryUsage = m_desc.size;
        const u32 numElements = m_desc.getElementsCount();

        if (m_desc.initData)
        {
            if (m_desc.usage == GfxResourceUsage::kStagingUpload || m_desc.usage == GfxResourceUsage::kStaging)
            {
                setData(m_desc.initData, m_desc.size);
            }
            else if (m_device->isRendering())
            {
                m_device->getMainContextDX12()->updateBuffer(this, m_desc.initData, m_desc.size, 0);
            }
            else
            {
                setData(m_desc.initData, m_desc.size);
            }
        }

        if ((m_desc.flags & GfxBufferFlags::kCounter) != GfxBufferFlags::kNone ||
            (m_desc.flags & GfxBufferFlags::kAppend) != GfxBufferFlags::kNone)
        {
            m_counter = TkMemory::newObject<DX12Buffer>(TkMemory::getDefault(), m_device, TkStringView::Empty);
            if (!m_counter->init(GfxBufferDescription::raw(4, GfxBufferFlags::kUnorderedAccess)))
            {
                return false;
            }
        }

        m_view.init(m_device, this, this);
        if (useSRV)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
            if ((m_desc.flags & GfxBufferFlags::kRawBuffer) != GfxBufferFlags::kNone)
                srvDesc.Format = DxUtils::toDxgiFormat(m_desc.format);
            else
                srvDesc.Format = DxUtils::toDxgiFormat(GfxPixelFormat::findShaderResourceFormat(m_desc.format, false));
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = numElements;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            if ((m_desc.flags & GfxBufferFlags::kStructured) != GfxBufferFlags::kNone)
            {
                srvDesc.Buffer.StructureByteStride = m_desc.stride;
                srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            }
            else
            {
                srvDesc.Buffer.StructureByteStride = 0;
            }
            if ((m_desc.flags & GfxBufferFlags::kRawBuffer) != GfxBufferFlags::kNone)
                srvDesc.Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
            m_view.setSRV(srvDesc);
        }
        if (useUAV)
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.StructureByteStride = 0;
            uavDesc.Buffer.CounterOffsetInBytes = 0;
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            uavDesc.Buffer.NumElements = numElements;
            if ((m_desc.flags & GfxBufferFlags::kStructured) != GfxBufferFlags::kNone)
                uavDesc.Buffer.StructureByteStride = m_desc.stride;
            if ((m_desc.flags & GfxBufferFlags::kRawBuffer) != GfxBufferFlags::kNone)
                uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
            if ((m_desc.flags & GfxBufferFlags::kStructured) != GfxBufferFlags::kNone)
                uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            else
                uavDesc.Format = DxUtils::toDxgiFormat(GfxPixelFormat::findUnorderedAccessFormat(m_desc.format));
            m_view.setUAV(uavDesc, m_counter ? m_counter->getResource() : nullptr);
        }

        return true;
    }

    void DX12Buffer::onReleaseData()
    {
        m_view.release();
        releaseResource();
        GFX_DEL_RESOURCE(m_counter);

        GfxBuffer::onReleaseData();
    }

}

#endif
