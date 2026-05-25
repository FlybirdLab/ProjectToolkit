/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxSwapChain.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Window-backed presentation surface for the OpenGL fallback backend.
    //
    // The first resize() call creates the per-window GL context that shares
    // resources with the device's hidden context. Subsequent resize() calls
    // just refresh the viewport.
    class GLSwapChain : public GfxSwapChain
    {
    public:
        GLSwapChain(class GLDevice *device, GfxWindow *window);
        ~GLSwapChain();

    public:
        bool isFullscreen() override { return m_isFullscreen; }
        void setFullscreen(bool isFullscreen) override { m_isFullscreen = isFullscreen; }

        GfxTextureView *getBackBufferView() override;
        void present(bool vsync) override;
        bool resize(i32 width, i32 height) override;
        void copyBackbuffer(GfxContext *context, GfxTexture *dst) override;
        bool isReady() const override { return m_hasWindowContext && m_width > 0 && m_height > 0; }

    public:
        // Used by GLContext to make the swapchain's GL context current before
        // touching the default framebuffer.
        bool makeCurrent();

    protected:
        void onReleaseData() override;

    private:
        class GLDevice *m_device = nullptr;
        // Heap-owned so this header doesn't need GLPlatform.hpp.
        struct GLPlatformContext *m_windowContext = nullptr;
        bool m_hasWindowContext = false;
        bool m_isFullscreen = false;
        // Heap-owned. Kept as a pointer so this header doesn't need to pull
        // in the full GLTextureView definition.
        class GLTextureView *m_backBufferView = nullptr;
        // VAO owned by THIS swap chain (Core Profile requires a bound VAO on
        // every draw, and VAOs are NOT shared between GL contexts). Generated
        // lazily inside m_windowContext on the first resize() that produces a
        // current context.
        u32 m_vao = 0;

    public:
        // Returns the VAO created inside this swap chain's GL context. Valid
        // only after the first successful resize(). GLContext uses this when
        // the last-bound RT belonged to a swap chain so glDraw* operates on a
        // VAO that actually exists in the current context.
        u32 vao() const { return m_vao; }
    };

}

#endif
