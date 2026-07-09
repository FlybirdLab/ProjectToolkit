/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/directx/DxUtils.hpp>

#if GFX_API_DIRECTX11 || GFX_API_DIRECTX12

#include "toolkit/graphics/GfxDevice.hpp"
#include "toolkit/types/StringBuilder.hpp"
#include <winerror.h>

namespace toolkit
{
    DXGI_FORMAT s_pixelFormatToDXGIFormat[110] =
        {
            DXGI_FORMAT_UNKNOWN,
            DXGI_FORMAT_R32G32B32A32_TYPELESS,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            DXGI_FORMAT_R32G32B32A32_UINT,
            DXGI_FORMAT_R32G32B32A32_SINT,
            DXGI_FORMAT_R32G32B32_TYPELESS,
            DXGI_FORMAT_R32G32B32_FLOAT,
            DXGI_FORMAT_R32G32B32_UINT,
            DXGI_FORMAT_R32G32B32_SINT,
            DXGI_FORMAT_R16G16B16A16_TYPELESS,
            DXGI_FORMAT_R16G16B16A16_FLOAT,
            DXGI_FORMAT_R16G16B16A16_UNORM,
            DXGI_FORMAT_R16G16B16A16_UINT,
            DXGI_FORMAT_R16G16B16A16_SNORM,
            DXGI_FORMAT_R16G16B16A16_SINT,
            DXGI_FORMAT_R32G32_TYPELESS,
            DXGI_FORMAT_R32G32_FLOAT,
            DXGI_FORMAT_R32G32_UINT,
            DXGI_FORMAT_R32G32_SINT,
            DXGI_FORMAT_R32G8X24_TYPELESS,
            DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
            DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
            DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
            DXGI_FORMAT_R10G10B10A2_TYPELESS,
            DXGI_FORMAT_R10G10B10A2_UNORM,
            DXGI_FORMAT_R10G10B10A2_UINT,
            DXGI_FORMAT_R11G11B10_FLOAT,
            DXGI_FORMAT_R8G8B8A8_TYPELESS,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            DXGI_FORMAT_R8G8B8A8_UINT,
            DXGI_FORMAT_R8G8B8A8_SNORM,
            DXGI_FORMAT_R8G8B8A8_SINT,
            DXGI_FORMAT_R16G16_TYPELESS,
            DXGI_FORMAT_R16G16_FLOAT,
            DXGI_FORMAT_R16G16_UNORM,
            DXGI_FORMAT_R16G16_UINT,
            DXGI_FORMAT_R16G16_SNORM,
            DXGI_FORMAT_R16G16_SINT,
            DXGI_FORMAT_R32_TYPELESS,
            DXGI_FORMAT_D32_FLOAT,
            DXGI_FORMAT_R32_FLOAT,
            DXGI_FORMAT_R32_UINT,
            DXGI_FORMAT_R32_SINT,
            DXGI_FORMAT_R24G8_TYPELESS,
            DXGI_FORMAT_D24_UNORM_S8_UINT,
            DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
            DXGI_FORMAT_X24_TYPELESS_G8_UINT,
            DXGI_FORMAT_R8G8_TYPELESS,
            DXGI_FORMAT_R8G8_UNORM,
            DXGI_FORMAT_R8G8_UINT,
            DXGI_FORMAT_R8G8_SNORM,
            DXGI_FORMAT_R8G8_SINT,
            DXGI_FORMAT_R16_TYPELESS,
            DXGI_FORMAT_R16_FLOAT,
            DXGI_FORMAT_D16_UNORM,
            DXGI_FORMAT_R16_UNORM,
            DXGI_FORMAT_R16_UINT,
            DXGI_FORMAT_R16_SNORM,
            DXGI_FORMAT_R16_SINT,
            DXGI_FORMAT_R8_TYPELESS,
            DXGI_FORMAT_R8_UNORM,
            DXGI_FORMAT_R8_UINT,
            DXGI_FORMAT_R8_SNORM,
            DXGI_FORMAT_R8_SINT,
            DXGI_FORMAT_A8_UNORM,
            DXGI_FORMAT_R1_UNORM,
            DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
            DXGI_FORMAT_R8G8_B8G8_UNORM,
            DXGI_FORMAT_G8R8_G8B8_UNORM,
            DXGI_FORMAT_BC1_TYPELESS,
            DXGI_FORMAT_BC1_UNORM,
            DXGI_FORMAT_BC1_UNORM_SRGB,
            DXGI_FORMAT_BC2_TYPELESS,
            DXGI_FORMAT_BC2_UNORM,
            DXGI_FORMAT_BC2_UNORM_SRGB,
            DXGI_FORMAT_BC3_TYPELESS,
            DXGI_FORMAT_BC3_UNORM,
            DXGI_FORMAT_BC3_UNORM_SRGB,
            DXGI_FORMAT_BC4_TYPELESS,
            DXGI_FORMAT_BC4_UNORM,
            DXGI_FORMAT_BC4_SNORM,
            DXGI_FORMAT_BC5_TYPELESS,
            DXGI_FORMAT_BC5_UNORM,
            DXGI_FORMAT_BC5_SNORM,
            DXGI_FORMAT_B5G6R5_UNORM,
            DXGI_FORMAT_B5G5R5A1_UNORM,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT_B8G8R8X8_UNORM,
            DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
            DXGI_FORMAT_B8G8R8A8_TYPELESS,
            DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            DXGI_FORMAT_B8G8R8X8_TYPELESS,
            DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
            DXGI_FORMAT_BC6H_TYPELESS,
            DXGI_FORMAT_BC6H_UF16,
            DXGI_FORMAT_BC6H_SF16,
            DXGI_FORMAT_BC7_TYPELESS,
            DXGI_FORMAT_BC7_UNORM,
            DXGI_FORMAT_BC7_UNORM_SRGB,
            DXGI_FORMAT_UNKNOWN, // ASTC_4x4_UNorm
            DXGI_FORMAT_UNKNOWN, // ASTC_4x4_UNorm_sRGB
            DXGI_FORMAT_UNKNOWN, // ASTC_6x6_UNorm
            DXGI_FORMAT_UNKNOWN, // ASTC_6x6_UNorm_sRGB
            DXGI_FORMAT_UNKNOWN, // ASTC_8x8_UNorm
            DXGI_FORMAT_UNKNOWN, // ASTC_8x8_UNorm_sRGB
            DXGI_FORMAT_UNKNOWN, // ASTC_10x10_UNorm
            DXGI_FORMAT_UNKNOWN, // ASTC_10x10_UNorm_sRGB
            DXGI_FORMAT_YUY2,
            DXGI_FORMAT_NV12,
    };

