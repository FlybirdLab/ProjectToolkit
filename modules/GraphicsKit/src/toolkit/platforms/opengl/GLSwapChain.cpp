/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLSwapChain.hpp"

#if GFX_API_OPENGL

#include "GLDevice.hpp"
#include "GLNative.hpp"
#include "GLTexture.hpp"
#include "GLUtils.hpp"
#include "platform/GLPlatform.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/windowing/Window.hpp"

namespace toolkit
{

    GLSwapChain::GLSwapChain(GLDevice *device, GfxWindow *window)
        : GfxSwapChain(), m_device(device)
    {
        m_window = window;
        m_format = GFX_BACK_BUFFER_PIXEL_FORMAT;
        m_backBufferView = TkMemory::newObject<GLTextureView>();
    }

    GLSwapChain::~GLSwapChain()
    {
        GLSwapChain::onReleaseData();
    }

    void GLSwapChain::onReleaseData()
    {
        if (m_hasWindowContext && m_windowContext)
        {
            // VAOs are per-context. Delete ours while the owning GL context is
            // still alive and current; the destroyContext call right after
            // tears the context down and any GL handle goes invalid with it.
            if (m_vao != 0)
            {
                GLPlatform::makeCurrent(m_windowContext);
                const GLuint vao = static_cast<GLuint>(m_vao);
                glDeleteVertexArrays(1, &vao);
                m_vao = 0;
            }
            GLPlatform::destroyContext(*m_windowContext);
            m_hasWindowContext = false;
        }
        TK_SAFE_DELETE(m_windowContext);
        TK_SAFE_DELETE(m_backBufferView);
    }

    bool GLSwapChain::makeCurrent()
    {
        if (!m_hasWindowContext || !m_windowContext)
            return false;
        return GLPlatform::makeCurrent(m_windowContext);
    }

    GfxTextureView *GLSwapChain::getBackBufferView()
    {
        return m_backBufferView;
    }

    bool GLSwapChain::resize(i32 width, i32 height)
    {
        if (width <= 0 || height <= 0)
        {
            LogWarn("GLSwapChain::resize: invalid size %dx%d", width, height);
            return false;
        }

        if (!m_hasWindowContext)
        {
            if (!m_window)
            {
                LogError("GLSwapChain::resize: no GfxWindow attached");
                return false;
            }
            void *windowHandle = m_window->getWindowPtr();
            if (!windowHandle)
            {
                LogError("GLSwapChain::resize: window has no native handle");
                return false;
            }

            m_windowContext = TkMemory::newObject<GLPlatformContext>();
            if (!m_windowContext)
                return false;

            GLPlatformContext *shared = m_device ? m_device->getDeviceContext() : nullptr;
            if (!GLPlatform::createWindowContext(windowHandle, *m_windowContext, shared))
            {
                LogError("GLPlatform::createWindowContext failed");
                TK_SAFE_DELETE(m_windowContext);
                return false;
            }
            m_hasWindowContext = true;
            if (m_backBufferView)
                m_backBufferView->initBackBuffer(this, GFX_BACK_BUFFER_PIXEL_FORMAT);
        }

        if (!GLPlatform::makeCurrent(m_windowContext))
        {
            LogError("GLSwapChain::resize: makeCurrent failed");
            return false;
        }

        // Tell the platform layer the drawable geometry changed. On macOS this
        // forwards to [NSOpenGLContext update] (mandatory after view resize /
        // Retina backing-scale change). No-op on WGL.
        GLPlatform::updateContext(*m_windowContext);

        // Lazily create the per-context VAO. VAOs are not shared between GL
        // contexts, so the one in GLContext (created in the device context)
        // is invalid here. Without this every glDraw* would hit
        // GL_INVALID_OPERATION on Core Profile.
        if (m_vao == 0)
        {
            GLuint vao = 0;
            glGenVertexArrays(1, &vao);
            if (vao == 0)
            {
                LogError("GLSwapChain::resize: glGenVertexArrays failed");
                return false;
            }
            m_vao = static_cast<u32>(vao);
        }

        m_width = width;
        m_height = height;
        m_format = GFX_BACK_BUFFER_PIXEL_FORMAT;
        glViewport(0, 0, width, height);
        GLCheckError("GLSwapChain::resize");
        return true;
    }

    void GLSwapChain::present(bool vsync)
    {
        if (!isReady())
            return;
        GLPlatform::makeCurrent(m_windowContext);
        GLPlatform::setSwapInterval(vsync);
        GLPlatform::swapBuffers(*m_windowContext);
        onPresented();
    }

    void GLSwapChain::copyBackbuffer(GfxContext *context, GfxTexture *dst)
    {
        GLReportUnsupported("GLSwapChain::copyBackbuffer");
    }

}

#elif GFX_API_OPENGLES

namespace toolkit
{
    GLSwapChain::GLSwapChain(GLDevice *device, GfxWindow *window) : GfxSwapChain(), m_device(device) { m_window = window; }
    GLSwapChain::~GLSwapChain() {}
    void GLSwapChain::onReleaseData() {}
    bool GLSwapChain::makeCurrent() { return false; }
    GfxTextureView *GLSwapChain::getBackBufferView() { return nullptr; }
    bool GLSwapChain::resize(i32, i32) { return false; }
    void GLSwapChain::present(bool) {}
    void GLSwapChain::copyBackbuffer(GfxContext *, GfxTexture *) {}
}

#endif
