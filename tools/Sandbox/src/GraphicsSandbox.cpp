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
#include <toolkit/utils/CommandLine.hpp>
#include <toolkit/windowing/Window.hpp>
#include <toolkit/windowing/WindowSystem.hpp>

#if TK_WINDOWS
#include <Windows.h>
#endif

#ifndef SANDBOX_DEFAULT_BACKEND_NAME
#define SANDBOX_DEFAULT_BACKEND_NAME ""
#endif

namespace toolkit
{
    namespace
    {
        struct BackendDescriptor
        {
            const char *cliName;          // matches --backend=<this>
            const char *compilerArg;      // passed to ShaderCompiler --backend
            const char *shaderFileName;   // shader source under SANDBOX_SHADER_SRC_DIR
            const char *cacheFileName;    // produced shader cache name
            const char *displayName;
            GfxBackendType type;
        };

        constexpr BackendDescriptor kBackends[] = {
#if GFX_API_DIRECTX12
            {"dx12",   "dx",     "Triangle.shader",      "Triangle_dx_sm6.shadercache",  "DirectX 12",        GfxBackendType::kDirectX12},
#endif
#if GFX_API_DIRECTX11
            {"dx11",   "d3d",    "Triangle.shader",      "Triangle_d3d_sm5.shadercache", "DirectX 11",        GfxBackendType::kDirectX11},
#endif
#if GFX_API_VULKAN
            {"vulkan", "vk",     "Triangle.shader",      "Triangle_vk_sm5.shadercache",  "Vulkan",            GfxBackendType::kVulkan},
#endif
#if GFX_API_OPENGL
            // OpenGL fallback: uses a parallel GLSL 410 source file because the
            // GLSL passthrough compiler can't ingest the HLSL Triangle.shader.
            {"opengl", "opengl", "Triangle.glsl.shader", "Triangle.glsl_glsl410.shadercache", "OpenGL 4.1",   GfxBackendType::kOpenGL4_1},
#endif
        };

        constexpr i32 kBackendCount = sizeof(kBackends) / sizeof(kBackends[0]);

        static_assert(kBackendCount > 0, "Sandbox needs at least one graphics backend enabled at compile time.");

        const BackendDescriptor *pickBackend(const TkCommandLine &cmd)
        {
            TkString requested = cmd.get("backend");
            if (requested.isEmpty())
                requested = cmd.get("b");
            if (requested.isEmpty())
                requested = TkString(SANDBOX_DEFAULT_BACKEND_NAME);

            if (!requested.isEmpty())
            {
                requested = requested.toLowerAscii();
                for (i32 i = 0; i < kBackendCount; ++i)
                {
                    if (requested == kBackends[i].cliName)
                        return &kBackends[i];
                }
                LogWarn("Sandbox: --backend=%s is not compiled in; falling back to the first available backend.", requested.getText());
            }
            return &kBackends[0];
        }

        void logAvailableBackends()
        {
            LogInfo("Available graphics backends (--backend=<name>):");
            for (i32 i = 0; i < kBackendCount; ++i)
                LogInfo("  %s - %s", kBackends[i].cliName, kBackends[i].displayName);
        }

        bool compileTriangleShader(const BackendDescriptor &backend, TkString &cachePath)
        {
            const TkString shaderPath = TkString(SANDBOX_SHADER_SRC_DIR) / backend.shaderFileName;
            const TkString outputDir = System::getWorkDir() / "shader-output";
            if (!TkFileSystem::isDirExist(outputDir) && !TkFileSystem::makeDir(outputDir))
            {
                LogError("Failed to create shader output directory: %s", outputDir.getText());
                return false;
            }

            ProcessInitArgs compilerArgs;
            compilerArgs.file = TkString::format("\"%s\"", SANDBOX_SHADER_COMPILER_PATH);
            compilerArgs.args = TkString::format("--file \"%s\" --backend %s --out \"%s\"",
                                                 shaderPath.getText(),
                                                 backend.compilerArg,
                                                 outputDir.getText());
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

            cachePath = outputDir / backend.cacheFileName;
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

    int GraphicsSandbox::run(const TkCommandLine &commandLine)
    {
        if (commandLine.has("list-backends"))
        {
            logAvailableBackends();
            return 0;
        }

        const BackendDescriptor *backend = pickBackend(commandLine);
        LogInfo("Sandbox using backend: %s (%s)", backend->displayName, backend->cliName);

#if TK_WINDOWS
        WindowSystem::init(GetModuleHandleW(nullptr));
#else
        WindowSystem::init(nullptr);
#endif

        TkString shaderCachePath;
        if (!compileTriangleShader(*backend, shaderCachePath))
        {
            WindowSystem::dispose();
            return 1;
        }

        if (!GfxManager::createDevice(backend->type))
        {
            LogError("Failed to create %s graphics device.", backend->displayName);
            WindowSystem::dispose();
            return 1;
        }

        WindowParams params;
        params.title = TkString::format("Toolkit Sandbox - Graphics Triangle (%s)", backend->displayName);
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
