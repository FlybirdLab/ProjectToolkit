/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "DX12Texture.hpp"
#include "toolkit/graphics/utils/TextureData.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Context.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    // -------------------------------------------------------------------------
    // DX12TextureView
    // -------------------------------------------------------------------------

    void DX12TextureView::release()
    {
        m_rtv.release();
        m_srv.release();
        m_dsv.release();
        m_uav.release();
    }

    void DX12TextureView::setRTV(D3D12_RENDER_TARGET_VIEW_DESC &rtvDesc)
    {
        m_rtv.createRTV(m_device, m_owner->getResource(), &rtvDesc);
    }

    void DX12TextureView::setSRV(D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc)
    {
        srvDimension = srvDesc.ViewDimension;
        m_srv.createSRV(m_device, m_owner->getResource(), &srvDesc);
    }

    void DX12TextureView::setDSV(D3D12_DEPTH_STENCIL_VIEW_DESC &dsvDesc)
    {
        m_dsv.createDSV(m_device, m_owner->getResource(), &dsvDesc);
    }

    void DX12TextureView::setUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC &uavDesc, ID3D12Resource *counterResource)
    {
        uavDimension = uavDesc.ViewDimension;
        m_uav.createUAV(m_device, m_owner->getResource(), &uavDesc, counterResource);
    }

    // -------------------------------------------------------------------------
    // DX12Texture
    // -------------------------------------------------------------------------

    bool DX12Texture::getData(i32 arrayIndex, i32 mipMapIndex, TextureMipData &data, u32 mipRowPitch)
    {
        if (!isStaging())
        {
            LogWarn("Texture::getData is valid only for staging resources.");
            return true;
        }

        // Internally it's a buffer, so adapt resource index and offset
        const u32 subresource = DxUtils::calcSubresourceIndex(mipMapIndex, arrayIndex, mipLevels());
        const i32 offsetInBytes = computeBufferOffset(subresource, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
        const i32 lengthInBytes = computeSubresourceSize(subresource, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
        const i32 rowPitch = computeRowPitch(mipMapIndex, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        const i32 depthPitch = computeSlicePitch(mipMapIndex, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

        // Map the staging resource mip map for reading
        D3D12_RANGE range;
        range.Begin = offsetInBytes;
        range.End = offsetInBytes + lengthInBytes;
        void *mapped = nullptr;
        const HRESULT mapResult = m_resource->Map(0, &range, &mapped);
        if (FAILED(mapResult))
        {
            LOG_DIRECTX_RESULT(mapResult);
            return true;
        }
        mapped = (byte *)mapped + offsetInBytes;

        data.copy(mapped, rowPitch, depthPitch, depth(), mipRowPitch);

        // Unmap buffer
        m_resource->Unmap(0, nullptr);

        return false;
    }

    bool DX12Texture::onInit()
    {
        ID3D12Resource *resource = nullptr;

        // Cache formats
        const PixelFormat fmt = format();
        const PixelFormat typelessFormat = GfxPixelFormat::makeTypeless(fmt);
        const DXGI_FORMAT dxgiFormat = DxUtils::toDxgiFormat(typelessFormat);
        m_dxgiFormatDSV = DxUtils::toDxgiFormat(GfxPixelFormat::findDepthStencilFormat(fmt));
        m_dxgiFormatSRV = DxUtils::toDxgiFormat(GfxPixelFormat::findShaderResourceFormat(fmt, m_sRGB));
        m_dxgiFormatRTV = m_dxgiFormatSRV;
        m_dxgiFormatUAV = DxUtils::toDxgiFormat(GfxPixelFormat::findUnorderedAccessFormat(fmt));

        // Cache properties
        auto device = m_device->getDevice();
        bool useSRV = isShaderResource();
        bool useDSV = isDepthStencil();
        bool useRTV = isRenderTarget();
        bool useUAV = isUnorderedAccess();
        i32 depthOrArraySize = isVolume() ? depth() : arraySize();

        if (isStaging())
        {
            // Initialize as a buffer
            const i32 totalSize = computeBufferTotalSize(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
            D3D12_HEAP_PROPERTIES heapProperties;
            heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
            heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProperties.CreationNodeMask = 1;
            heapProperties.VisibleNodeMask = 1;
            D3D12_RESOURCE_DESC resourceDesc;
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Alignment = 0;
            resourceDesc.Width = totalSize;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            auto result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));
            LOG_DIRECTX_RESULT_WITH_RETURN(result, false);
            initResource(resource, D3D12_RESOURCE_STATE_COPY_DEST, 1u);
            DX_SET_DEBUG_NAME(m_resource, getName());
            m_memoryUsage = totalSize;
            return true;
        }

        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

        // Create texture description
        D3D12_RESOURCE_DESC resourceDesc;
        resourceDesc.MipLevels = m_desc.mipLevels;
        resourceDesc.Format = dxgiFormat;
        resourceDesc.Width = m_desc.width;
        resourceDesc.Height = m_desc.height;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.DepthOrArraySize = depthOrArraySize;
        resourceDesc.SampleDesc.Count = static_cast<UINT>(m_desc.multiSampleLevel);
        resourceDesc.SampleDesc.Quality = isMultiSample() ? DX12Device::getMaxMSAAQuality((u32)m_desc.multiSampleLevel) : 0;
        resourceDesc.Alignment = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Dimension = isVolume() ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        if (useRTV)
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
        }
        else if (useDSV)
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            if (!useSRV)
            {
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
            }
        }
        if (useUAV)
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        // Create heap properties
        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 1;
        heapProperties.VisibleNodeMask = 1;

        // Create clear value (used by render targets and depth stencil buffers)
        D3D12_CLEAR_VALUE *clearValuePtr = nullptr;
        D3D12_CLEAR_VALUE clearValue;
        if (useRTV)
        {
            clearValue.Format = m_dxgiFormatSRV;
            TkMemory::memoryCopy(&clearValue.Color, m_desc.defaultClearColor.raw, sizeof(Color));
            clearValuePtr = &clearValue;
        }
        else if (useDSV)
        {
            clearValue.Format = m_dxgiFormatDSV;
            clearValue.DepthStencil.Depth = 1.0f;
            clearValue.DepthStencil.Stencil = 0;
            clearValuePtr = &clearValue;
        }

        if (isRegularTexture())
            initialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        // Create texture
        auto result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, clearValuePtr, IID_PPV_ARGS(&resource));
        LOG_DIRECTX_RESULT_WITH_RETURN(result, false);

        // Set state
        bool isRead = useSRV || useUAV;
        bool isWrite = useDSV || useRTV || useUAV;
        initResource(resource, initialState, resourceDesc, isRead && isWrite);
        DX_SET_DEBUG_NAME(m_resource, getName());
        m_memoryUsage = calculateMemoryUsage();

        // Initialize handles to the resource
        if (isRegularTexture())
        {
            // 'Regular' texture is using only one handle (texture/cubemap)
            m_handlesPerSlice.resize(1, false);
        }
        else
        {
            // Create all handles
            initHandles();
        }

        return true;
    }

    void DX12Texture::onResidentMipsChange()
    {
        const i32 firstMipIndex = mipLevels() - residentMipLevels();
        const i32 mipLevelsCount = residentMipLevels();
        D3D12_SHADER_RESOURCE_VIEW_DESC srDesc;
        srDesc.Format = m_dxgiFormatSRV;
        srDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        if (isCubeMap())
        {
            srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srDesc.TextureCube.MostDetailedMip = firstMipIndex;
            srDesc.TextureCube.MipLevels = mipLevelsCount;
            srDesc.TextureCube.ResourceMinLODClamp = 0;
        }
        else if (isVolume())
        {
            srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            srDesc.Texture3D.MostDetailedMip = firstMipIndex;
            srDesc.Texture3D.MipLevels = mipLevelsCount;
            srDesc.Texture3D.ResourceMinLODClamp = 0;
        }
        else
        {
            srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srDesc.Texture2D.MostDetailedMip = firstMipIndex;
            srDesc.Texture2D.MipLevels = mipLevelsCount;
            srDesc.Texture2D.PlaneSlice = 0;
            srDesc.Texture2D.ResourceMinLODClamp = 0;
        }
        DX12TextureView &v = isVolume() ? m_handleVolume : m_handlesPerSlice[0];
        if (v.getParent() == nullptr)
            v.init(this, m_device, this, format(), multiSampleLevel());
        if (mipLevelsCount != 0)
            v.setSRV(srDesc);
    }

    void DX12Texture::onReleaseData()
    {
        m_handlesPerMip.resize(0, false);
        m_handlesPerSlice.resize(0, false);
        m_handleArray.release();
        m_handleVolume.release();
        m_handleReadOnlyDepth.release();
        m_srv.release();
        m_uav.release();
        releaseResource();

        // Base
        GfxTexture::onReleaseData();
    }

    void DX12Texture::initHandles()
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtDesc;
        D3D12_SHADER_RESOURCE_VIEW_DESC srDesc;
        D3D12_DEPTH_STENCIL_VIEW_DESC dsDesc;
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;

        rtDesc.Format = m_dxgiFormatRTV;
        srDesc.Format = m_dxgiFormatSRV;
        srDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        dsDesc.Format = m_dxgiFormatDSV;
        dsDesc.Flags = D3D12_DSV_FLAG_NONE;
        uavDesc.Format = m_dxgiFormatUAV;

        // Cache properties
        bool useSRV = isShaderResource();
        bool useDSV = isDepthStencil();
        bool useRTV = isRenderTarget();
        bool useUAV = isUnorderedAccess();
        i32 arrSize = arraySize();
        i32 mipLevelsCount = mipLevels();
        bool isArr = arrSize > 1;
        bool isCube = isCubeMap();
        bool isMsaa = isMultiSample();
        bool isVol = isVolume();
        auto fmt = format();
        auto msaa = multiSampleLevel();

        // Create resource views
        if (useUAV)
        {
            if (isVol)
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.MipSlice = 0;
                uavDesc.Texture3D.WSize = depth();
                uavDesc.Texture3D.FirstWSlice = 0;
            }
            else if (isArr)
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.MipSlice = 0;
                uavDesc.Texture2DArray.ArraySize = arrSize * 6;
                uavDesc.Texture2DArray.FirstArraySlice = 0;
                uavDesc.Texture2DArray.PlaneSlice = 0;
            }
            else
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = 0;
                uavDesc.Texture2D.PlaneSlice = 0;
            }
            m_uav.createUAV(m_device, m_resource, &uavDesc);
        }
        if (isVol)
        {
            // Create handle for whole 3d texture
            m_handleVolume.init(this, m_device, this, fmt, msaa);
            if (useSRV)
            {
                srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                srDesc.Texture3D.MostDetailedMip = 0;
                srDesc.Texture3D.MipLevels = mipLevels();
                srDesc.Texture3D.ResourceMinLODClamp = 0;
                m_handleVolume.setSRV(srDesc);
            }
            if (useRTV)
            {
                rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                rtDesc.Texture3D.MipSlice = 0;
                rtDesc.Texture3D.FirstWSlice = 0;
                rtDesc.Texture3D.WSize = depth();
                m_handleVolume.setRTV(rtDesc);
            }
            if (useUAV)
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture2D.MipSlice = 0;
                uavDesc.Texture2D.PlaneSlice = 0;
                m_handleVolume.setUAV(uavDesc);
            }

            // Init per slice views
            m_handlesPerSlice.resize(depth(), false);
            if (m_desc.hasPerSliceViews() && useRTV)
            {
                rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                rtDesc.Texture3D.MipSlice = 0;
                rtDesc.Texture3D.WSize = 1;

                for (i32 sliceIndex = 0; sliceIndex < depth(); sliceIndex++)
                {
                    rtDesc.Texture3D.FirstWSlice = sliceIndex;
                    m_handlesPerSlice[sliceIndex].init(this, m_device, this, fmt, msaa);
                    m_handlesPerSlice[sliceIndex].setRTV(rtDesc);
                }
            }
        }
        else if (isArr)
        {
            // Resize handles
            m_handlesPerSlice.resize(arraySize(), false);

            // Create per array slice handles
            for (i32 arrayIndex = 0; arrayIndex < arrSize; arrayIndex++)
            {
                m_handlesPerSlice[arrayIndex].init(this, m_device, this, fmt, msaa);

                if (useDSV)
                {
                    dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsDesc.Texture2DArray.ArraySize = 1;
                    dsDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                    dsDesc.Texture2DArray.MipSlice = 0;
                    m_handlesPerSlice[arrayIndex].setDSV(dsDesc);
                }
                if (useRTV)
                {
                    rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtDesc.Texture2DArray.ArraySize = 1;
                    rtDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                    rtDesc.Texture2DArray.MipSlice = 0;
                    rtDesc.Texture2DArray.PlaneSlice = 0;
                    m_handlesPerSlice[arrayIndex].setRTV(rtDesc);
                }
                if (useSRV)
                {
                    srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    srDesc.Texture2DArray.ArraySize = 1;
                    srDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                    srDesc.Texture2DArray.MipLevels = mipLevelsCount;
                    srDesc.Texture2DArray.MostDetailedMip = 0;
                    srDesc.Texture2DArray.PlaneSlice = 0;
                    srDesc.Texture2DArray.ResourceMinLODClamp = 0;
                    m_handlesPerSlice[arrayIndex].setSRV(srDesc);
                    if (isCube)
                        m_handlesPerSlice[arrayIndex].srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // Hack (to reproduce the problem comment this line and use Spot Light with a shadow)
                }
                if (useUAV)
                {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    uavDesc.Texture2DArray.ArraySize = 1;
                    uavDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                    uavDesc.Texture2DArray.MipSlice = 0;
                    uavDesc.Texture2DArray.PlaneSlice = 0;
                    m_handlesPerSlice[arrayIndex].setSRV(srDesc);
                }
            }

            // Create whole array handle
            {
                m_handleArray.init(this, m_device, this, fmt, msaa);
                if (useDSV)
                {
                    dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsDesc.Texture2DArray.ArraySize = arrSize;
                    dsDesc.Texture2DArray.FirstArraySlice = 0;
                    dsDesc.Texture2DArray.MipSlice = 0;
                    m_handleArray.setDSV(dsDesc);
                }
                if (useRTV)
                {
                    rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtDesc.Texture2DArray.ArraySize = arrSize;
                    rtDesc.Texture2DArray.FirstArraySlice = 0;
                    rtDesc.Texture2DArray.MipSlice = 0;
                    rtDesc.Texture2DArray.PlaneSlice = 0;
                    m_handleArray.setRTV(rtDesc);
                }
                if (useSRV)
                {
                    if (isCube)
                    {
                        srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                        srDesc.TextureCube.MostDetailedMip = 0;
                        srDesc.TextureCube.MipLevels = mipLevelsCount;
                        srDesc.TextureCube.ResourceMinLODClamp = 0;
                    }
                    else
                    {
                        srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        srDesc.Texture2DArray.ArraySize = arrSize;
                        srDesc.Texture2DArray.FirstArraySlice = 0;
                        srDesc.Texture2DArray.MipLevels = mipLevelsCount;
                        srDesc.Texture2DArray.MostDetailedMip = 0;
                        srDesc.Texture2DArray.ResourceMinLODClamp = 0;
                        srDesc.Texture2DArray.PlaneSlice = 0;
                    }
                    m_handleArray.setSRV(srDesc);
                }
                if (useUAV)
                {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    uavDesc.Texture2DArray.ArraySize = arrSize;
                    uavDesc.Texture2DArray.FirstArraySlice = 0;
                    uavDesc.Texture2DArray.MipSlice = 0;
                    uavDesc.Texture2DArray.PlaneSlice = 0;
                    m_handleArray.setUAV(uavDesc);
                }
            }
        }
        else
        {
            // Create single handle for the whole texture
            m_handlesPerSlice.resize(1, false);
            m_handlesPerSlice[0].init(this, m_device, this, fmt, msaa);
            if (useDSV)
            {
                if (isCube)
                {
                    dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsDesc.Texture2DArray.MipSlice = 0;
                    dsDesc.Texture2DArray.FirstArraySlice = 0;
                    dsDesc.Texture2DArray.ArraySize = arrSize * 6;
                }
                else if (isMsaa)
                {
                    dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    dsDesc.Texture2D.MipSlice = 0;
                }
                m_handlesPerSlice[0].setDSV(dsDesc);
            }
            if (useRTV)
            {
                if (isCube)
                {
                    rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtDesc.Texture2DArray.MipSlice = 0;
                    rtDesc.Texture2DArray.FirstArraySlice = 0;
                    rtDesc.Texture2DArray.ArraySize = arrSize * 6;
                    rtDesc.Texture2DArray.PlaneSlice = 0;
                }
                else if (isMsaa)
                {
                    rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    rtDesc.Texture2D.MipSlice = 0;
                    rtDesc.Texture2D.PlaneSlice = 0;
                }
                m_handlesPerSlice[0].setRTV(rtDesc);
            }
            if (useSRV)
            {
                if (isCube)
                {
                    srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    srDesc.TextureCube.MostDetailedMip = 0;
                    srDesc.TextureCube.MipLevels = mipLevelsCount;
                    srDesc.TextureCube.ResourceMinLODClamp = 0;
                }
                else if (isMsaa)
                {
                    srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srDesc.Texture2D.MostDetailedMip = 0;
                    srDesc.Texture2D.MipLevels = mipLevelsCount;
                    srDesc.Texture2D.ResourceMinLODClamp = 0;
                    srDesc.Texture2D.PlaneSlice = 0;
                }
                m_handlesPerSlice[0].setSRV(srDesc);
            }
            if (useUAV)
            {
                if (isCube)
                {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    uavDesc.Texture2DArray.ArraySize = arrSize;
                    uavDesc.Texture2DArray.MipSlice = 0;
                    uavDesc.Texture2DArray.FirstArraySlice = 0;
                    uavDesc.Texture2DArray.PlaneSlice = 0;
                }
                else
                {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                    uavDesc.Texture2D.MipSlice = 0;
                    uavDesc.Texture2D.PlaneSlice = 0;
                }
                m_handlesPerSlice[0].setUAV(uavDesc);
            }
        }

        // Init per mip map handles
        if (hasPerMipViews())
        {
            // Init handles
            m_handlesPerMip.resize(arrSize, false);
            for (i32 arrayIndex = 0; arrayIndex < arrSize; arrayIndex++)
            {
                auto &slice = m_handlesPerMip[arrayIndex];
                slice.resize(mipLevelsCount, false);

                for (i32 mipIndex = 0; mipIndex < mipLevelsCount; mipIndex++)
                {
                    i32 subresourceIndex = arrayIndex * mipLevelsCount + mipIndex;
                    slice[mipIndex].init(this, m_device, this, fmt, msaa, subresourceIndex);

                    // DSV
                    if (useDSV)
                    {
                        if (isArr)
                        {
                            dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                            dsDesc.Texture2DArray.ArraySize = 1;
                            dsDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                            dsDesc.Texture2DArray.MipSlice = mipIndex;
                        }
                        else
                        {
                            dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                            dsDesc.Texture2D.MipSlice = mipIndex;
                        }
                        slice[mipIndex].setDSV(dsDesc);
                    }

                    // RTV
                    if (useRTV)
                    {
                        if (isArr)
                        {
                            rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                            rtDesc.Texture2DArray.ArraySize = 1;
                            rtDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                            rtDesc.Texture2DArray.MipSlice = mipIndex;
                            rtDesc.Texture2DArray.PlaneSlice = 0;
                        }
                        else
                        {
                            rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                            rtDesc.Texture2D.MipSlice = mipIndex;
                            rtDesc.Texture2D.PlaneSlice = 0;
                        }
                        slice[mipIndex].setRTV(rtDesc);
                    }

                    // SRV
                    if (useSRV)
                    {
                        if (isArr)
                        {
                            srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                            srDesc.Texture2DArray.ArraySize = 1;
                            srDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                            srDesc.Texture2DArray.MipLevels = 1;
                            srDesc.Texture2DArray.MostDetailedMip = mipIndex;
                            srDesc.Texture2DArray.ResourceMinLODClamp = 0;
                            srDesc.Texture2DArray.PlaneSlice = 0;
                        }
                        else
                        {
                            srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                            srDesc.Texture2D.MipLevels = 1;
                            srDesc.Texture2D.MostDetailedMip = mipIndex;
                            srDesc.Texture2D.ResourceMinLODClamp = 0;
                            srDesc.Texture2D.PlaneSlice = 0;
                        }
                        slice[mipIndex].setSRV(srDesc);
                    }
                }
            }
        }

        // Read-only depth-stencil
        if ((m_desc.flags & GfxTextureFlags::kReadOnlyDepthView) != GfxTextureFlags::kNone)
        {
            m_handleReadOnlyDepth.init(this, m_device, this, fmt, msaa);
            m_handleReadOnlyDepth.readOnlyDepthView = true;
            if (useDSV)
            {
                if (isCube)
                {
                    dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsDesc.Texture2DArray.MipSlice = 0;
                    dsDesc.Texture2DArray.FirstArraySlice = 0;
                    dsDesc.Texture2DArray.ArraySize = arrSize * 6;
                }
                else if (isMsaa)
                {
                    dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    dsDesc.Texture2D.MipSlice = 0;
                }
                dsDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
                if (GfxPixelFormat::hasStencil(fmt))
                    dsDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
                m_handleReadOnlyDepth.setDSV(dsDesc);
            }
            TK_ASSERT(!useRTV);
            if (useSRV)
            {
                if (isCube)
                {
                    srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    srDesc.TextureCube.MostDetailedMip = 0;
                    srDesc.TextureCube.MipLevels = mipLevelsCount;
                    srDesc.TextureCube.ResourceMinLODClamp = 0;
                }
                else if (isMsaa)
                {
                    srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srDesc.Texture2D.MostDetailedMip = 0;
                    srDesc.Texture2D.MipLevels = mipLevelsCount;
                    srDesc.Texture2D.ResourceMinLODClamp = 0;
                    srDesc.Texture2D.PlaneSlice = 0;
                }
                m_handleReadOnlyDepth.setSRV(srDesc);
            }
        }
    }

}

#endif
