/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VideoSandbox.hpp"

#include <toolkit/audio/Audio.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/graphics/GfxContext.hpp>
#include <toolkit/graphics/GfxDevice.hpp>
#include <toolkit/graphics/GfxManager.hpp>
#include <toolkit/graphics/GfxPipelineState.hpp>
#include <toolkit/graphics/GfxResource.hpp>
#include <toolkit/graphics/GfxSampler.hpp>
#include <toolkit/graphics/GfxShader.hpp>
#include <toolkit/graphics/GfxSwapChain.hpp>
#include <toolkit/graphics/GfxTexture.hpp>
#include <toolkit/graphics/GfxWindow.hpp>
#include <toolkit/math/Vector2.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/platforms/ProcessInitArgs.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/shader/format/GfxShaderAssetLoader.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/utils/CommandLine.hpp>
#include <toolkit/utils/StringUtils.hpp>
#include <toolkit/video/VideoBackend.hpp>
#include <toolkit/video/VideoMgr.hpp>
#include <toolkit/video/VideoPlayer.hpp>
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
        constexpr f32 kPlaybackStartTimeoutSeconds = 8.0f;
        constexpr f32 kMinimumPlaybackTickRate = 30.0f;

        struct VideoBackendDescriptor
        {
            const char *cliName;
            const char *compilerArg;
            const char *displayName;
            GfxBackendType type;
        };

        constexpr VideoBackendDescriptor kBackends[] = {
#if GFX_API_DIRECTX12
            {"dx12",   "dx",     "DirectX 12", GfxBackendType::kDirectX12},
#endif
#if GFX_API_DIRECTX11
            {"dx11",   "d3d",    "DirectX 11", GfxBackendType::kDirectX11},
#endif
#if GFX_API_VULKAN
            {"vulkan", "vk",     "Vulkan",     GfxBackendType::kVulkan},
#endif
#if GFX_API_OPENGL
            {"opengl", "opengl", "OpenGL 4.1", GfxBackendType::kOpenGL4_1},
#endif
        };

        constexpr i32 kBackendCount = sizeof(kBackends) / sizeof(kBackends[0]);
        static_assert(kBackendCount > 0, "VideoSandbox needs at least one graphics backend enabled at compile time.");

        const VideoBackendDescriptor *pickBackend(const TkCommandLine &commandLine)
        {
            TkString requested = commandLine.get("backend");
            if (requested.isEmpty())
                requested = commandLine.get("b");
            if (requested.isEmpty())
                requested = TkString(SANDBOX_DEFAULT_BACKEND_NAME);

            if (!requested.isEmpty())
            {
                requested = requested.toLowerAscii();
                for (i32 i = 0; i < kBackendCount; i++)
                {
                    if (requested == kBackends[i].cliName)
                        return &kBackends[i];
                }
                LogWarn("[VideoSandbox] --backend=%s is not compiled in; falling back to the first available backend.",
                        requested.getText());
            }
            return &kBackends[0];
        }

        void logAvailableBackends()
        {
            LogInfo("[VideoSandbox] Available graphics backends (--backend=<name>):");
            for (i32 i = 0; i < kBackendCount; i++)
                LogInfo("  %s - %s", kBackends[i].cliName, kBackends[i].displayName);
        }

        TkString getTestFilePath(const TkCommandLine &commandLine)
        {
            TkString path;
            if (commandLine.get("file", path))
                return path;
            return TkString(SANDBOX_VIDEO_TEST_FILE);
        }

        bool parsePositiveFloat(const TkCommandLine &commandLine, const char *key, f32 &outValue)
        {
            TkString raw;
            if (!commandLine.get(key, raw))
                return false;

            const TkStringView view(raw.getText(), raw.length());
            if (!TkStringUtils::fromString(view, outValue) || outValue <= 0.0f)
            {
                LogError("[VideoSandbox] --%s must be a positive number.", key);
                return false;
            }
            return true;
        }

        bool makeVideoShaderOutputDir(const char *backendName, TkString &outputDir)
        {
            TkString rootDir = System::getWorkDir() / "shader-output";
            TkPath::normalize(rootDir);
            outputDir = rootDir / TkString(backendName);
            TkPath::normalize(outputDir);

            if (!TkFileSystem::isDirExist(rootDir) && !TkFileSystem::makeDir(rootDir))
                return false;
            return TkFileSystem::isDirExist(outputDir) || TkFileSystem::makeDir(outputDir);
        }

        bool compileVideoQuadShader(const VideoBackendDescriptor &backend, TkString &cachePath)
        {
            // Shader payloads are backend-specific; keep concurrent clients
            // from replacing the tksd that a different backend is loading.
            TkString outputDir;
            if (!makeVideoShaderOutputDir(backend.cliName, outputDir))
            {
                LogError("[VideoSandbox] Failed to create shader output directory: %s", outputDir.getText());
                return false;
            }

            const TkString shaderPath = TkString(SANDBOX_SHADER_SRC_DIR) / "VideoQuad.tkshader";
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
                LogError("[VideoSandbox] ShaderCompiler failed with exit code %d.", result);
                return false;
            }

            cachePath = outputDir / TkString("Sandbox_VideoQuad.tksd");
            if (!TkFileSystem::isFileExist(cachePath))
            {
                LogError("[VideoSandbox] Compiled .tksd was not produced: %s", cachePath.getText());
                return false;
            }
            return true;
        }
    }

    int VideoSandbox::run(const TkCommandLine &commandLine)
    {
        if (commandLine.has("list-backends"))
        {
            logAvailableBackends();
            return 0;
        }

        const TkString filePath = getTestFilePath(commandLine);
        LogInfo("[VideoSandbox] Test file: %s", filePath.getText());

        if (!TkFileSystem::isFileExist(TkStringView(filePath.getText(), filePath.length())))
        {
            LogError("[VideoSandbox] File not found: %s", filePath.getText());
            LogError("[VideoSandbox] Drop a short MP4 there or pass --file=<absolute path>.");
            return 1;
        }

        f32 maxPlaySeconds = 0.0f;
        bool hasMaxPlayDuration = false;
        if (commandLine.has("duration"))
        {
            if (!parsePositiveFloat(commandLine, "duration", maxPlaySeconds))
                return 1;
            hasMaxPlayDuration = true;
        }

        f32 startTimeSeconds = 0.0f;
        if (commandLine.has("start") && !parsePositiveFloat(commandLine, "start", startTimeSeconds))
            return 1;

        const bool loop = commandLine.has("loop");

        const VideoBackendDescriptor *backend = pickBackend(commandLine);
        LogInfo("[VideoSandbox] Using graphics backend: %s (%s)", backend->displayName, backend->cliName);
        GfxManager::useVSync = !commandLine.has("no-vsync");
        LogInfo("[VideoSandbox] VSync: %s", GfxManager::useVSync ? "on" : "off");

#if TK_WINDOWS
        WindowSystem::init(GetModuleHandleW(nullptr));
#else
        WindowSystem::init(nullptr);
#endif

        TkString shaderCachePath;
        if (!compileVideoQuadShader(*backend, shaderCachePath))
        {
            WindowSystem::dispose();
            return 1;
        }

        if (!GfxManager::createDevice(backend->type))
        {
            LogError("[VideoSandbox] Failed to create graphics device.");
            WindowSystem::dispose();
            return 1;
        }

        WindowParams params;
        params.title = TkString::format("Toolkit Sandbox - Video Playback (%s)", backend->displayName);
        params.size = Pos(960.0f, 540.0f);
        params.startPos = WindowStartPos::kCenterScreen;

        GfxWindow *window = TkMemory::newObject<GfxWindow>(TkMemory::getDefault(), params);
        if (!window || !WindowSystem::createWindow(window) || !window->initSwapChain())
        {
            LogError("[VideoSandbox] Failed to create sandbox graphics window.");
            if (window)
                WindowSystem::destroyWindow(window);
            GfxManager::disposeDevice();
            WindowSystem::dispose();
            return 1;
        }

        GfxDevice *device = GfxManager::getDevice();
        GfxShader *shader = nullptr;
        GfxPipelineState *pipeline = nullptr;
        GfxSampler *sampler = nullptr;
        bool resourcesReady = false;

        {
            TkString loadError;
            shader = GfxShaderAssetLoader::loadShaderFromTksd(
                device, shaderCachePath, TkStringView("Forward"), &loadError);
            if (shader == nullptr)
            {
                LogError("[VideoSandbox] Failed to load tksd shader '%s': %s",
                         shaderCachePath.getText(), loadError.getText());
            }
            else
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

                    if (pipeline != nullptr && pipeline->init(desc))
                    {
                        sampler = device->createSampler();
                        if (sampler != nullptr)
                        {
                            const GfxSamplerDescription samplerDesc = GfxSamplerDescription::newSampler(
                                GfxSamplerFilter::kBilinear, GfxSamplerAddressMode::kClamp);
                            resourcesReady = sampler->init(samplerDesc);
                        }
                    }
                }
            }
        }

        if (!resourcesReady)
        {
            LogError("[VideoSandbox] Failed to initialize video rendering resources.");
            GFX_DEL_RESOURCE(sampler);
            GFX_DEL_RESOURCE(pipeline);
            GFX_DEL_RESOURCE(shader);
            WindowSystem::destroyWindow(window);
            GfxManager::disposeDevice();
            WindowSystem::dispose();
            return 1;
        }

        if (!Video::init())
        {
            LogError("[VideoSandbox] Failed to initialize video system.");
            GFX_DEL_RESOURCE(sampler);
            GFX_DEL_RESOURCE(pipeline);
            GFX_DEL_RESOURCE(shader);
            WindowSystem::destroyWindow(window);
            GfxManager::disposeDevice();
            WindowSystem::dispose();
            return 1;
        }

        if (!Audio::init())
        {
            LogError("[VideoSandbox] Failed to initialize audio system.");
            Video::dispose();
            GFX_DEL_RESOURCE(sampler);
            GFX_DEL_RESOURCE(pipeline);
            GFX_DEL_RESOURCE(shader);
            WindowSystem::destroyWindow(window);
            GfxManager::disposeDevice();
            WindowSystem::dispose();
            return 1;
        }

        int result = 0;
        {
            VideoPlayer player;
            player.url = filePath;
            player.setIsLooping(loop);
            player.startTime = startTimeSeconds;

            player.play();
            if (player.getState() != VideoPlayerState::kPlaying)
            {
                LogError("[VideoSandbox] VideoPlayer failed to start playback (check backend / url).");
                result = 1;
            }
            else
            {
                if (startTimeSeconds > 0.0f)
                    player.setTime(startTimeSeconds);

                if (hasMaxPlayDuration)
                    LogInfo("[VideoSandbox] Playing %s for up to %.1f seconds%s. Close the window to stop.",
                            filePath.getText(), maxPlaySeconds, loop ? " (loop enabled)" : "");
                else
                    LogInfo("[VideoSandbox] Playing %s%s. Close the window to stop.",
                            filePath.getText(), loop ? " (loop enabled)" : "");

                bool isRunning = true;
                window->onClosed.bind([&]() {
                    isRunning = false;
                });

                bool hasPendingResize = false;
                i32 pendingWidth = 0;
                i32 pendingHeight = 0;
                window->onResized.bind([&](Pos size) {
                    pendingWidth = (i32)size.x;
                    pendingHeight = (i32)size.y;
                    hasPendingResize = true;
                });

                window->show();

                GfxContext *context = device->getMainContext();
                GfxSwapChain *swapChain = window->getSwapChain();

                double startDeadline = System::getTimeSeconds() + (double)kPlaybackStartTimeoutSeconds;
                double playbackDeadline = hasMaxPlayDuration
                                              ? System::getTimeSeconds() + (double)maxPlaySeconds
                                              : 0.0;
                bool reportedSize = false;
                bool reportedFrames = false;
                i32 lastFramesCount = 0;
                bool isInteractiveSizeMove = false;
                bool isPausedForPresentation = false;
                double presentationPauseStart = 0.0;
                double nextPlaybackTickAt = 0.0;

                auto pauseForPresentation = [&]() {
                    if (isPausedForPresentation || player.getState() != VideoPlayerState::kPlaying)
                        return;

                    player.pause();
                    isPausedForPresentation = true;
                    presentationPauseStart = System::getTimeSeconds();
                };

                auto resumeAfterPresentation = [&]() {
                    if (!isPausedForPresentation || window->isMinimized() ||
                        isInteractiveSizeMove || !swapChain->isReady())
                    {
                        return;
                    }

                    const double pausedSeconds = System::getTimeSeconds() - presentationPauseStart;
                    startDeadline += pausedSeconds;
                    if (hasMaxPlayDuration)
                        playbackDeadline += pausedSeconds;

                    // A modal window operation blocks this render loop. Reset
                    // the wall-clock delta so video resumes at the paused time.
                    Video::resetUpdateClock();
                    nextPlaybackTickAt = 0.0;
                    player.play();
                    isPausedForPresentation = false;
                };

                window->onSizeMoveBegin.bind([&]() {
                    isInteractiveSizeMove = true;
                    pauseForPresentation();
                });
                window->onSizeMoveEnd.bind([&]() {
                    isInteractiveSizeMove = false;
                });

                while (isRunning && !window->isClosed())
                {
                    WindowSystem::update();
                    if (window->isMinimized() || isInteractiveSizeMove)
                    {
                        pauseForPresentation();
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
                        pauseForPresentation();
                        TkThread::sleep(5);
                        continue;
                    }

                    resumeAfterPresentation();

                    // Updating and presenting at the display refresh rate
                    // repeats work for ordinary 24/30 fps media. Keep at
                    // least 30 Hz so the short audio queue remains fed.
                    const f32 mediaFrameRate = player.getFrameRate();
                    if (mediaFrameRate > 0.0f && nextPlaybackTickAt > 0.0)
                    {
                        const double now = System::getTimeSeconds();
                        if (now < nextPlaybackTickAt)
                        {
                            const i32 sleepMilliseconds = TkMath::max<i32>(
                                1, static_cast<i32>((nextPlaybackTickAt - now) * 1000.0));
                            TkThread::sleep(sleepMilliseconds);
                            continue;
                        }
                    }

                    const double playbackTickStartedAt = System::getTimeSeconds();
                    device->beginFrame();
                    swapChain->begin();

                    // Backend uploads the latest decoded frame to the GPU during this call,
                    // so it must happen inside an active frame.
                    Video::update();
                    Audio::update();
                    player.tick();

                    if (!reportedSize)
                    {
                        const IVec2 size = player.getSize();
                        if (size.x > 0 && size.y > 0)
                        {
                            LogInfo("[VideoSandbox] Decoded media info: %dx%d @ %.2ffps, duration=%.2fs",
                                    size.x, size.y, player.getFrameRate(), player.getDuration());
                            reportedSize = true;
                        }
                    }

                    const i32 frames = player.getFramesCount();
                    if (frames > lastFramesCount)
                    {
                        if (!reportedFrames)
                        {
                            LogInfo("[VideoSandbox] First decoded frame received at t=%.2fs", player.getTime());
                            reportedFrames = true;
                        }
                        lastFramesCount = frames;
                    }

                    GfxTextureView *backBuffer = swapChain->getBackBufferView();
                    if (backBuffer)
                    {
                        context->setRenderTarget(backBuffer);
                        context->setViewportAndScissors(
                            (f32)swapChain->getWidth(),
                            (f32)swapChain->getHeight());

                        GfxTexture *frame = player.getFrame();
                        if (frame != nullptr && frame->isAllocated() && reportedFrames)
                        {
                            context->setState(pipeline);
                            context->bindSR(0, frame);
                            const i32 samplerSlot = backend->type == GfxBackendType::kOpenGL4_1
                                                        ? 0
                                                        : GFX_STATIC_SAMPLERS_COUNT;
                            context->bindSampler(samplerSlot, sampler);
                            context->draw(0, 3);
                        }
                    }

                    swapChain->end();
                    device->endFrame();
                    swapChain->present(GfxManager::useVSync);

                    const f32 playbackTickRate = TkMath::max(kMinimumPlaybackTickRate, player.getFrameRate());
                    if (playbackTickRate > 0.0f)
                        nextPlaybackTickAt = playbackTickStartedAt + 1.0 / (double)playbackTickRate;

                    const double now = System::getTimeSeconds();
                    if (!reportedFrames && now >= startDeadline)
                    {
                        LogError("[VideoSandbox] No video frames decoded within %.1f seconds.", kPlaybackStartTimeoutSeconds);
                        result = 1;
                        break;
                    }
                    if (player.getState() == VideoPlayerState::kStopped)
                    {
                        LogInfo("[VideoSandbox] Playback finished naturally.");
                        break;
                    }
                    if (hasMaxPlayDuration && now >= playbackDeadline)
                    {
                        LogInfo("[VideoSandbox] Playback duration limit reached (frames=%d, time=%.2fs).",
                                lastFramesCount, player.getTime());
                        break;
                    }
                }

                player.stop();
                device->waitForGPU();
            }
        }

        Video::dispose();
        Audio::dispose();
        GFX_DEL_RESOURCE(sampler);
        GFX_DEL_RESOURCE(pipeline);
        GFX_DEL_RESOURCE(shader);
        WindowSystem::destroyWindow(window);
        GfxManager::disposeDevice();
        WindowSystem::dispose();

        LogInfo("[VideoSandbox] Done.");
        return result;
    }
}
