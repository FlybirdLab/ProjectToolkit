/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxDevice.hpp"
#include "GfxBuffer.hpp"
#include "GfxContext.hpp"
#include "GfxManager.hpp"
#include "utils/GfxUtils.hpp"
#include "GfxPipelineState.hpp"
#include "GfxResource.hpp"
#include "GfxTargetPool.hpp"
#include "GfxVertexLayout.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/graphics/async/GfxTaskContext.hpp"
#include "toolkit/graphics/async/GfxTaskMgr.hpp"
#include "toolkit/types/StringBuilder.hpp"
#include "toolkit/utils/Convertor.hpp"
#include "async/GfxTaskExecutor.hpp"

namespace toolkit
{

    struct GfxDevice::PrivateData
    {
        GfxTaskManager tskMgr;
        GfxPipelineState *copyLinearPS = nullptr;
        GfxPipelineState *clearPS = nullptr;
        GfxPipelineState *decodeYUY2PS = nullptr;
        GfxPipelineState *decodeNV12PS = nullptr;
        GfxBuffer *fullscreenTriangleVB = nullptr;
    };

    GfxDevice *GfxDevice::sInstance = nullptr;

    void GfxDevice::onDeviceLostOrFatalError()
    {
        m_state = DeviceState::kRemoved;
        dumpResourcesToLog();
    }

    GfxDevice::GfxDevice(GfxBackendType type, GfxShaderProfile profile)
        : m_state(DeviceState::kMissing), m_backend(type), m_isRendering(false), m_isDisposing(false), m_wasVSyncUsed(false), m_drawGpuEventIndex(0), m_frameIndex(0), m_shaderProfile(profile), 
        m_featureLevel(GfxUtils::getFeatureLevel(profile)), m_res(TkMemory::newObject<PrivateData>()), m_resources(1024), totalMemory(0), isDebuging(false)
    {
        TK_ASSERT(m_backend != GfxBackendType::kUnknown);
    }

    GfxDevice::~GfxDevice()
    {
        if (m_state != DeviceState::kDisposed)
            preDispose(false);

        TkMemory::deletePtr(m_res);

        // Unlink
        TK_ASSERT(GfxDevice::sInstance == this);
        GfxDevice::sInstance = nullptr;
    }

    bool GfxDevice::init()
    {
        // Clamp texture limits (eg. if driver reports higher value)
        limits.maxTexture1DSize = TkMath::min(limits.maxTexture1DSize, GFX_MAX_TEXTURE_SIZE);
        limits.maxTexture2DSize = TkMath::min(limits.maxTexture2DSize, GFX_MAX_TEXTURE_SIZE);
        limits.maxTexture3DSize = TkMath::min(limits.maxTexture3DSize, GFX_MAX_TEXTURE_SIZE);
        limits.maxTextureCubeSize = TkMath::min(limits.maxTextureCubeSize, GFX_MAX_TEXTURE_SIZE);
        limits.maxTexture1DArraySize = TkMath::min(limits.maxTexture1DArraySize, GFX_MAX_TEXTURE_ARRAY_SIZE);
        limits.maxTexture2DArraySize = TkMath::min(limits.maxTexture2DArraySize, GFX_MAX_TEXTURE_ARRAY_SIZE);
        limits.maxMipLevelsCount = TkMath::min(limits.maxMipLevelsCount, GFX_MAX_TEXTURE_MIP_LEVELS);

        m_res->tskMgr.setExecutor(createTasksExecutor());
        LogInfo("Total graphics memory: %s", TkConvertor::bytesToText(totalMemory));
        if (!limits.hasCompute)
            LogWarn("Compute Shaders are not supported");
        m_state = DeviceState::kCreated;
        return true;
    }

