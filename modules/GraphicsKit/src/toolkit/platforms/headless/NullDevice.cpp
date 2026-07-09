/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NullDevice.hpp"
#include "toolkit/memory/Memory.hpp"

#if GFX_API_NULL

#include "NullAdapter.hpp"
#include "NullShader.hpp"
#include "NullVertexLayout.hpp"
#include "NullPipelineState.hpp"
#include "NullBuffer.hpp"
#include "NullContext.hpp"
#include "NullTimeQuery.hpp"
#include "NullSwapChain.hpp"
#include "NullTexture.hpp"
#include "NullSampler.hpp"
#include "NullDevice.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/async/GfxTaskMgr.hpp"

namespace toolkit
{
    NullDevice::NullDevice()
        : GfxDevice(GfxBackendType::kNull, GfxShaderProfile::kUnknown), 
        m_mainContext(nullptr), m_adapter(TkMemory::newObject<NullAdapter>())
    {
    }

    GfxDevice *NullDevice::create()
    {
        // Create device
        auto device = TkMemory::newObject<NullDevice>();
        if (!device->init())
        {
            LogWarn("Graphics Device init failed");
            TkMemory::deletePtr(device);
            return nullptr;
        }

        return device;
    }

    NullDevice::~NullDevice()
    {
        // Ensure to be disposed
        NullDevice::dispose();
    }

    bool NullDevice::init()
    {
        totalMemory = 0;
        m_state = DeviceState::kCreated;

        // Init device limits
        {
            auto &limits = GfxDevice::limits;
            TkMemory::memoryClear(&limits, sizeof(limits));
            limits.maxMipLevelsCount = 14;
            limits.maxTexture1DSize = 8192;
            limits.maxTexture1DArraySize = 512;
            limits.maxTexture2DSize = 8192;
            limits.maxTexture2DArraySize = 512;
            limits.maxTexture3DSize = 2048;
            limits.maxTextureCubeSize = 16384;
            limits.maxSamplerAnisotropy = 1;
            for (i32 i = 0; i < static_cast<i32>(PixelFormat::MAX); i++)
                features[i] = FormatFeatures(static_cast<PixelFormat>(i), GfxMSAALevel::kNone, FormatSupport::kNone);
        }

        // Create main context
        m_mainContext = TkMemory::newObject<NullContext>(TkMemory::getDefault(), this);

        m_state = DeviceState::kReady;
        return GfxDevice::init();
    }

    void NullDevice::draw()
    {
        drawBegin();

        auto context = getMainContext();

        renderBegin();
        getTaskManager()->frameBegin();
        context->frameBegin();

        // don't render anything

        context->frameEnd();
        getTaskManager()->frameEnd();
        renderEnd();

        drawEnd();
    }

    void NullDevice::dispose()
    {
        GfxDeviceLock lock(this);

        // Check if has been disposed already
        if (m_state == DeviceState::kDisposed)
            return;

        // Set current state
        m_state = DeviceState::kDisposing;

        // Wait for rendering end
        waitForGPU();

        // Pre dispose
        preDispose();

        // Clear stuff
        TK_SAFE_DELETE(m_mainContext);
        TK_SAFE_DELETE(m_adapter);

        // Base
        GfxDevice::dispose();

        // Set current state
        m_state = DeviceState::kDisposed;
    }

    GfxContext *NullDevice::getMainContext()
    {
        return reinterpret_cast<GfxContext *>(m_mainContext);
    }

    GfxAdapter *NullDevice::getAdapter() const
    {
        return reinterpret_cast<GfxAdapter *>(m_adapter);
    }

    void *NullDevice::getNativePtr() const
    {
        return nullptr;
    }

    void NullDevice::waitForGPU()
    {
    }

    GfxTexture *NullDevice::createTexture(const TkStringView &name)
    {
        return TkMemory::newObject<NullTexture>();
    }

    GfxShader *NullDevice::createShader(const TkStringView &name)
    {
        return TkMemory::newObject<NullShader>();
    }

    GfxPipelineState *NullDevice::createPipelineState()
    {
        return TkMemory::newObject<NullPipelineState>();
    }

    GfxTimerQuery *NullDevice::createTimerQuery()
    {
        return TkMemory::newObject<NullTimeQuery>();
    }

    GfxBuffer *NullDevice::createBuffer(const TkStringView &name)
    {
        return TkMemory::newObject<NullBuffer>();
    }

    GfxSampler *NullDevice::createSampler()
    {
        return TkMemory::newObject<NullSampler>();
    }

    GfxVertexLayout *NullDevice::createVertexLayout(const VertexElements &elements, bool explicitOffsets)
    {
        return TkMemory::newObject<NullVertexLayout>(TkMemory::getDefault(), elements);
    }

    GfxSwapChain *NullDevice::createSwapChain(GfxWindow *window)
    {
        return TkMemory::newObject<NullSwapChain>(TkMemory::getDefault(), window);
    }

    GfxConstantBuffer *NullDevice::createConstantBuffer(u32 size, const TkStringView &name)
    {
        return nullptr;
    }

    GfxDevice *createNullDevice()
    {
        return NullDevice::create();
    }
}

#endif