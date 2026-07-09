/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxSwapChain.hpp>

#if GFX_API_NULL

namespace toolkit
{

    // Graphics Device rendering output for Null backend.
    class NullSwapChain : public GfxSwapChain
    {
    public:
        NullSwapChain(GfxWindow *window)
            : GfxSwapChain()
        {
            m_window = window;
        }

    public:
        bool isFullscreen() override
        {
            return false;
        }

        void setFullscreen(bool isFullscreen) override
        {
        }

        GfxTextureView *getBackBufferView() override
        {
            return nullptr;
        }

        void present(bool vsync) override
        {
        }

        bool resize(i32 width, i32 height) override
        {
            return false;
        }

        void copyBackbuffer(GfxContext *context, GfxTexture *dst) override
        {
        }

        bool isReady() const override
        {
            return true;
        }
    };

}

#endif
