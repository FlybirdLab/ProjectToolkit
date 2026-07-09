/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11SwapChain.hpp"

#if GFX_API_DIRECTX11

#include "Dx11Context.hpp"
#include "toolkit/graphics/GfxWindow.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    DX11SwapChain::DX11SwapChain(DX11Device *device, GfxWindow *window)
        : DX11Resource<GfxSwapChain>(device, TkStringView::Empty), m_windowHandle(static_cast<HWND>(window->getWindowPtr())), m_swapChain(nullptr), m_allowTearing(false), m_isFullscreen(false), m_backBuffer(nullptr)
    {
        TK_ASSERT(m_windowHandle);
        m_window = window;
    }

    void DX11SwapChain::getBackBuffer()
    {
        VALIDATE_DIRECTX_CALL(m_swapChain->GetBuffer(0, __uuidof(m_backBuffer), reinterpret_cast<void **>(&m_backBuffer)));

        ID3D11RenderTargetView *rtv = nullptr;
        ID3D11ShaderResourceView *srv = nullptr;
        VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateRenderTargetView(m_backBuffer, nullptr, &rtv));
#if GFX_USE_WINDOW_SRV
        VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateShaderResourceView(m_backBuffer, nullptr, &srv));
#endif

        m_backBufferHandle.init(this, rtv, srv, nullptr, nullptr, m_format, GfxMSAALevel::kNone);
    }

    void DX11SwapChain::releaseBackBuffer()
    {
        m_backBufferHandle.release();
        DX_DEL(m_backBuffer, 0);
    }

    void DX11SwapChain::onReleaseData()
    {
        if (m_swapChain)
        {
            VALIDATE_DIRECTX_CALL(m_swapChain->SetFullscreenState(false, nullptr));
        }

        releaseBackBuffer();
        DX_DEL(m_swapChain, 0);
        m_width = m_height = 0;
    }

    ID3D11Resource *DX11SwapChain::getResource()
    {
        return m_backBuffer;
    }

    bool DX11SwapChain::isFullscreen()
    {
        if (m_swapChain == nullptr)
            return false;

        BOOL state;
        VALIDATE_DIRECTX_CALL(m_swapChain->GetFullscreenState(&state, nullptr));
        return state == TRUE;
    }

    void DX11SwapChain::setFullscreen(bool isFullscreen)
    {
        if (m_swapChain && isFullscreen != this->isFullscreen())
        {
            m_device->waitForGPU();
            GfxDeviceLock lock(m_device);

            DXGI_SWAP_CHAIN_DESC swapChainDesc;
            m_swapChain->GetDesc(&swapChainDesc);

            IDXGIOutput *output = nullptr;
            if (isFullscreen && m_device->outputs.count() > 0)
            {
                const u32 outputIdx = 0;
                auto &outputDX = m_device->outputs[outputIdx];
                output = outputDX.output.get();
                swapChainDesc.BufferDesc = outputDX.desktopViewMode;
            }

            if (FAILED(m_swapChain->ResizeTarget(&swapChainDesc.BufferDesc)))
            {
                LogWarn("Swapchain resize failed.");
            }

            if (FAILED(m_swapChain->SetFullscreenState(isFullscreen, output)))
            {
                LogWarn("Cannot change fullscreen mode.");
            }

            m_isFullscreen = isFullscreen;

            if (swapChainDesc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL ||
                swapChainDesc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD)
            {
                const i32 width = m_width;
                const i32 height = m_height;
                m_width = m_height = 0;
                resize(width, height);
            }
        }
    }

    GfxTextureView *DX11SwapChain::getBackBufferView()
    {
        return &m_backBufferHandle;
    }

    bool DX11SwapChain::isReady() const
    {
        return m_swapChain != nullptr && m_width > 0 && m_height > 0;
    }

    void DX11SwapChain::present(bool vsync)
    {
        TK_ASSERT(m_swapChain);
        UINT presentFlags = 0;
        if (!vsync && !m_isFullscreen && m_allowTearing)
        {
            presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
        }
        const HRESULT result = m_swapChain->Present(vsync ? 1 : 0, presentFlags);
        LOG_DIRECTX_RESULT(result);

        onPresented();
    }

    bool DX11SwapChain::resize(i32 width, i32 height)
    {
        if (width == m_width && height == m_height)
        {
            return false;
        }

        m_device->waitForGPU();
        GfxDeviceLock lock(m_device);
        m_allowTearing = m_device->m_allowTearing;
        m_format = GFX_BACK_BUFFER_PIXEL_FORMAT;

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        if (m_swapChain == nullptr)
        {
            onReleaseData();

            if (m_device->outputs.count() > 0)
            {
                const u32 outputIdx = 0;
                auto &output = m_device->outputs[outputIdx];
                swapChainDesc.BufferDesc = output.desktopViewMode;
            }
            else
            {
                swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
                swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
                swapChainDesc.BufferDesc.Format = DxUtils::toDxgiFormat(m_format);
                swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
                swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            }
            swapChainDesc.BufferDesc.Width = width;
            swapChainDesc.BufferDesc.Height = height;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = 2;
            swapChainDesc.OutputWindow = m_windowHandle;
            swapChainDesc.Windowed = TRUE;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            if (m_allowTearing)
            {
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            }
#if GFX_USE_WINDOW_SRV
            swapChainDesc.BufferUsage |= DXGI_USAGE_SHADER_INPUT;
#endif

            auto dxgi = m_device->getDXGIFactory();
            VALIDATE_DIRECTX_CALL(dxgi->CreateSwapChain(m_device->getDevice(), &swapChainDesc, &m_swapChain));
            TK_ASSERT(m_swapChain);

            VALIDATE_DIRECTX_CALL(dxgi->MakeWindowAssociation(m_windowHandle, DXGI_MWA_NO_ALT_ENTER));
        }
        else
        {
            releaseBackBuffer();

            m_swapChain->GetDesc(&swapChainDesc);
            VALIDATE_DIRECTX_CALL(m_swapChain->ResizeBuffers(swapChainDesc.BufferCount, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));
        }

        m_width = width;
        m_height = height;
        m_memoryUsage = GfxUtils::calculateTextureMemoryUsage(m_format, m_width, m_height, 1) * swapChainDesc.BufferCount;

        getBackBuffer();

        return true;
    }

    void DX11SwapChain::copyBackbuffer(GfxContext *context, GfxTexture *dst)
    {
        const auto contextDX11 = (DX11Context *)context;
        contextDX11->GetContext()->CopyResource((ID3D11Resource *)dst->getNativePtr(), m_backBuffer);
    }

}

#endif