    bool GfxDevice::initInternalResources()
    {
        static const float fullscreenTriangle[] =
        {
            -1.0f, -1.0f, 0.0f, 1.0f,
            -1.0f, 3.0f, 0.0f, -1.0f,
            3.0f, -1.0f, 2.0f, 1.0f,
        };

        GfxVertexLayout::Elements elements;
        elements.add(VertexElement{VertexElementTypes::kPosition, 0, 0, 0, PixelFormat::kR32G32_Float});
        elements.add(VertexElement{VertexElementTypes::kTexCoord0, 0, 8, 0, PixelFormat::kR32G32_Float});

        auto *layout = createVertexLayout(elements, true);
        if (!layout)
        {
            LogError("Cannot initialize fullscreen triangle vertex layout.");
            return false;
        }

        m_res->fullscreenTriangleVB = createBuffer("FullscreenTriangleVB");
        if (!m_res->fullscreenTriangleVB)
        {
            LogError("Cannot create fullscreen triangle vertex buffer.");
            return false;
        }

        if (!m_res->fullscreenTriangleVB->init(GfxBufferDescription::vertex(layout, sizeof(float) * 4, 3, fullscreenTriangle)))
        {
            LogError("Cannot initialize fullscreen triangle vertex buffer.");
            GFX_DEL_RESOURCE(m_res->fullscreenTriangleVB);
            return false;
        }

        m_state = DeviceState::kReady;
        return true;
    }

    bool GfxDevice::canDraw()
    {
        return m_state == DeviceState::kCreated || m_state == DeviceState::kReady;
    }

    void GfxDevice::addResource(GfxResource *resource)
    {
        m_resourcesLock.lock();
        TK_ASSERT(resource && !m_resources.contains(resource));
        m_resources.add(resource);
        m_resourcesLock.unlock();
    }

    void GfxDevice::removeResource(GfxResource *resource)
    {
        m_resourcesLock.lock();
        TK_ASSERT(resource && m_resources.contains(resource));
        m_resources.remove(resource);
        m_resourcesLock.unlock();
    }

    void GfxDevice::dumpResourcesToLog()
    {
        TkStringBuilder output;
        m_resourcesLock.lock();

        u64 totalMemoryUsed = 0;
        for (i32 i = 0; i < m_resources.count(); i++)
            totalMemoryUsed += m_resources[i]->getMemoryUse();

        output.appendFormat("GPU Resources dump. Count: %i, total GPU memory used: %s", m_resources.count(), TkConvertor::bytesToText(totalMemoryUsed));
        output.appendLine();
        output.appendLine();

        const bool printTypes[(i32)GfxResType::MAX] =
            {
                true,  // RenderTarget
                true,  // Texture
                true,  // CubeTexture
                true,  // VolumeTexture
                true,  // Buffer
                true,  // Shader
                false, // PipelineState
                false, // Descriptor
                false, // Query
                false, // Sampler
            };
        for (i32 typeIndex = 0; typeIndex < (i32)GfxResType::MAX; typeIndex++)
        {
            const auto type = static_cast<GfxResType>(typeIndex);
            const auto printType = printTypes[typeIndex];

            output.appendFormat("Group: %is", type);
            output.appendLine();

            i32 count = 0;
            u64 memUsage = 0;
            for (i32 i = 0; i < m_resources.count(); i++)
            {
                const GfxResource *resource = m_resources[i];
                if (resource->getType() == type && resource->getMemoryUse() != 0)
                {
                    count++;
                    memUsage += resource->getMemoryUse();
                    auto str = resource->toString();
                    if (str.hasChars() && printType)
                    {
                        output.append('\t');
                        output.append(str);
                        output.appendLine();
                    }
                }
            }

            output.appendFormat("Total count: %i, memory usage: %s", count, TkConvertor::bytesToText(memUsage));
            output.appendLine();
            output.appendLine();
        }

        m_resourcesLock.unlock();
        LogInfo("%s", output.toStringView());
    }

    extern void clearVertexLayoutCache();

    void GfxDevice::preDispose()
    {
        preDispose(true);
    }