    DXGI_FORMAT DxUtils::toDxgiFormat(PixelFormat format)
    {
        return s_pixelFormatToDXGIFormat[(i32)format];
    }

    const char *DxUtils::getFeatureLevelString(D3D_FEATURE_LEVEL featureLevel)
    {
        switch (featureLevel)
        {
        case D3D_FEATURE_LEVEL_9_1:
            return "9.1";
        case D3D_FEATURE_LEVEL_9_2:
            return "9.2";
        case D3D_FEATURE_LEVEL_9_3:
            return "9.3";
        case D3D_FEATURE_LEVEL_10_0:
            return "10";
        case D3D_FEATURE_LEVEL_10_1:
            return "10.1";
        case D3D_FEATURE_LEVEL_11_0:
            return "11";
        case D3D_FEATURE_LEVEL_11_1:
            return "11.1";
#if GFX_API_DIRECTX12
        case D3D_FEATURE_LEVEL_12_0:
            return "12";
        case D3D_FEATURE_LEVEL_12_1:
            return "12.1";
#endif
        default:
            return "?";
        }
    }

    u32 DxUtils::countAdapterOutputs(IDXGIAdapter *adapter)
    {
        u32 count = 0;
        while (true)
        {
            IDXGIOutput *output;
            HRESULT hr = adapter->EnumOutputs(count, &output);
            if (FAILED(hr))
            {
                break;
            }
            count++;
        }
        return count;
    }

