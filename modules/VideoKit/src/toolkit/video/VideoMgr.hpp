/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/video/common.hpp>

namespace toolkit
{
    // The video service used for video media playback.
    // Owns the lifecycle of the active video backend and exposes player creation.
    class TK_API Video
    {
    public:
        // Initializes the video runtime and backend.
        // Returns true on success, false on failure.
        static bool init();
        // Updates the runtime and backend. Should be called once per frame by the host.
        // Auto-computes the delta time internally from a monotonic clock.
        static void update();
        // Updates the runtime and backend with a host-supplied frame delta (seconds).
        static void update(f32 deltaTime);
        // Resets the automatic wall-clock delta after a host-side presentation pause.
        static void resetUpdateClock();
        // Releases the runtime and backend resources.
        static void dispose();
        // True after init() has succeeded.
        static bool isInitialized();

    public:
        // Asks the active backend to create a new player for the given info.
        // Returns false on success, true on failure (mirrors the original API contract).
        static bool createPlayer(const VideoBackendPlayerInfo &info, VideoBackendPlayer &player);
    };

}
