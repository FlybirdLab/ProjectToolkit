/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Device.hpp"
#include "toolkit/graphics/GfxSwapChain.hpp"
#include "Dx12ResOwner.hpp"
#include "Dx12Texture.hpp"

namespace toolkit
{
    class DX12SwapChain;

    // Represents a DirectX 12 swap chain back buffer wrapper object.
    class BackBufferDX12 : public DX12ResourceOwner
    {
    public:
        // The render target surface handle.
        DX12TextureView handle;

    public:
        // Setup backbuffer wrapper
        void setup(DX12SwapChain *window, ID3D12Resource *backbuffer);
        // Release references to the backbuffer
        void release();

    public:
        GfxResource *asResource() const override
        {
            return nullptr;
        }
    };

    // Graphics Device rendering output for DirectX 12 backend
    class DX12SwapChain : public DX12Resource<GfxSwapChain>
    {
        friend class WindowsWindow;
        friend class GPUContextDX12;
        friend DX12Device;

    public:
        DX12SwapChain(DX12Device *device, GfxWindow *window);

    public:
        // Get current backbuffer resource.
        ID3D12Resource *getBackBuffer() const
        {
            return m_backBuffers[m_currentFrameIndex].getResource();
        }
        // Get render target handle for DirectX 12 backend.
        const DX12TextureView *getBackBufferHandleDX12() const
        {
            return &m_backBuffers[m_currentFrameIndex].handle;
        }

    private:
        void getBackBuffer();
        void releaseBackBuffer();

    public:
        bool isFullscreen() override;
        void setFullscreen(bool isFullscreen) override;
        GfxTextureView *getBackBufferView() override;
        bool isReady() const override;
        void end() override;
        void present(bool vsync) override;
        bool resize(i32 width, i32 height) override;
        void copyBackbuffer(GfxContext *context, GfxTexture *dst) override;

    protected:
        void onReleaseData() override;

    private:
        bool m_allowTearing, m_isFullscreen;
        HWND m_windowHandle;
        IDXGISwapChain3 *m_swapChain;
        i32 m_currentFrameIndex;
        TkStaticArray<BackBufferDX12, 4> m_backBuffers;
    };

}

#endif
