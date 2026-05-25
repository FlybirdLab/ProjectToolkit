/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GraphicsSandbox.hpp"

#include <toolkit/debug/Logger.hpp>
#include <toolkit/graphics/GfxContext.hpp>
#include <toolkit/graphics/GfxDevice.hpp>
#include <toolkit/graphics/GfxManager.hpp>
#include <toolkit/graphics/GfxPipelineState.hpp>
#include <toolkit/graphics/GfxResource.hpp>
#include <toolkit/graphics/GfxShader.hpp>
#include <toolkit/graphics/GfxSwapChain.hpp>
#include <toolkit/graphics/GfxWindow.hpp>
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/platforms/ProcessInitArgs.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/windowing/Window.hpp>
#include <toolkit/windowing/WindowSystem.hpp>

#if TK_WINDOWS
#include <Windows.h>
#endif

#if GFX_API_DIRECTX12
#define SANDBOX_GFX_BACKEND_ARG "dx"
#define SANDBOX_GFX_SHADER_CACHE_NAME "Triangle_dx_sm6.shadercache"
#define SANDBOX_GFX_BACKEND_NAME "DirectX 12"
#define SANDBOX_GFX_BACKEND_TYPE GfxBackendType::kDirectX12
#elif GFX_API_DIRECTX11
#define SANDBOX_GFX_BACKEND_ARG "d3d"
#define SANDBOX_GFX_SHADER_CACHE_NAME "Triangle_d3d_sm5.shadercache"
#define SANDBOX_GFX_BACKEND_NAME "DirectX 11"
#define SANDBOX_GFX_BACKEND_TYPE GfxBackendType::kDirectX11
#elif GFX_API_VULKAN
#define SANDBOX_GFX_BACKEND_ARG "vk"
#define SANDBOX_GFX_SHADER_CACHE_NAME "Triangle_vk_sm5.shadercache"
#define SANDBOX_GFX_BACKEND_NAME "Vulkan"
#define SANDBOX_GFX_BACKEND_TYPE GfxBackendType::kVulkan
#else
#error "Sandbox: no supported graphics backend is enabled (expected GFX_API_DIRECTX12, GFX_API_DIRECTX11 or GFX_API_VULKAN)."
#endif

namespace toolkit
{
    namespace
    {
        bool compileTriangleShader(TkString &cachePath)
        {
            const TkString shaderPath = TkString(SANDBOX_SHADER_SRC_DIR) / "Triangle.shader";
            const TkString outputDir = System::getWorkDir() / "shader-output";
            if (!TkFileSystem::isDirExist(outputDir) && !TkFileSystem::makeDir(outputDir))
            {
                LogError("Failed to create shader output directory: %s", outputDir.getText());
                return false;
            }

            ProcessInitArgs compilerArgs;
            compilerArgs.file = TkString::format("\"%s\"", SANDBOX_SHADER_COMPILER_PATH);
            compilerArgs.args = TkString::format("--file \"%s\" --backend " SANDBOX_GFX_BACKEND_ARG " --out \"%s\"", shaderPath.getText(), outputDir.getText());
            compilerArgs.workDir = System::getWorkDir();
            compilerArgs.logOutput = true;
            compilerArgs.saveOutput = false;
            compilerArgs.waitForQuit = true;
            compilerArgs.hiddenWindow = true;

            const i32 result = System::newProcess(compilerArgs);
            if (result != 0)
            {
                LogError("ShaderCompiler failed with exit code %d.", result);
                return false;
            }

            cachePath = outputDir / SANDBOX_GFX_SHADER_CACHE_NAME;
            if (!TkFileSystem::isFileExist(cachePath))
            {
                LogError("Compiled shader cache was not produced: %s", cachePath.getText());
                return false;
            }
            return true;
        }

        bool loadShaderCache(const TkString &path, TkArray<byte> &bytes)
        {
            OpenFileParams params{path, FileMode::kOpenExisting, FileAccess::kRead, FileShare::kReadWrite};
            FileHandle file = TkFileSystem::openFile(params);
            if (!file)
            {
                LogError("Failed to open compiled shader cache: %s", path.getText());
                return false;
            }

            const bool loaded = TkFileSystem::readAsBytes(file, bytes);
            TkFileSystem::closeFile(file);
            if (!loaded || bytes.isEmpty())
            {
                LogError("Failed to read compiled shader cache: %s", path.getText());
                return false;
            }
            return true;
        }
    }

