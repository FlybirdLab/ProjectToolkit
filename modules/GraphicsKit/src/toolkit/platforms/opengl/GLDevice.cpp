/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLDevice.hpp"

#if GFX_API_OPENGL

#include "GLAdapter.hpp"
#include "GLBuffer.hpp"
#include "GLConstantBuffer.hpp"
#include "GLContext.hpp"
#include "GLFramebufferCache.hpp"
#include "GLNative.hpp"
#include "GLPipelineState.hpp"
#include "GLSampler.hpp"
#include "GLShader.hpp"
#include "GLSwapChain.hpp"
#include "GLTexture.hpp"
#include "GLTimerQuery.hpp"
#include "GLUtils.hpp"
#include "GLVertexLayout.hpp"
#include "platform/GLPlatform.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/async/GfxTaskMgr.hpp"
#include "toolkit/memory/Memory.hpp"

namespace toolkit
{

    GLDevice::GLDevice()
        : GfxDevice(GfxBackendType::kOpenGL4_1, GfxShaderProfile::kGLSL_410),
          m_adapter(TkMemory::newObject<GLAdapter>())
    {
        m_featureLevel = GfxFeatureLevel::kSM4;
    }

    GfxDevice *GLDevice::create()
    {
        auto device = TkMemory::newObject<GLDevice>();
        if (!device->init())
        {
            LogWarn("[%s] device init failed", GLBackendName());
            TkMemory::deletePtr(device);
            return nullptr;
        }
        return device;
    }

    GLDevice::~GLDevice()
    {
        GLDevice::dispose();
    }

    bool GLDevice::init()
    {
        if (m_hasDeviceContext)
        {
            // Already initialised; GfxManager::init occasionally calls init()
            // twice during retries -- don't leak or recreate a context.
            return true;
        }

        totalMemory = 0;
        m_state = DeviceState::kCreated;

        if (!GLPlatform::init())
        {
            LogError("GLPlatform::init failed");
            return false;
        }

        m_deviceContext = TkMemory::newObject<GLPlatformContext>();
        if (!m_deviceContext)
        {
            LogError("Failed to allocate GLPlatformContext");
            return false;
        }

        if (!GLPlatform::createDeviceContext(*m_deviceContext))
        {
            LogError("GLPlatform::createDeviceContext failed");
            TK_SAFE_DELETE(m_deviceContext);
            return false;
        }
        m_hasDeviceContext = true;

        if (!GLPlatform::makeCurrent(m_deviceContext))
        {
            LogError("GLPlatform::makeCurrent(device) failed");
            return false;
        }

        if (!GLPlatform::loadGLFunctions())
        {
            LogError("Failed to load OpenGL function pointers via glad");
            return false;
        }

        GLLogVersionInfo();
        if (!GLCheckMinimumVersion(4, 1))
        {
            // We still continue -- some drivers misreport, and the harness
            // catches actual GL errors. But surface this loudly.
            LogWarn("OpenGL 4.1 Core was requested but driver reports a lower version");
        }

        {
            auto &lims = GfxDevice::limits;
            TkMemory::memoryClear(&lims, sizeof(lims));
            lims.maxMipLevelsCount = 14;
            lims.maxTexture1DSize = 8192;
            lims.maxTexture1DArraySize = 512;
            lims.maxTexture2DSize = 8192;
            lims.maxTexture2DArraySize = 512;
            lims.maxTexture3DSize = 2048;
            lims.maxTextureCubeSize = 16384;
            lims.maxSamplerAnisotropy = 1;
            lims.hasCompute = false;
            lims.hasGeometryShaders = false;
            lims.hasTessellation = false;
            lims.hasDrawIndirect = false;
            lims.hasInstancing = true;
            lims.hasAppendConsumeBuffers = false;
            for (i32 i = 0; i < static_cast<i32>(PixelFormat::MAX); i++)
                features[i] = FormatFeatures(static_cast<PixelFormat>(i), GfxMSAALevel::kNone, FormatSupport::kNone);
        }

        m_mainContext = TkMemory::newObject<GLContext>(TkMemory::getDefault(), this);

        // FBO cache: allocate now, but real FBO objects are created lazily on
        // the first setRenderTarget(texture). The cache itself doesn't touch
        // any GL state until then.
        m_framebufferCache = TkMemory::newObject<GLFramebufferCache>();
        if (m_framebufferCache && !m_framebufferCache->init())
        {
            LogWarn("GLFramebufferCache::init failed");
            TK_SAFE_DELETE(m_framebufferCache);
        }

        LogInfo("OpenGL fallback backend initialized");

        // Surface any GL errors that piled up during context bootstrap before
        // the first user draw call can mask them.
        GLCheckError("GLDevice::init");

        m_state = DeviceState::kReady;
        return GfxDevice::init();
    }

