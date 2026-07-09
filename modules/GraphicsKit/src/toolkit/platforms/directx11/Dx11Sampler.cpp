/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11Sampler.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    static D3D11_TEXTURE_ADDRESS_MODE toDX11(GfxSamplerAddressMode value)
    {
        switch (value)
        {
        case GfxSamplerAddressMode::kWrap:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case GfxSamplerAddressMode::kClamp:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        case GfxSamplerAddressMode::kMirror:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case GfxSamplerAddressMode::kBorder:
            return D3D11_TEXTURE_ADDRESS_BORDER;
        default:
            return (D3D11_TEXTURE_ADDRESS_MODE)-1;
        }
    }

    bool DX11Sampler::onInit()
    {
        D3D11_SAMPLER_DESC samplerDesc;
        if (m_desc.comparisonFunction == GfxSamplerCompareFunction::kNever)
        {
            switch (m_desc.filter)
            {
            case GfxSamplerFilter::kPoint:
                samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
                break;
            case GfxSamplerFilter::kBilinear:
                samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                break;
            case GfxSamplerFilter::kTrilinear:
                samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                break;
            case GfxSamplerFilter::kAnisotropic:
                samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
                break;
            default:
                return false;
            }
        }
        else
        {
            switch (m_desc.filter)
            {
            case GfxSamplerFilter::kPoint:
                samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                break;
            case GfxSamplerFilter::kBilinear:
                samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
                break;
            case GfxSamplerFilter::kTrilinear:
                samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
                break;
            case GfxSamplerFilter::kAnisotropic:
                samplerDesc.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
                break;
            default:
                return false;
            }
        }
        samplerDesc.AddressU = toDX11(m_desc.addressU);
        samplerDesc.AddressV = toDX11(m_desc.addressV);
        samplerDesc.AddressW = toDX11(m_desc.addressW);
        samplerDesc.MipLODBias = m_desc.mipBias;
        samplerDesc.MaxAnisotropy = m_desc.maxAnisotropy;
        switch (m_desc.comparisonFunction)
        {
        case GfxSamplerCompareFunction::kNever:
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            break;
        case GfxSamplerCompareFunction::kLess:
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
            break;
        default:
            return false;
        }
        switch (m_desc.borderColor)
        {
        case GfxSamplerBorderColor::kTransparentBlack:
            samplerDesc.BorderColor[0] = 0;
            samplerDesc.BorderColor[1] = 0;
            samplerDesc.BorderColor[2] = 0;
            samplerDesc.BorderColor[3] = 0;
            break;
        case GfxSamplerBorderColor::kOpaqueBlack:
            samplerDesc.BorderColor[0] = 0;
            samplerDesc.BorderColor[1] = 0;
            samplerDesc.BorderColor[2] = 0;
            samplerDesc.BorderColor[3] = 1.0f;
            break;
        case GfxSamplerBorderColor::kOpaqueWhite:
            samplerDesc.BorderColor[0] = 1.0f;
            samplerDesc.BorderColor[1] = 1.0f;
            samplerDesc.BorderColor[2] = 1.0f;
            samplerDesc.BorderColor[3] = 1.0f;
            break;
        default:
            return false;
        }
        samplerDesc.MinLOD = m_desc.minMipLevel;
        samplerDesc.MaxLOD = m_desc.maxMipLevel;
        HRESULT result = m_device->getDevice()->CreateSamplerState(&samplerDesc, &samplerState);
        LOG_DIRECTX_RESULT_WITH_RETURN(result, false);
        TK_ASSERT(samplerState != nullptr);
        m_memoryUsage = sizeof(D3D11_SAMPLER_DESC);

        return true;
    }

    void DX11Sampler::onReleaseData()
    {
        DX_DEL(samplerState, 0);
        GfxSampler::onReleaseData();
    }

}

#endif
