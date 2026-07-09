/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx11/common.hpp>

#if GFX_API_DIRECTX11

#include "Dx11Device.hpp"
#include "Dx11Texture.hpp"
#include "toolkit/graphics/GfxSwapChain.hpp"

namespace toolkit
{
    // Graphics Device rendering output for DirectX 11 backend.
    class DX11SwapChain : public DX11Resource<GfxSwapChain>
    {
        friend class TkWindow;
        friend class DX11Context;
        friend DX11Device;

    public:
        DX11SwapChain(DX11Device *device, GfxWindow *window);

    private:
        void getBackBuffer();
        void releaseBackBuffer();

    public:
        ID3D11Resource *getResource() override;

        bool isFullscreen() override;
        void setFullscreen(bool isFullscreen) override;
        GfxTextureView *getBackBufferView() override;
        bool isReady() const override;
        void present(bool vsync) override;
        bool resize(i32 width, i32 height) override;
        void copyBackbuffer(GfxContext *context, GfxTexture *dst) override;

    protected:
        void onReleaseData() final override;

    private:
        HWND m_windowHandle;
        IDXGISwapChain *m_swapChain;
        bool m_allowTearing, m_isFullscreen;
        ID3D11Texture2D *m_backBuffer;
        DX11TextureView m_backBufferHandle;
    };

}

#endif
