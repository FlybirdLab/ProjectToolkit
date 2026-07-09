/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxDevice.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL

namespace toolkit
{

    // Implementation of GfxDevice for the OpenGL fallback backend.
    //
    // Phase 2: a real shared-with-everyone device context is created and the
    // version is queried; swapchain / draw paths are still minimal.
    class GLDevice : public GfxDevice
    {
        friend class GLContext;
        friend class GLSwapChain;
        friend class GLAdapter;

    private:
        class GLContext *m_mainContext = nullptr;
        class GLAdapter *m_adapter = nullptr;
        // Owned. Allocated in init(), released in dispose(). Kept as a pointer
        // so this header does not need to pull in GLPlatform.hpp.
        struct GLPlatformContext *m_deviceContext = nullptr;
        bool m_hasDeviceContext = false;
        // Owned. Phase 5: FBO cache shared by all GLContexts on this device.
        class GLFramebufferCache *m_framebufferCache = nullptr;

    public:
        static GfxDevice *create();
        GLDevice();
        ~GLDevice();

    public:
        GfxContext *getMainContext() override;
        GfxAdapter *getAdapter() const override;
        void *getNativePtr() const override;
        bool init() override;
        void draw() override;
        void dispose() override;
        void waitForGPU() override;

        GfxTexture *createTexture(const TkStringView &name) override;
        GfxShader *createShader(const TkStringView &name) override;
        GfxPipelineState *createPipelineState() override;
        GfxTimerQuery *createTimerQuery() override;
        GfxBuffer *createBuffer(const TkStringView &name) override;
        GfxSampler *createSampler() override;
        GfxVertexLayout *createVertexLayout(const VertexElements &elements, bool explicitOffsets) override;
        GfxSwapChain *createSwapChain(GfxWindow *window) override;
        GfxConstantBuffer *createConstantBuffer(u32 size, const TkStringView &name) override;

    public:
        // Returns the shared device GL context (used by swapchains to share resources).
        struct GLPlatformContext *getDeviceContext()
        {
            return m_hasDeviceContext ? m_deviceContext : nullptr;
        }

        // Returns the shared FBO cache (owned by the device). May be null
        // briefly during init / dispose.
        class GLFramebufferCache *framebufferCache() const { return m_framebufferCache; }
    };

    extern GfxDevice *createDeviceOGL();

}

#endif