    void GfxDevice::preDispose(bool waitForGpu)
    {
        if (m_state == DeviceState::kDisposing || m_state == DeviceState::kDisposed)
            return;

        m_isDisposing = true;
        m_state = DeviceState::kDisposing;

        m_res->tskMgr.dispose();
        if (waitForGpu)
            waitForGPU();

        GfxTargetPool::flush(true);
        GFX_DEL_RESOURCE(m_res->copyLinearPS);
        GFX_DEL_RESOURCE(m_res->clearPS);
        GFX_DEL_RESOURCE(m_res->decodeYUY2PS);
        GFX_DEL_RESOURCE(m_res->decodeNV12PS);
        GFX_DEL_RESOURCE(m_res->fullscreenTriangleVB);
        clearVertexLayoutCache();

        TkArray<GfxResource *> resources;
        m_resourcesLock.lock();
        resources = m_resources;
        m_resources.clear();
        m_resourcesLock.unlock();

        for (i32 i = resources.count() - 1; i >= 0 && i < resources.count(); i--)
            resources[i]->onDeviceDispose();

        m_state = DeviceState::kDisposed;
    }

    void GfxDevice::drawBegin()
    {
        beginFrame();
    }

    void GfxDevice::drawEnd()
    {
        endFrame();
    }

    void GfxDevice::renderBegin()
    {
#if TK_PROFILER
        m_drawGpuEventIndex = ProfilerGPU::beginEvent(("Draw"));
#endif
    }

    void GfxDevice::renderEnd()
    {
#if TK_PROFILER
        ProfilerGPU::endEvent(m_drawGpuEventIndex);
#endif
    }

    GfxTaskContext *GfxDevice::createTasksContext()
    {
        return TkMemory::newObject<GfxTaskContext>(TkMemory::getDefault(), this);
    }

    GfxTaskExecutor *GfxDevice::createTasksExecutor()
    {
        return TkMemory::newObject<DefaultGfxTasksExecutor>();
    }

    void GfxDevice::draw()
    {
        beginFrame();
        endFrame();
    }

    void GfxDevice::beginFrame()
    {
        m_isRendering = true;
        ++m_frameIndex;

        auto *context = getMainContext();
        context->frameBegin();
        renderBegin();
        m_res->tskMgr.frameBegin();
    }

    void GfxDevice::endFrame()
    {
        PROFILE_NAMED("GfxFrameEnd");

        m_res->tskMgr.frameEnd();
        renderEnd();

        auto *context = getMainContext();
        context->frameEnd();
        context->flush();

        m_wasVSyncUsed = false;
        m_isRendering = false;
        GfxTargetPool::flush();
    }

    void GfxDevice::dispose()
    {
        if (m_state == DeviceState::kDisposing || m_state == DeviceState::kDisposed)
            return;
        preDispose();
    }

    u64 GfxDevice::getMemoryUsage()
    {
        u64 result = 0;
        m_resourcesLock.lock();
        for (i32 i = 0; i < m_resources.count(); i++)
            result += m_resources[i]->getMemoryUse();
        m_resourcesLock.unlock();
        return result;
    }

    TkArray<GfxResource *> GfxDevice::getResources()
    {
        m_resourcesLock.lock();
        TkArray<GfxResource *> result = m_resources;
        m_resourcesLock.unlock();
        return result;
    }

    GfxTaskManager *GfxDevice::getTaskManager() const
    {
        return &m_res->tskMgr;
    }

    GfxPipelineState *GfxDevice::getCopyLinearPS() const
    {
        return m_res->copyLinearPS;
    }

    GfxPipelineState *GfxDevice::getClearPS() const
    {
        return m_res->clearPS;
    }

    GfxPipelineState *GfxDevice::getDecodeYUY2PS() const
    {
        return m_res->decodeYUY2PS;
    }

    GfxPipelineState *GfxDevice::getDecodeNV12PS() const
    {
        return m_res->decodeNV12PS;
    }

    GfxBuffer *GfxDevice::getFullscreenTriangleVB() const
    {
        return m_res->fullscreenTriangleVB;
    }

}
