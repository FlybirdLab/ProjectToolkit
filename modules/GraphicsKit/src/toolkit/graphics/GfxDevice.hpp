/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/utils/FormatSupport.hpp>
#include <toolkit/graphics/GfxLimits.hpp>
#include "toolkit/threading/Sync.hpp"
#include "toolkit/windowing/Window.hpp"
#include <toolkit/collection/Array.hpp>
#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    class GfxWindow;

    enum class DeviceState
    {
        kMissing = 0,
        kCreated,
        kReady,
        kRemoved,
        kDisposing,
        kDisposed
    };

    struct VideoOutputMode
    {
        u32 width = 0;
        u32 height = 0;
        u32 refreshRate = 0;
    };

    // Graphics device object for rendering on GPU.
    class TK_API GfxDevice
    {
    public:
        // The singleton instance of the graphics device.
        static GfxDevice *sInstance;
        static GfxDevice *getInstance()
        {
            return sInstance;
        }

    protected:
        // State
        DeviceState m_state;
        GfxBackendType m_backend;
        bool m_isRendering;
        bool m_isDisposing;
        bool m_wasVSyncUsed;
        i32 m_drawGpuEventIndex;
        u64 m_frameIndex;
        GfxShaderProfile m_shaderProfile;
        GfxFeatureLevel m_featureLevel;

        // Private resources (hidden with declaration)
        struct PrivateData;
        PrivateData *m_res;
        TkArray<GfxResource *> m_resources;
        TkMutex m_resourcesLock;

    protected:
        GfxDevice(GfxBackendType type, GfxShaderProfile profile);

    public:
        virtual ~GfxDevice();

    public:
        // The graphics device locking mutex.
        mutable TkMutex mtx;
        // The total amount of graphics memory in bytes.
        u64 totalMemory;
        // Indicates that debug tool is profiling device (eg. RenderDoc).
        bool isDebuging;
        // The GPU limits.
        GfxLimits limits;

        // The available video output modes.
        TkArray<VideoOutputMode> outputs;
        // The supported features for the specified format (index is the pixel format value).
        FormatFeatures features[static_cast<i32>(PixelFormat::MAX)];
        // Get the supported features for the specified format (index is the pixel format value).
        FormatFeatures getFormatFeatures(const PixelFormat format) const
        {
            return features[(i32)format];
        }

    public:
        // Get current device state.
        TK_FORCE_INLINE DeviceState getState() const
        {
            return m_state;
        }
        // Get device backend type.
        TK_FORCE_INLINE GfxBackendType getBackendType() const
        {
            return m_backend;
        }

        // Returns true if device is during rendering state, otherwise false.
        TK_FORCE_INLINE bool isRendering() const
        {
            return m_isRendering;
        }

        TK_FORCE_INLINE bool isDisposing() const
        {
            return m_isDisposing;
        }

        TK_FORCE_INLINE u64 getFrameIndex() const
        {
            return m_frameIndex;
        }

        // Returns true if VSync was used during the last frame present.
        TK_FORCE_INLINE bool wasVSyncUsed() const
        {
            return m_wasVSyncUsed;
        }

        // Get device shader profile type.
        TK_FORCE_INLINE GfxShaderProfile getShaderProfile() const
        {
            return m_shaderProfile;
        }

        // Get device feature level type.
        TK_FORCE_INLINE GfxFeatureLevel getFeatureLevel() const
        {
            return m_featureLevel;
        }

        // Get the main GPU context.
        virtual class GfxContext *getMainContext() = 0;
        // Get the adapter device.
        virtual class GfxAdapter *getAdapter() const = 0;
        // Get the native pointer to the underlying graphics device. It's a low-level platform-specific handle.
        virtual void *getNativePtr() const = 0;

        // Get the amount of memory usage by all the GPU resources (in bytes).
        u64 getMemoryUsage();

        // Get the list with all active GPU resources.
        TkArray<GfxResource *> getResources();
        // Get the GPU asynchronous work manager.
        class GfxTaskManager *getTaskManager() const;
        GfxPipelineState *getCopyLinearPS() const;
        GfxPipelineState *getClearPS() const;
        GfxPipelineState *getDecodeYUY2PS() const;
        GfxPipelineState *getDecodeNV12PS() const;
        GfxBuffer *getFullscreenTriangleVB() const;

    public:
        // Init device resources
        virtual bool init();
        // Initialize private RHI resources.
        virtual bool initInternalResources();
        // Checks if GPU can render frame now (all data is ready), otherwise will skip frame rendering.
        virtual bool canDraw();
        // Begins a GPU frame. Does not submit higher-level rendering or present swapchains.
        virtual void beginFrame();
        // Ends a GPU frame. Does not submit higher-level rendering or present swapchains.
        virtual void endFrame();
        // Compatibility wrapper for beginFrame/endFrame.
        virtual void draw();
        // Clean all allocated data by device
        virtual void dispose();
        // Wait for GPU end doing submitted work
        virtual void waitForGPU() = 0;

    public:
        void addResource(GfxResource *resource);
        void removeResource(GfxResource *resource);

        // Dumps all GPU resources information to the log.
        void dumpResourcesToLog();
        void onDeviceLostOrFatalError();

    protected:
        virtual void preDispose();
        void preDispose(bool waitForGpu);

        // Compatibility wrapper for beginFrame().
        virtual void drawBegin();
        // Compatibility wrapper for endFrame().
        virtual void drawEnd();

        // Called during Draw method after rendering begin. Can be used to submit commands to the GPU after opening GPU command list.
        virtual void renderBegin();
        // Called during Draw method before rendering end. Can be used to submit commands to the GPU before closing GPU command list.
        virtual void renderEnd();

    public:
        // Create the texture.
        virtual GfxTexture *createTexture(const TkStringView &name = TkStringView::Empty) = 0;
        // Create the shader.
        virtual GfxShader *createShader(const TkStringView &name = TkStringView::Empty) = 0;
        // Create the GPU pipeline state object.
        virtual GfxPipelineState *createPipelineState() = 0;
        // Create the timer query object.
        virtual class GfxTimerQuery *createTimerQuery() = 0;
        // Create the buffer.
        virtual GfxBuffer *createBuffer(const TkStringView &name = TkStringView::Empty) = 0;
        // Create the texture sampler.
        virtual GfxSampler *createSampler() = 0;

        // Create the vertex buffer layout.
        virtual GfxVertexLayout *createVertexLayout(const TkStaticArray<struct VertexElement, GFX_MAX_VS_ELEMENTS> &elements, bool explicitOffsets = false) = 0;
        typedef TkStaticArray<VertexElement, GFX_MAX_VS_ELEMENTS> VertexElements;

        // Create the native window swap chain.
        virtual GfxSwapChain *createSwapChain(GfxWindow *window) = 0;

        // Create the constant buffer.
        virtual class GfxConstantBuffer *createConstantBuffer(u32 size, const TkStringView &name = TkStringView::Empty) = 0;
        // Create the GPU tasks context.
        virtual class GfxTaskContext *createTasksContext();
        // Create the GPU tasks executor.
        virtual class GfxTaskExecutor *createTasksExecutor();
    };

    // Utility structure to safety graphics device locking.
    struct TK_API GfxDeviceLock : NonCopyable
    {
        const GfxDevice *device;

        GfxDeviceLock(const GfxDevice *device)
            : device(device)
        {
            device->mtx.lock();
        }

        ~GfxDeviceLock()
        {
            device->mtx.unlock();
        }
    };

}
