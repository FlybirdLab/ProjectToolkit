/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12SwapChain.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Context.hpp"
#include "Dx12Texture.hpp"
#include "toolkit/graphics/GfxWindow.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/utils/Color.hpp"

namespace toolkit
{

    void BackBufferDX12::setup(DX12SwapChain *window, ID3D12Resource *backbuffer)
    {
        // Cache handle and set default initial state for the backbuffers
        initResource(backbuffer, D3D12_RESOURCE_STATE_PRESENT, 1);

        handle.init(window, window->getDevice(), this, GFX_BACK_BUFFER_PIXEL_FORMAT, GfxMSAALevel::kNone);

        // Create RTV
        {
            D3D12_RENDER_TARGET_VIEW_DESC rtDesc;
            rtDesc.Format = DxUtils::toDxgiFormat(GFX_BACK_BUFFER_PIXEL_FORMAT);
            rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtDesc.Texture2D.MipSlice = 0;
            rtDesc.Texture2D.PlaneSlice = 0;
            handle.setRTV(rtDesc);
        }

#if GFX_USE_WINDOW_SRV
        // Create SRV
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srDesc;
            srDesc.Format = DxUtils::toDxgiFormat(GFX_BACK_BUFFER_PIXEL_FORMAT);
            srDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srDesc.Texture2D.MostDetailedMip = 0;
            srDesc.Texture2D.MipLevels = 1;
            srDesc.Texture2D.ResourceMinLODClamp = 0;
            srDesc.Texture2D.PlaneSlice = 0;
            handle.setSRV(srDesc);
        }
#endif
    }

    void BackBufferDX12::release()
    {
        handle.release();
        if (m_resource)
        {
            m_resource->Release();
            m_resource = nullptr;
        }
    }

    DX12SwapChain::DX12SwapChain(DX12Device *device, GfxWindow *window)
        : DX12Resource<GfxSwapChain>(device, TkStringView::Empty), m_allowTearing(false), m_isFullscreen(false), m_windowHandle(static_cast<HWND>(window->getWindowPtr())), m_swapChain(nullptr), m_currentFrameIndex(0)
    {
        TK_ASSERT(m_windowHandle);
        m_window = window;
    }

    void DX12SwapChain::onReleaseData()
    {
        m_device->waitForGPU();

        // Disable fullscreen mode
        if (m_swapChain)
        {
            VALIDATE_DIRECTX_CALL(m_swapChain->SetFullscreenState(false, nullptr));
        }

        // Release data
        releaseBackBuffer();
        m_backBuffers.resize(0);
        if (m_swapChain)
        {
            m_device->addResourceToLateRelease(m_swapChain);
            m_swapChain = nullptr;
        }
        m_width = m_height = 0;
    }

    void DX12SwapChain::releaseBackBuffer()
    {
        for (i32 i = 0; i < m_backBuffers.count(); i++)
        {
            m_backBuffers[i].release();
        }
    }

    bool DX12SwapChain::isFullscreen()
    {
        // Check if has no swap chain created
        if (m_swapChain == nullptr)
            return false;

        // Get state
        BOOL state;
        VALIDATE_DIRECTX_CALL(m_swapChain->GetFullscreenState(&state, nullptr));
        return state == TRUE;
    }

    void DX12SwapChain::setFullscreen(bool isFullscreen)
    {
        if (m_swapChain && isFullscreen != this->isFullscreen())
        {
            m_device->waitForGPU();
            GfxDeviceLock lock(m_device);

            DXGI_SWAP_CHAIN_DESC swapChainDesc;
            m_swapChain->GetDesc(&swapChainDesc);

            // Setup target for fullscreen mode
            IDXGIOutput *output = nullptr;

            if (FAILED(m_swapChain->ResizeTarget(&swapChainDesc.BufferDesc)))
            {
                LogWarn("Swapchain resize failed.");
            }

            if (FAILED(m_swapChain->SetFullscreenState(isFullscreen, output)))
            {
                LogWarn("Cannot change fullscreen mode.");
            }

            m_isFullscreen = isFullscreen;

            // Buffers must be resized in flip presentation model
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

    bool DX12SwapChain::resize(i32 width, i32 height)
    {
        // Check if window state won't change
        if (width == m_width && height == m_height)
        {
            return true;
        }

        m_device->waitForGPU();
        GfxDeviceLock lock(m_device);
        m_allowTearing = m_device->allowTearing;
        m_format = GFX_BACK_BUFFER_PIXEL_FORMAT;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        if (m_swapChain == nullptr)
        {
            onReleaseData();

            // Create swap chain description
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = DxUtils::toDxgiFormat(m_format);
            swapChainDesc.Stereo = FALSE;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = DX12_BACK_BUFFER_COUNT;
            swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
#if GFX_USE_WINDOW_SRV
            swapChainDesc.BufferUsage |= DXGI_USAGE_SHADER_INPUT;
#endif
            if (m_allowTearing)
                swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
            fullscreenDesc.RefreshRate.Numerator = 0;
            fullscreenDesc.RefreshRate.Denominator = 1;
            fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            fullscreenDesc.Windowed = TRUE;

            // Create swap chain (it needs the queue so that it can force a flush on it)
            IDXGISwapChain1 *swapChain;
            auto dxgiFactory = m_device->getDXGIFactory();
            VALIDATE_DIRECTX_CALL(dxgiFactory->CreateSwapChainForHwnd(m_device->getDX12CmdQueue(), m_windowHandle, &swapChainDesc, &fullscreenDesc, nullptr, &swapChain));
            m_swapChain = static_cast<IDXGISwapChain3 *>(swapChain);
            TK_ASSERT(m_swapChain);
            DX_SET_DEBUG_NAME((IDXGIObject *)m_swapChain, "RenderOutput:SwapChain");
            m_swapChain->SetBackgroundColor((const DXGI_RGBA *)Color::Black.raw);
            m_backBuffers.resize(swapChainDesc.BufferCount);

            // Disable DXGI changes to the window
            VALIDATE_DIRECTX_CALL(dxgiFactory->MakeWindowAssociation(m_windowHandle, DXGI_MWA_NO_ALT_ENTER));
        }
        else
        {
            releaseBackBuffer();

            m_swapChain->GetDesc1(&swapChainDesc);

            VALIDATE_DIRECTX_CALL(m_swapChain->ResizeBuffers(swapChainDesc.BufferCount, width, height, swapChainDesc.Format, swapChainDesc.Flags));
        }

        m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();
        m_width = width;
        m_height = height;
        m_memoryUsage = GfxUtils::calculateTextureMemoryUsage(m_format, m_width, m_height, 1) * swapChainDesc.BufferCount;

        getBackBuffer();

        return true;
    }

    void DX12SwapChain::copyBackbuffer(GfxContext *context, GfxTexture *dst)
    {
        const auto contextDX12 = (DX12Context *)context;

        auto dstDX12 = (DX12Texture *)dst;
        auto backbuffer = &m_backBuffers[m_currentFrameIndex];

        contextDX12->setResourceState(dstDX12, D3D12_RESOURCE_STATE_COPY_DEST);
        contextDX12->setResourceState(backbuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
        contextDX12->flushResourceBarriers();

        if (dst->isStaging())
        {
            const i32 copyOffset = dstDX12->computeBufferOffset(0, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

            D3D12_TEXTURE_COPY_LOCATION dstLocation;
            dstLocation.pResource = dstDX12->getResource();
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dstLocation.PlacedFootprint.Offset = copyOffset;
            dstLocation.PlacedFootprint.Footprint.Width = dstDX12->calculateMipSize(dstDX12->width(), 0);
            dstLocation.PlacedFootprint.Footprint.Height = dstDX12->calculateMipSize(dstDX12->height(), 0);
            dstLocation.PlacedFootprint.Footprint.Depth = dstDX12->calculateMipSize(dstDX12->depth(), 0);
            dstLocation.PlacedFootprint.Footprint.Format = DxUtils::toDxgiFormat(dstDX12->format());
            dstLocation.PlacedFootprint.Footprint.RowPitch = dstDX12->computeRowPitch(0, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

            D3D12_TEXTURE_COPY_LOCATION srcLocation;
            srcLocation.pResource = backbuffer->getResource();
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            srcLocation.SubresourceIndex = 0;

            contextDX12->getCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
        }
        else
        {
            contextDX12->getCommandList()->CopyResource(dstDX12->getResource(), backbuffer->getResource());
        }
    }

    void DX12SwapChain::getBackBuffer()
    {
        m_backBuffers.resize(DX12_BACK_BUFFER_COUNT);
        for (i32 i = 0; i < m_backBuffers.count(); i++)
        {
            ID3D12Resource *backbuffer;
            VALIDATE_DIRECTX_CALL(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backbuffer)));
            DX_SET_DEBUG_NAME_EX(backbuffer, "RenderOutput", "BackBuffer", i);
            m_backBuffers[i].setup(this, backbuffer);
        }
    }

    void DX12SwapChain::end()
    {
        GfxSwapChain::end();

        auto context = m_device->getMainContextDX12();

        // Indicate that the back buffer will be used to present a frame
        // Note: after that we should not use this backbuffer
        context->setResourceState(&m_backBuffers[m_currentFrameIndex], D3D12_RESOURCE_STATE_PRESENT);

        // Send event
        context->onSwapChainFlush();
    }

    void DX12SwapChain::present(bool vsync)
    {
        // Present frame
        UINT presentFlags = 0;
        if (!vsync && !m_isFullscreen && m_allowTearing)
        {
            presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
        }
        const HRESULT res = m_swapChain->Present(vsync ? 1 : 0, presentFlags);
        LOG_DIRECTX_RESULT(res);

        // Base
        GfxSwapChain::present(vsync);

        // Switch to next back buffer
        m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();
    }

    GfxTextureView *DX12SwapChain::getBackBufferView()
    {
        return &m_backBuffers[m_currentFrameIndex].handle;
    }

    bool DX12SwapChain::isReady() const
    {
        return m_swapChain != nullptr && m_width > 0 && m_height > 0;
    }

}

#endif
