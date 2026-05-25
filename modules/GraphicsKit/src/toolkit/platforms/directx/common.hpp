/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>

// Common
#if GFX_API_DIRECTX11 || GFX_API_DIRECTX12

#include <toolkit/platforms/win32/IncludeWindows.hpp>
#include <toolkit/platforms/win32/ComPointer.hpp>

// Helper define to dispose the COM object with remaining references counter checking
#define DX_DEL(x, refs)            \
    if (x)                         \
    {                              \
        auto res = (x)->Release(); \
        (x) = nullptr;             \
        TK_CHECK(res == refs);     \
    }

#endif

#define GFX_API_DIRECTX (GFX_API_DIRECTX11 || GFX_API_DIRECTX12)

// D3D11 and D3D12
#if GFX_API_DIRECTX11 || GFX_API_DIRECTX12

#if GFX_API_DIRECTX12
#include <D3D12.h>
#endif
#include <d3dcommon.h>
#if GFX_API_DIRECTX11
#define D3D11_NO_HELPERS
#include <D3D11.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#endif
#if GFX_API_DIRECTX12
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#endif

#pragma comment(lib, "DXGI.lib")

namespace toolkit
{
    typedef IUnknown IGraphicsUnknown;
}

#endif
