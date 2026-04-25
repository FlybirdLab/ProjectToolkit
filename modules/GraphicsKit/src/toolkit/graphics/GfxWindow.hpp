/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>
#include <toolkit/windowing/Window.hpp>

namespace toolkit
{
    class GfxWindow : public TkWindow
    {
        friend GfxSwapChain;

    public:
        GfxWindow(const WindowParams &p);
        ~GfxWindow();

    public:
        // Get rendering output swap chain
        TK_FORCE_INLINE GfxSwapChain *getSwapChain() const
        {
            return m_swapChain;
        }

        // Init the swap chain and the rendering task.
        bool initSwapChain();

    private:
        GfxSwapChain *m_swapChain;
    };
}