    int GraphicsSandbox::run()
    {
#if TK_WINDOWS
        WindowSystem::init(GetModuleHandleW(nullptr));
#else
        WindowSystem::init(nullptr);
#endif

        TkString shaderCachePath;
        if (!compileTriangleShader(shaderCachePath))
        {
            WindowSystem::dispose();
            return 1;
        }

        if (!GfxManager::createDevice(SANDBOX_GFX_BACKEND_TYPE))
        {
            LogError("Failed to create " SANDBOX_GFX_BACKEND_NAME " graphics device.");
            WindowSystem::dispose();
            return 1;
        }

        WindowParams params;
        params.title = "Toolkit Sandbox - Graphics Triangle (" SANDBOX_GFX_BACKEND_NAME ")";
        params.size = Pos(960.0f, 540.0f);
        params.startPos = WindowStartPos::kCenterScreen;

        GfxWindow *window = TkMemory::newObject<GfxWindow>(TkMemory::getDefault(), params);
        if (!window || !WindowSystem::createWindow(window) || !window->initSwapChain())
        {
            LogError("Failed to create sandbox graphics window.");
            if (window)
                WindowSystem::destroyWindow(window);
            GfxManager::disposeDevice();
            WindowSystem::dispose();
            return 1;
        }

        TkArray<byte> shaderBytes;
        GfxShader *shader = nullptr;
        GfxPipelineState *pipeline = nullptr;
        bool initialized = false;

        if (loadShaderCache(shaderCachePath, shaderBytes))
        {
            MemoryIStream shaderStream(shaderBytes);
            GfxDevice *device = GfxManager::getDevice();
            shader = device->createShader("Sandbox.TriangleShader");
            if (shader && shader->create(shaderStream))
            {
                auto *vs = shader->getVS("VSMain");
                auto *ps = shader->getPS("PSMain");
                if (vs && ps)
                {
                    pipeline = device->createPipelineState();

                    GfxPipelineState::Description desc = GfxPipelineState::Description::DefaultNoDepth;
                    desc.vs = vs;
                    desc.ps = ps;
                    desc.primitiveTopology = GfxPrimitiveTopologyType::kTriangle;
                    desc.cullMode = GfxCullMode::kTwoSided;

                    initialized = pipeline != nullptr && pipeline->init(desc);
                }
            }
            shaderBytes.clear();
        }

        if (!initialized)
        {
            LogError("Failed to initialize triangle rendering resources.");
            GFX_DEL_RESOURCE(pipeline);
            GFX_DEL_RESOURCE(shader);
            WindowSystem::destroyWindow(window);
            GfxManager::disposeDevice();
            WindowSystem::dispose();
            return 1;
        }

        bool isRunning = true;
        window->onClosed.bind([&]()
        {
            isRunning = false;
        });
        window->show();

        GfxDevice *device = GfxManager::getDevice();
        GfxContext *context = device->getMainContext();
        GfxSwapChain *swapChain = window->getSwapChain();
        bool hasPendingResize = false;
        i32 pendingWidth = swapChain ? swapChain->getWidth() : 0;
        i32 pendingHeight = swapChain ? swapChain->getHeight() : 0;
        window->onResized.bind([&](Pos size)
        {
            pendingWidth = static_cast<i32>(size.x);
            pendingHeight = static_cast<i32>(size.y);
            hasPendingResize = true;
        });

        while (isRunning && !window->isClosed())
        {
            WindowSystem::update();
            if (window->isMinimized())
            {
                TkThread::sleep(5);
                continue;
            }

            if (hasPendingResize)
            {
                hasPendingResize = false;
                if (pendingWidth > 0 && pendingHeight > 0 &&
                    (pendingWidth != swapChain->getWidth() || pendingHeight != swapChain->getHeight()))
                {
                    swapChain->resize(pendingWidth, pendingHeight);
                }
            }

            if (!swapChain->isReady())
            {
                TkThread::sleep(5);
                continue;
            }

            device->beginFrame();
            swapChain->begin();

            GfxTextureView *backBuffer = swapChain->getBackBufferView();
            if (backBuffer)
            {
                context->setRenderTarget(backBuffer);
                context->setViewportAndScissors(static_cast<float>(swapChain->getWidth()), static_cast<float>(swapChain->getHeight()));
                context->setState(pipeline);
                context->draw(0, 3);
            }

            swapChain->end();
            device->endFrame();
            swapChain->present(GfxManager::useVSync);
        }

        device->waitForGPU();
        GFX_DEL_RESOURCE(pipeline);
        GFX_DEL_RESOURCE(shader);
        WindowSystem::destroyWindow(window);
        GfxManager::disposeDevice();
        WindowSystem::dispose();
        return 0;
    }
}
