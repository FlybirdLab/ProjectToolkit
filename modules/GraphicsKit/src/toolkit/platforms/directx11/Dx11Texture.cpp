/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11Texture.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/graphics/utils/TextureData.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    bool DX11Texture::onInit()
    {
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

        // Create resource
        HRESULT result;
        if (isVolume())
        {
            // Create texture description
            D3D11_TEXTURE3D_DESC textureDesc;
            textureDesc.MipLevels = mipLevels();
            textureDesc.Format = dxgiFormat;
            textureDesc.Width = width();
            textureDesc.Height = height();
            textureDesc.Depth = depth();
            textureDesc.BindFlags = 0;
            textureDesc.CPUAccessFlags = DxUtils::getDX11CpuAccessFlagsFromUsage(m_desc.usage);
            textureDesc.MiscFlags = 0;
            textureDesc.Usage = DxUtils::toD3D11Usage(m_desc.usage);
            if (useSRV)
                textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            if (useRTV)
                textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            if (useDSV)
                textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            if (useUAV)
                textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

            // Create texture
            ID3D11Texture3D *texture = nullptr;
            result = device->CreateTexture3D(&textureDesc, nullptr, &texture);
            m_resource = texture;
        }
        else
        {
            // Create texture description
            D3D11_TEXTURE2D_DESC textureDesc;
            textureDesc.MipLevels = mipLevels();
            textureDesc.Format = dxgiFormat;
            textureDesc.Width = width();
            textureDesc.Height = height();
            textureDesc.Usage = DxUtils::toD3D11Usage(m_desc.usage);
            textureDesc.BindFlags = 0;
            textureDesc.CPUAccessFlags = DxUtils::getDX11CpuAccessFlagsFromUsage(m_desc.usage);
            textureDesc.ArraySize = arraySize();
            textureDesc.SampleDesc.Count = static_cast<UINT>(m_desc.multiSampleLevel);
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.MiscFlags = 0;
            if (useSRV)
                textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            if (useRTV)
                textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            if (useDSV)
                textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            if (useUAV)
                textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            if (m_device->getBackendType() >= GfxBackendType::kDirectX11 && isMultiSample())
                textureDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
            if (isCubeMap())
                textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

            // Create texture
            ID3D11Texture2D *texture = nullptr;
            result = device->CreateTexture2D(&textureDesc, nullptr, &texture);
            m_resource = texture;
        }
        LOG_DIRECTX_RESULT_WITH_RETURN(result, false);
        TK_ASSERT(m_resource != nullptr);
        DX_SET_DEBUG_NAME(m_resource, getName());

        // Update memory usage
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

    void DX11Texture::onResidentMipsChange()
    {
        const i32 firstMipIndex = mipLevels() - residentMipLevels();
        const i32 mipLevelsCount = residentMipLevels();
        D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
        srDesc.Format = m_dxgiFormatSRV;
        if (isCubeMap())
        {
            srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srDesc.TextureCube.MostDetailedMip = firstMipIndex;
            srDesc.TextureCube.MipLevels = mipLevelsCount;
        }
        else if (isVolume())
        {
            srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            srDesc.Texture3D.MostDetailedMip = firstMipIndex;
            srDesc.Texture3D.MipLevels = mipLevelsCount;
        }
        else
        {
            srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srDesc.Texture2D.MostDetailedMip = firstMipIndex;
            srDesc.Texture2D.MipLevels = mipLevelsCount;
        }
        ID3D11ShaderResourceView *srView = nullptr;
        if (mipLevelsCount != 0)
            VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateShaderResourceView(m_resource, &srDesc, &srView));
        DX11TextureView &v = isVolume() ? m_handleVolume : m_handlesPerSlice[0];
        if (v.getParent() == nullptr)
            v.init(this, nullptr, srView, nullptr, nullptr, format(), multiSampleLevel());
        else
            v.setSRV(srView);
    }

    void DX11Texture::onReleaseData()
    {
        m_handlesPerMip.resize(0, false);
        m_handlesPerSlice.resize(0, false);
        m_handleArray.release();
        m_handleVolume.release();
        m_handleReadOnlyDepth.release();
        DX_DEL(m_resource, 0);

        // Base
        GfxTexture::onReleaseData();
    }

