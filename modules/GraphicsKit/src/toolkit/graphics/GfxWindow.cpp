/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxWindow.hpp"

#include "GfxSwapChain.hpp"
#include "GfxDevice.hpp"

namespace toolkit
{

    GfxWindow::GfxWindow(const WindowParams &p)
        : TkWindow(p)
    {
    }

    GfxWindow::~GfxWindow()
    {
        if (m_swapChain)
            TkMemory::deletePtr(m_swapChain);
    }

    bool GfxWindow::initSwapChain()
    {
        // Setup swapchain
        if (m_swapChain == nullptr)
        {
            m_swapChain = GfxDevice::sInstance->createSwapChain((GfxWindow *)this);
            if (!m_swapChain)
                return false;
        }
        if (!m_swapChain->resize(static_cast<i32>(m_clientSize.x), static_cast<i32>(m_clientSize.y)))
            return false;
        if (getParams().fullscreen)
            m_swapChain->setFullscreen(true);

        return true;
    }
}