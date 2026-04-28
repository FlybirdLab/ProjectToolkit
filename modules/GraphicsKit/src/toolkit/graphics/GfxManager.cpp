/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxManager.hpp"
#include "GfxDevice.hpp"
#include "GfxAdapter.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/graphics/async/GfxTaskMgr.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/Version.hpp"

namespace toolkit
{

    bool GfxManager::useVSync = false;
    TkMutex GfxManager::sharedMutex {};

#if GFX_API_NULL
    extern GfxDevice *createDeviceNull();
#endif
#if GFX_API_VULKAN
    extern GfxDevice *createDeviceVulkan();
#endif
#if GFX_API_DIRECTX11
    extern GfxDevice *createDeviceDX11();
#endif
#if GFX_API_DIRECTX12
    extern GfxDevice *createDeviceDX12();
#endif
#if GFX_API_OPENGL || GFX_API_OPENGLES
    extern GfxDevice *createDeviceOGL();
#endif
#if GFX_API_METAL
    extern GfxDevice *createDeviceMetal();
#endif

    GfxDevice *GfxManager::getDevice()
    {
        return GfxDevice::sInstance;
    }

    static GfxDevice *createDeviceForBackend(GfxBackendType backend)
    {
        switch (backend)
        {
#if GFX_API_NULL
        case GfxBackendType::kNull:
            return createDeviceNull();
#endif
#if GFX_API_VULKAN
        case GfxBackendType::kVulkan:
            return createDeviceVulkan();
#endif
#if GFX_API_DIRECTX11
        case GfxBackendType::kDirectX11:
            return createDeviceDX11();
#endif
#if GFX_API_DIRECTX12
        case GfxBackendType::kDirectX12:
            return createDeviceDX12();
#endif
#if GFX_API_OPENGL || GFX_API_OPENGLES
        case GfxBackendType::kOpenGL4_1:
        case GfxBackendType::kOpenGL4_4:
        case GfxBackendType::kOpenGLES3:
        case GfxBackendType::kOpenGLES3_1:
            return createDeviceOGL();
#endif
#if GFX_API_METAL
        case GfxBackendType::kMetal:
            return createDeviceMetal();
#endif
        default:
            return nullptr;
        }
    }

    bool GfxManager::createDevice(GfxBackendType backend)
    {
        TK_ASSERT(GfxDevice::sInstance == nullptr);

        GfxDevice *device = createDeviceForBackend(backend);
        if (!device)
        {
            LogError("Requested graphics backend is not enabled or failed to create.");
            return false;
        }

        GfxDevice::sInstance = device;
        if (!device->init() || !device->initInternalResources())
        {
            disposeDevice();
            return false;
        }

        return true;
    }

    void GfxManager::disposeDevice()
    {
        if (GfxDevice::sInstance)
        {
            GfxDevice::sInstance->dispose();
            TkMemory::deletePtr(GfxDevice::sInstance);
            GfxDevice::sInstance = nullptr;
        }
    }

    bool GfxManager::init()
    {
        TK_ASSERT(GfxDevice::sInstance == nullptr);

        LogInfo("[Graphics] Creating device...");
        GfxPixelFormat::init();
        GfxDevice *device = nullptr;

#if TK_GRAPHICS_BACKEND_NONE
        LogError("No graphics backend enabled.");
        return false;
#elif GFX_API_NULL
        device = createDeviceNull();
#elif GFX_API_DIRECTX11
        device = createDeviceDX11();
#elif GFX_API_DIRECTX12
        device = createDeviceDX12();
#elif GFX_API_VULKAN
        device = createDeviceVulkan();
#elif GFX_API_OPENGL
        device = createDeviceOGL();
#elif GFX_API_OPENGLES
        device = createDeviceOGL();
#elif GFX_API_METAL
        device = createDeviceMetal();
#else
        LogError("No graphics backend enabled.");
        return false;
#endif

        if (!device)
        {
            LogError("Failed to create graphics device.");
            return false;
        }

        GfxDevice::sInstance = device;
        if (!device->init())
        {
            disposeDevice();
            return false;
        }

        if (!device->initInternalResources())
        {
            disposeDevice();
            return false;
        }

        const TkString adapterDescription = device->getAdapter()->getDescription();
        const TkString driverVersion = device->getAdapter()->getDriverVersion().toString();
        LogInfo("GPU Device created: %s", adapterDescription.getText());
        LogInfo("Shader Profile: %i, Feature Level: %i, Driver: %s",
            static_cast<i32>(device->getShaderProfile()),
            static_cast<i32>(device->getFeatureLevel()),
            driverVersion.getText());

        if (device->isDebuging)
        {
#if !TK_DEBUG && !TK_LINUX
            TK_ASSERT(false && "[Graphics] Debugger attached.");
#endif
#if TK_WITH_PROFILER
            ProfilerGPU::eventsEnabled = true;
#endif
        }

        return true;
    }

    void GfxManager::cleanup()
    {
        if (GfxDevice::sInstance)
        {
            // Start disposing
            GfxDevice::sInstance->getTaskManager()->dispose();
        }
    }

    void GfxManager::dispose()
    {
        // Device lifetime is controlled by the graphics manager.
    }

}