#define CLEAR_VIEWS()  \
    rtView = nullptr;  \
    srView = nullptr;  \
    dsView = nullptr;  \
    uaView = nullptr

    void DX11Texture::initHandles()
    {
        ID3D11RenderTargetView *rtView;
        ID3D11ShaderResourceView *srView;
        ID3D11DepthStencilView *dsView;
        ID3D11UnorderedAccessView *uaView;
        D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
        D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
        D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
        D3D11_UNORDERED_ACCESS_VIEW_DESC uaDesc;

        CLEAR_VIEWS();

        rtDesc.Format = m_dxgiFormatRTV;
        srDesc.Format = m_dxgiFormatSRV;
        dsDesc.Format = m_dxgiFormatDSV;
        dsDesc.Flags = 0;
        uaDesc.Format = m_dxgiFormatUAV;

        // Cache properties
        auto device = m_device->getDevice();
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
        if (isVol)
        {
            // Create handle for whole 3d texture
            if (useSRV)
            {
                srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                srDesc.Texture3D.MostDetailedMip = 0;
                srDesc.Texture3D.MipLevels = mipLevelsCount;
                VALIDATE_DIRECTX_CALL(device->CreateShaderResourceView(m_resource, &srDesc, &srView));
            }
            if (useRTV)
            {
                rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                rtDesc.Texture3D.MipSlice = 0;
                rtDesc.Texture3D.FirstWSlice = 0;
                rtDesc.Texture3D.WSize = depth();
                VALIDATE_DIRECTX_CALL(device->CreateRenderTargetView(m_resource, &rtDesc, &rtView));
            }
            if (useUAV)
            {
                uaDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
                uaDesc.Texture3D.MipSlice = 0;
                uaDesc.Texture3D.WSize = depth();
                uaDesc.Texture3D.FirstWSlice = 0;
                VALIDATE_DIRECTX_CALL(device->CreateUnorderedAccessView(m_resource, &uaDesc, &uaView));
            }
            m_handleVolume.init(this, rtView, srView, nullptr, uaView, fmt, msaa);

            // Init per slice views
            m_handlesPerSlice.resize(depth(), false);
            if (m_desc.hasPerSliceViews() && useRTV)
            {
                rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                rtDesc.Texture3D.MipSlice = 0;
                rtDesc.Texture3D.WSize = 1;

                for (i32 sliceIndex = 0; sliceIndex < depth(); sliceIndex++)
                {
                    rtDesc.Texture3D.FirstWSlice = sliceIndex;
                    VALIDATE_DIRECTX_CALL(device->CreateRenderTargetView(m_resource, &rtDesc, &rtView));
                    m_handlesPerSlice[sliceIndex].init(this, rtView, nullptr, nullptr, nullptr, fmt, msaa);
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
                CLEAR_VIEWS();

                if (useDSV)
                {
                    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsDesc.Texture2DArray.ArraySize = 1;
                    dsDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                    dsDesc.Texture2DArray.MipSlice = 0;
                    VALIDATE_DIRECTX_CALL(device->CreateDepthStencilView(m_resource, &dsDesc, &dsView));
                }
                if (useRTV)
                {
                    rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtDesc.Texture2DArray.ArraySize = 1;
                    rtDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                    rtDesc.Texture2DArray.MipSlice = 0;
                    VALIDATE_DIRECTX_CALL(device->CreateRenderTargetView(m_resource, &rtDesc, &rtView));
                }
                if (useSRV)
                {
                    // When GetFeatureLevel returns D3D_FEATURE_LEVEL_10_0 or less, Resources created with D3D11_RESOURCE_MISC_TEXTURECUBE may only be treated as cubemap ShaderResourceViews.
                    if (isCube && m_device->getBackendType() != GfxBackendType::kDirectX10)
                    {
                        srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                        srDesc.Texture2DArray.ArraySize = 1;
                        srDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                        srDesc.Texture2DArray.MipLevels = mipLevelsCount;
                        srDesc.Texture2DArray.MostDetailedMip = 0;
                        VALIDATE_DIRECTX_CALL(device->CreateShaderResourceView(m_resource, &srDesc, &srView));
                    }
                }

                m_handlesPerSlice[arrayIndex].init(this, rtView, srView, dsView, nullptr, fmt, msaa);
            }

            // Create whole array handle
            {
                CLEAR_VIEWS();

                if (useDSV)
                {
                    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsDesc.Texture2DArray.ArraySize = arrSize;
                    dsDesc.Texture2DArray.FirstArraySlice = 0;
                    dsDesc.Texture2DArray.MipSlice = 0;
                    VALIDATE_DIRECTX_CALL(device->CreateDepthStencilView(m_resource, &dsDesc, &dsView));
                }
                if (useRTV)
                {
                    rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtDesc.Texture2DArray.ArraySize = arrSize;
                    rtDesc.Texture2DArray.FirstArraySlice = 0;
                    rtDesc.Texture2DArray.MipSlice = 0;
                    VALIDATE_DIRECTX_CALL(device->CreateRenderTargetView(m_resource, &rtDesc, &rtView));
                }
                if (useSRV)
                {
                    if (isCube)
                    {
                        srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                        srDesc.TextureCube.MostDetailedMip = 0;
                        srDesc.TextureCube.MipLevels = mipLevelsCount;
                    }
                    else
                    {
                        srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                        srDesc.Texture2DArray.ArraySize = arrSize;
                        srDesc.Texture2DArray.FirstArraySlice = 0;
                        srDesc.Texture2DArray.MipLevels = mipLevelsCount;
                        srDesc.Texture2DArray.MostDetailedMip = 0;
                    }
                    VALIDATE_DIRECTX_CALL(device->CreateShaderResourceView(m_resource, &srDesc, &srView));
                }
                if (useUAV)
                {
                    uaDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                    uaDesc.Texture2DArray.MipSlice = 0;
                    uaDesc.Texture2DArray.ArraySize = arrSize;
                    uaDesc.Texture2DArray.FirstArraySlice = 0;
                    VALIDATE_DIRECTX_CALL(device->CreateUnorderedAccessView(m_resource, &uaDesc, &uaView));
                }
                m_handleArray.init(this, rtView, srView, dsView, uaView, fmt, msaa);
            }
        }
        else
        {
            // Resize handles
            m_handlesPerSlice.resize(1, false);
            CLEAR_VIEWS();

            // Create single handle for the whole texture
            if (useDSV)
            {
                if (isCube)
                {
                    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsDesc.Texture2DArray.MipSlice = 0;
                    dsDesc.Texture2DArray.FirstArraySlice = 0;
                    dsDesc.Texture2DArray.ArraySize = arrSize * 6;
                }
                else if (isMsaa)
                {
                    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                    dsDesc.Texture2D.MipSlice = 0;
                }
                VALIDATE_DIRECTX_CALL(device->CreateDepthStencilView(m_resource, &dsDesc, &dsView));
            }
            if (useRTV)
            {
                if (isCube)
                {
                    rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtDesc.Texture2DArray.MipSlice = 0;
                    rtDesc.Texture2DArray.FirstArraySlice = 0;
                    rtDesc.Texture2DArray.ArraySize = arrSize * 6;
                }
                else if (isMsaa)
                {
                    rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                    rtDesc.Texture2D.MipSlice = 0;
                }
                VALIDATE_DIRECTX_CALL(device->CreateRenderTargetView(m_resource, &rtDesc, &rtView));
            }
            if (useSRV)
            {
                if (isCube)
                {
                    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                    srDesc.TextureCube.MostDetailedMip = 0;
                    srDesc.TextureCube.MipLevels = mipLevelsCount;
                }
                else if (isMsaa)
                {
                    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    srDesc.Texture2D.MostDetailedMip = 0;
                    srDesc.Texture2D.MipLevels = mipLevelsCount;
                }
                VALIDATE_DIRECTX_CALL(device->CreateShaderResourceView(m_resource, &srDesc, &srView));
            }
            if (useUAV)
            {
                uaDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                uaDesc.Texture2D.MipSlice = 0;
                VALIDATE_DIRECTX_CALL(device->CreateUnorderedAccessView(m_resource, &uaDesc, &uaView));
            }
            m_handlesPerSlice[0].init(this, rtView, srView, dsView, uaView, fmt, msaa);
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
                    dsView = nullptr;
                    rtView = nullptr;
                    srView = nullptr;

                    // DSV
                    if (useDSV)
                    {
                        dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                        dsDesc.Texture2DArray.ArraySize = 1;
                        dsDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                        dsDesc.Texture2DArray.MipSlice = mipIndex;
                        m_device->getDevice()->CreateDepthStencilView(m_resource, &dsDesc, &dsView);
                    }

                    // RTV
                    if (useRTV)
                    {
                        rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                        rtDesc.Texture2DArray.ArraySize = 1;
                        rtDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                        rtDesc.Texture2DArray.MipSlice = mipIndex;
                        m_device->getDevice()->CreateRenderTargetView(m_resource, &rtDesc, &rtView);
                    }

                    // SRV
                    if (useSRV)
                    {
                        if ((isCube && m_device->getBackendType() == GfxBackendType::kDirectX10) == false)
                        {
                            srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                            srDesc.Texture2DArray.ArraySize = 1;
                            srDesc.Texture2DArray.FirstArraySlice = arrayIndex;
                            srDesc.Texture2DArray.MipLevels = 1;
                            srDesc.Texture2DArray.MostDetailedMip = mipIndex;
                            m_device->getDevice()->CreateShaderResourceView(m_resource, &srDesc, &srView);
                        }
                    }

                    slice[mipIndex].init(this, rtView, srView, dsView, nullptr, fmt, msaa);
                }
            }
        }

        // Read-only depth-stencil
        if ((m_desc.flags & GfxTextureFlags::kReadOnlyDepthView) != GfxTextureFlags::kNone)
        {
            CLEAR_VIEWS();

            // Create single handle for the whole texture
            if (useDSV)
            {
                if (isCube)
                {
                    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsDesc.Texture2DArray.MipSlice = 0;
                    dsDesc.Texture2DArray.FirstArraySlice = 0;
                    dsDesc.Texture2DArray.ArraySize = arrSize * 6;
                }
                else if (isMsaa)
                {
                    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                    dsDesc.Texture2D.MipSlice = 0;
                }
                dsDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
                if (GfxPixelFormat::hasStencil(fmt))
                    dsDesc.Flags |= D3D11_DSV_READ_ONLY_STENCIL;
                VALIDATE_DIRECTX_CALL(device->CreateDepthStencilView(m_resource, &dsDesc, &dsView));
            }
            TK_ASSERT(!useRTV);
            rtView = nullptr;
            if (useSRV)
            {
                if (isCube)
                {
                    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                    srDesc.TextureCube.MostDetailedMip = 0;
                    srDesc.TextureCube.MipLevels = mipLevelsCount;
                }
                else if (isMsaa)
                {
                    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    srDesc.Texture2D.MostDetailedMip = 0;
                    srDesc.Texture2D.MipLevels = mipLevelsCount;
                }
                VALIDATE_DIRECTX_CALL(device->CreateShaderResourceView(m_resource, &srDesc, &srView));
            }
            m_handleReadOnlyDepth.init(this, rtView, srView, dsView, nullptr, fmt, msaa);
        }
    }

    bool DX11Texture::getData(i32 arrayIndex, i32 mipMapIndex, TextureMipData &data, u32 mipRowPitch)
    {
        if (!isStaging())
        {
            LogWarn("Texture::getData is valid only for staging resources.");
            return false;
        }
        GfxDeviceLock lock(m_device);

        // Map the staging resource mip map for reading
        const u32 subresource = DxUtils::calcSubresourceIndex(mipMapIndex, arrayIndex, mipLevels());
        D3D11_MAPPED_SUBRESOURCE mapped;
        const HRESULT mapResult = m_device->getIM()->Map(m_resource, subresource, D3D11_MAP_READ, 0, &mapped);
        if (FAILED(mapResult))
        {
            LOG_DIRECTX_RESULT(mapResult);
            return false;
        }

        data.copy(mapped.pData, mapped.RowPitch, mapped.DepthPitch, depth(), mipRowPitch);

        // Unmap texture
        m_device->getIM()->Unmap(m_resource, subresource);

        return true;
    }

}

#endif