    static void formatD3DErrorString(HRESULT errorCode, TkStringBuilder &sb, HRESULT &removedReason)
    {
#define D3DERR(x)            \
    case x:                  \
        sb.append(#x); \
        break
        switch (errorCode)
        {
            // Windows
            D3DERR(S_OK);
            D3DERR(E_FAIL);
            D3DERR(E_INVALIDARG);
            D3DERR(E_OUTOFMEMORY);
            D3DERR(E_NOINTERFACE);
            D3DERR(E_NOTIMPL);

            // DirectX
#if WITH_D3DX_LIBS
            D3DERR(D3DERR_INVALIDCALL);
            D3DERR(D3DERR_WASSTILLDRAWING);
#endif

            // DirectX 11
            D3DERR(D3D11_ERROR_FILE_NOT_FOUND);
            D3DERR(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS);
            D3DERR(D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS);
            D3DERR(D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD);

            // DirectX 12
            // D3DERR(D3D12_ERROR_FILE_NOT_FOUND);
            // D3DERR(D3D12_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS);
            // D3DERR(D3D12_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS);

            // DXGI
            D3DERR(DXGI_ERROR_INVALID_CALL);
            D3DERR(DXGI_ERROR_NOT_FOUND);
            D3DERR(DXGI_ERROR_MORE_DATA);
            D3DERR(DXGI_ERROR_UNSUPPORTED);
            D3DERR(DXGI_ERROR_DEVICE_REMOVED);
            D3DERR(DXGI_ERROR_DEVICE_HUNG);
            D3DERR(DXGI_ERROR_DEVICE_RESET);
            D3DERR(DXGI_ERROR_WAS_STILL_DRAWING);
            D3DERR(DXGI_ERROR_FRAME_STATISTICS_DISJOINT);
            D3DERR(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE);
            D3DERR(DXGI_ERROR_DRIVER_INTERNAL_ERROR);
            D3DERR(DXGI_ERROR_NONEXCLUSIVE);
            D3DERR(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE);
            D3DERR(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED);
            D3DERR(DXGI_ERROR_REMOTE_OUTOFMEMORY);
            D3DERR(DXGI_ERROR_ACCESS_LOST);
            D3DERR(DXGI_ERROR_WAIT_TIMEOUT);
            D3DERR(DXGI_ERROR_SESSION_DISCONNECTED);
            D3DERR(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE);
            D3DERR(DXGI_ERROR_CANNOT_PROTECT_CONTENT);
            D3DERR(DXGI_ERROR_ACCESS_DENIED);
            D3DERR(DXGI_ERROR_NAME_ALREADY_EXISTS);
            D3DERR(DXGI_ERROR_SDK_COMPONENT_MISSING);
#if GFX_API_DIRECTX12
            D3DERR(DXGI_ERROR_NOT_CURRENT);
            D3DERR(DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY);
            D3DERR(D3D12_ERROR_DRIVER_VERSION_MISMATCH);
#endif

        default:
            sb.appendFormat("0x%x", static_cast<unsigned int>(errorCode));
            break;
        }
#undef D3DERR

        if (errorCode == DXGI_ERROR_DEVICE_REMOVED || errorCode == DXGI_ERROR_DEVICE_RESET || errorCode == DXGI_ERROR_DRIVER_INTERNAL_ERROR)
        {
            GfxDevice *device = GfxDevice::sInstance;
            const GfxBackendType rendererType = device ? device->getBackendType() : GfxBackendType::kUnknown;
            void *nativePtr = device ? device->getNativePtr() : nullptr;
#if GFX_API_DIRECTX12
            if (rendererType == GfxBackendType::kDirectX12 && nativePtr)
            {
                removedReason = ((ID3D12Device *)nativePtr)->GetDeviceRemovedReason();
            }
#endif
#if GFX_API_DIRECTX11
            if ((rendererType == GfxBackendType::kDirectX11 ||
                 rendererType == GfxBackendType::kDirectX10_1 ||
                 rendererType == GfxBackendType::kDirectX10) &&
                nativePtr)
            {
                removedReason = ((ID3D11Device *)nativePtr)->GetDeviceRemovedReason();
            }
#endif
            const char *reasonStr = nullptr;
            switch (removedReason)
            {
            case DXGI_ERROR_DEVICE_HUNG:
                reasonStr = "HUNG";
                break;
            case DXGI_ERROR_DEVICE_REMOVED:
                reasonStr = "REMOVED";
                break;
            case DXGI_ERROR_DEVICE_RESET:
                reasonStr = "RESET";
                break;
            case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
                reasonStr = "INTERNAL_ERROR";
                break;
            case DXGI_ERROR_INVALID_CALL:
                reasonStr = "INVALID_CALL";
                break;
            }
            if (reasonStr != nullptr)
                sb.appendFormat(", Device Removed Reason: %s", reasonStr);
        }
    }

    void DxUtils::logD3DResult(HRESULT result, const char *file, u32 line, bool fatal)
    {
        TK_ASSERT(FAILED(result));

        // Process error and format message
        TkStringBuilder sb;
        HRESULT removedReason = S_OK;
        sb.append("DirectX error: ");
        formatD3DErrorString(result, sb, removedReason);
        if (file)
            sb.append(" at ").append(file).append(':').append(line);
        const TkStringView msg(sb.toStringView());

        // Handle error
        FatalErrorType errorType = FatalErrorType::kNone;
        if (result == E_OUTOFMEMORY)
            errorType = FatalErrorType::kGPUOutOfMemory;
        else if (removedReason != S_OK)
        {
            errorType = FatalErrorType::kGPUCrash;
            if (removedReason == DXGI_ERROR_DEVICE_HUNG)
                errorType = FatalErrorType::kGPUHang;
        }
        if (errorType != FatalErrorType::kNone) {
            LogError("[%s] %s", toolkit::toString(errorType), msg.get());
            TK_ASSERT(false);
        }
        else {
            LogError("[%s] %s", toolkit::toString(errorType), msg.get());
        }
    }

    LPCSTR DxUtils::getVertexInputSemantic(VertexElementTypes type, UINT &semanticIndex)
    {
        static_assert((i32)VertexElementTypes::MAX == 16, "Update code below.");
        semanticIndex = 0;
        switch (type)
        {
        case VertexElementTypes::kPosition:
            return "POSITION";
        case VertexElementTypes::kColor:
            return "COLOR";
        case VertexElementTypes::kNormal:
            return "NORMAL";
        case VertexElementTypes::kTangent:
            return "TANGENT";
        case VertexElementTypes::kBlendIndices:
            return "BLENDINDICES";
        case VertexElementTypes::kBlendWeights:
            return "BLENDWEIGHTS";
        case VertexElementTypes::kTexCoord0:
            return "TEXCOORD";
        case VertexElementTypes::kTexCoord1:
            semanticIndex = 1;
            return "TEXCOORD";
        case VertexElementTypes::kTexCoord2:
            semanticIndex = 2;
            return "TEXCOORD";
        case VertexElementTypes::kTexCoord3:
            semanticIndex = 3;
            return "TEXCOORD";
        case VertexElementTypes::kTexCoord4:
            semanticIndex = 4;
            return "TEXCOORD";
        case VertexElementTypes::kTexCoord5:
            semanticIndex = 5;
            return "TEXCOORD";
        case VertexElementTypes::kTexCoord6:
            semanticIndex = 6;
            return "TEXCOORD";
        case VertexElementTypes::kTexCoord7:
            semanticIndex = 7;
            return "TEXCOORD";
        case VertexElementTypes::kAttribute0:
            return "ATTRIBUTE";
        case VertexElementTypes::kAttribute1:
            semanticIndex = 1;
            return "ATTRIBUTE";
        case VertexElementTypes::kAttribute2:
            semanticIndex = 2;
            return "ATTRIBUTE";
        case VertexElementTypes::kAttribute3:
            semanticIndex = 3;
            return "ATTRIBUTE";
        case VertexElementTypes::kLightmap:
            return "LIGHTMAP";
        default:
            LogError("Invalid vertex shader element semantic type");
            return "";
        }
    }

}

#endif