    void GLDevice::draw()
    {
        drawBegin();

        auto context = getMainContext();

        renderBegin();
        getTaskManager()->frameBegin();
        context->frameBegin();

        // Phase 2: no high-level rendering yet.

        context->frameEnd();
        getTaskManager()->frameEnd();
        renderEnd();

        drawEnd();
    }

    void GLDevice::dispose()
    {
        GfxDeviceLock lock(this);

        if (m_state == DeviceState::kDisposed)
            return;

        m_state = DeviceState::kDisposing;

        waitForGPU();
        preDispose();

        // Order: GLContext first (releases VAO inside device context), then
        // FBO cache (releases any leftover FBOs while a GL context is still
        // current), then adapter, then platform context.
        TK_SAFE_DELETE(m_mainContext);
        if (m_framebufferCache)
        {
            m_framebufferCache->shutdown();
            TK_SAFE_DELETE(m_framebufferCache);
        }
        TK_SAFE_DELETE(m_adapter);

        if (m_hasDeviceContext)
        {
            GLPlatform::clearCurrent();
            if (m_deviceContext)
                GLPlatform::destroyContext(*m_deviceContext);
            m_hasDeviceContext = false;
        }
        TK_SAFE_DELETE(m_deviceContext);
        GLPlatform::shutdown();

        GfxDevice::dispose();

        m_state = DeviceState::kDisposed;
    }

    GfxContext *GLDevice::getMainContext()
    {
        return reinterpret_cast<GfxContext *>(m_mainContext);
    }

    GfxAdapter *GLDevice::getAdapter() const
    {
        return reinterpret_cast<GfxAdapter *>(m_adapter);
    }

    void *GLDevice::getNativePtr() const
    {
        return nullptr;
    }

    void GLDevice::waitForGPU()
    {
        if (!m_hasDeviceContext || !m_deviceContext)
            return;
        if (!GLPlatform::isCurrent(m_deviceContext))
            GLPlatform::makeCurrent(m_deviceContext);
        glFinish();
    }

    GfxTexture *GLDevice::createTexture(const TkStringView &name)
    {
        return TkMemory::newObject<GLTexture>();
    }

    GfxShader *GLDevice::createShader(const TkStringView &name)
    {
        return TkMemory::newObject<GLShader>();
    }

    GfxPipelineState *GLDevice::createPipelineState()
    {
        return TkMemory::newObject<GLPipelineState>();
    }

    GfxTimerQuery *GLDevice::createTimerQuery()
    {
        return TkMemory::newObject<GLTimerQuery>();
    }

    GfxBuffer *GLDevice::createBuffer(const TkStringView &name)
    {
        return TkMemory::newObject<GLBuffer>();
    }

    GfxSampler *GLDevice::createSampler()
    {
        return TkMemory::newObject<GLSampler>();
    }

    GfxVertexLayout *GLDevice::createVertexLayout(const VertexElements &elements, bool explicitOffsets)
    {
        return TkMemory::newObject<GLVertexLayout>(TkMemory::getDefault(), elements);
    }

    GfxSwapChain *GLDevice::createSwapChain(GfxWindow *window)
    {
        return TkMemory::newObject<GLSwapChain>(TkMemory::getDefault(), this, window);
    }

    GfxConstantBuffer *GLDevice::createConstantBuffer(u32 size, const TkStringView &name)
    {
        if (size == 0)
            return nullptr;
        auto *cb = TkMemory::newObject<GLConstantBuffer>(TkMemory::getDefault(), size);
        if (!cb || !cb->init())
        {
            TK_SAFE_DELETE(cb);
            return nullptr;
        }
        return cb;
    }

    GfxDevice *createDeviceOGL()
    {
        return GLDevice::create();
    }

}

#endif
