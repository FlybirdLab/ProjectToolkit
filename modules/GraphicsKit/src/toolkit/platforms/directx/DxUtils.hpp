/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>

#if GFX_API_DIRECTX12 || GFX_API_DIRECTX11

#include <toolkit/graphics/utils/GfxUtils.hpp>
#include <toolkit/graphics/GfxShader.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/platforms/directx/common.hpp>

#include "toolkit/graphics/GfxVertexLayout.hpp"

#if TK_DEBUG || GFX_ENABLE_RESOURCE_NAMING

#include <toolkit/utils/Convertor.hpp>
#include <cstring>
// Link DXGI lib
#pragma comment(lib, "dxguid.lib")

#endif

namespace toolkit
{

    // Set of utilities for rendering on DirectX platform.
    class DxUtils
    {
    public:
#if GFX_API_DIRECTX11
        static inline D3D11_USAGE toD3D11Usage(const GfxResourceUsage usage)
        {
            switch (usage)
            {
            case GfxResourceUsage::kDynamic:
                return D3D11_USAGE_DYNAMIC;
            case GfxResourceUsage::kStaging:
            case GfxResourceUsage::kStagingUpload:
            case GfxResourceUsage::kStagingReadback:
                return D3D11_USAGE_STAGING;
            default:
                return D3D11_USAGE_DEFAULT;
            }
        }

        static inline UINT getDX11CpuAccessFlagsFromUsage(const GfxResourceUsage usage)
        {
            switch (usage)
            {
            case GfxResourceUsage::kDynamic:
                return D3D11_CPU_ACCESS_WRITE;
            case GfxResourceUsage::kStaging:
                return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            case GfxResourceUsage::kStagingReadback:
                return D3D11_CPU_ACCESS_READ;
            case GfxResourceUsage::kStagingUpload:
                return D3D11_CPU_ACCESS_WRITE;
            default:
                return 0;
            }
        }
#endif
    public:
        // Calculate a subresource index for a texture
        static TK_FORCE_INLINE u32 calcSubresourceIndex(u32 mipSlice, u32 arraySlice, u32 mipLevels)
        {
            return mipSlice + arraySlice * mipLevels;
        }

        static DXGI_FORMAT toDxgiFormat(PixelFormat format);
        static const char *getFeatureLevelString(D3D_FEATURE_LEVEL featureLevel);
        static u32 countAdapterOutputs(IDXGIAdapter *adapter);
        static void logD3DResult(HRESULT result, const char *file = nullptr, u32 line = 0, bool fatal = false);
        static LPCSTR getVertexInputSemantic(VertexElementTypes type, UINT &semanticIndex);
    };

// DirectX results validation
#define VALIDATE_DIRECTX_CALL(x)                                     \
    {                                                                \
        HRESULT result = x;                                          \
        if (FAILED(result))                                          \
            DxUtils::logD3DResult(result, __FILE__, __LINE__, true); \
    }
#define LOG_DIRECTX_RESULT(result) \
    if (FAILED(result))            \
    DxUtils::logD3DResult(result, __FILE__, __LINE__)
#define LOG_DIRECTX_RESULT_WITH_RETURN(result, returnValue) \
    if (FAILED(result))                                     \
    {                                                       \
        DxUtils::logD3DResult(result, __FILE__, __LINE__);  \
        return returnValue;                                 \
    }

#if GFX_ENABLE_RESOURCE_NAMING

    template <typename T>
    inline void setDebugObjectName(T *resource, const char *data, UINT size)
    {
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, size, data);
    }

    template <u32 NameLength>
    inline void setDebugObjectName(IDXGIObject *resource, const char (&name)[NameLength])
    {
        setDebugObjectName(resource, name, NameLength - 1);
    }

#if GFX_API_DIRECTX11

    template <u32 NameLength>
    inline void setDebugObjectName(ID3D10DeviceChild *resource, const char (&name)[NameLength])
    {
        setDebugObjectName(resource, name, NameLength - 1);
    }

    template <u32 NameLength>
    inline void setDebugObjectName(ID3D11DeviceChild *resource, const char (&name)[NameLength])
    {
        setDebugObjectName(resource, name, NameLength - 1);
    }
#endif

#if GFX_API_DIRECTX12

    template <u32 NameLength>
    inline void setDebugObjectName(ID3D12DeviceChild *resource, const char (&name)[NameLength])
    {
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, NameLength - 1, name);
    }

#endif

#if GFX_API_DIRECTX11

    template <u32 NameLength>
    inline void setDebugObjectName(ID3D10Resource *resource, const char (&name)[NameLength])
    {
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, NameLength - 1, name);
    }

    template <u32 NameLength>
    inline void setDebugObjectName(ID3D11Resource *resource, const char (&name)[NameLength])
    {
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, NameLength - 1, name);
    }

#endif

#if GFX_API_DIRECTX12

    template <u32 NameLength>
    inline void setDebugObjectName(ID3D12Resource *resource, const char (&name)[NameLength])
    {
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, NameLength - 1, name);
    }

#endif

    template <typename T, u32 NameLength>
    inline void setDebugObjectName(ComPtr<T> resource, const char (&name)[NameLength])
    {
        setDebugObjectName(resource.get(), name);
    }

    // setDebugObjectName - UNICODE
    // size = source wide-character count (NOT bytes)
    template <typename T>
    inline void setDebugObjectName(T *resource, const wchar *data, UINT size)
    {
        // utf16ToAnsi may emit up to 4 UTF-8 bytes per source wchar plus a null terminator.
        const UINT bufSize = size * 4 + 1;
        char *ansi = (char *)TkMemory::allocate(TkMemory::getDefault(), bufSize);
        TkConvertor::utf16ToAnsi(data, ansi, (i32)size);
        // utf16ToAnsi terminates the output itself; use strlen for the actual byte length.
        const UINT actualBytes = (UINT)::strlen(ansi);
        setDebugObjectName(resource, ansi, actualBytes);
        TkMemory::free(TkMemory::getDefault(), ansi);
    }

    template <typename T>
    inline void setDebugObjectName(T *resource, const TkString &name)
    {
        setDebugObjectName(resource, *name, name.length());
    }

    template <typename T>
    inline void setDebugObjectName(ComPtr<T> resource, const wchar *data, UINT size)
    {
        setDebugObjectName(resource.get(), data, size);
    }

    template <typename T, u32 NameLength>
    inline void setDebugObjectName(T *resource, const wchar (&name)[NameLength])
    {
        setDebugObjectName(resource, name, NameLength - 1);
    }

    template <typename T, u32 NameLength>
    inline void setDebugObjectName(ComPtr<T> resource, const wchar (&name)[NameLength])
    {
        setDebugObjectName(resource.get(), name);
    }

    template <typename T>
    inline void setDebugObjectName(ComPtr<T> resource, const TkString &name)
    {
        setDebugObjectName(resource.get(), *name, name.length());
    }

    // Macros

#define DX_SET_DEBUG_NAME(resource, name) setDebugObjectName(resource, name)
#define DX_SET_DEBUG_NAME_EX(resource, parent, type, id)                    \
    {                                                                       \
        TkString dx_s = TkString::format("%s:%s%i", parent, type, id); \
        StringAsUTF16<> s(dx_s.get()); \
        setDebugObjectName(resource, s.get(), s.length());             \
    }

#else

#define DX_SET_DEBUG_NAME(resource, name)
#define DX_SET_DEBUG_NAME_EX(resource, parent, type, id)

#endif

}

#endif
