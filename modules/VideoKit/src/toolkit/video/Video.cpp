/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VideoMgr.hpp"

#include "VideoBackend.hpp"
#include "toolkit/debug/Logger.hpp"

#include <chrono>

namespace toolkit
{
    static bool g_initialized = false;
    static bool g_hasUpdateTimestamp = false;
    static std::chrono::steady_clock::time_point g_lastUpdateTimestamp;

    bool Video::init()
    {
        if (g_initialized)
            return true;

        LogInfo("Video initialization... (backend: %s)", VideoSystem::getName());
        if (!VideoSystem::init())
        {
            LogWarn("Video: failed to initialize backend");
            return false;
        }

        g_initialized = true;
        return true;
    }

    void Video::update()
    {
        if (!g_initialized)
            return;

        const auto now = std::chrono::steady_clock::now();
        f32 deltaTime = 0.0f;
        if (g_hasUpdateTimestamp)
        {
            const auto delta = now - g_lastUpdateTimestamp;
            deltaTime = std::chrono::duration_cast<std::chrono::duration<f32>>(delta).count();
        }
        g_lastUpdateTimestamp = now;
        g_hasUpdateTimestamp = true;

        VideoSystem::update(deltaTime);
    }

    void Video::update(f32 deltaTime)
    {
        if (!g_initialized)
            return;
        VideoSystem::update(deltaTime);
    }

    void Video::resetUpdateClock()
    {
        if (!g_initialized)
            return;

        g_lastUpdateTimestamp = std::chrono::steady_clock::now();
        g_hasUpdateTimestamp = true;
    }

    void Video::dispose()
    {
        if (!g_initialized)
            return;
        VideoSystem::dispose();
        g_hasUpdateTimestamp = false;
        g_initialized = false;
    }

    bool Video::isInitialized()
    {
        return g_initialized;
    }

    bool Video::createPlayer(const VideoBackendPlayerInfo &info, VideoBackendPlayer &player)
    {
        if (!g_initialized)
        {
            LogWarn("Video: cannot create player before init()");
            return true;
        }

        if (!VNativePlayer::create(info, player))
        {
            LogError("VideoKit: failed to setup video playback backend for the given url");
            return true;
        }

        player.isInitialized = true;
        player.created(info);
        return false;
    }
}
