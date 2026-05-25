/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/math/Transform.hpp"
#include "toolkit/types/StringView.hpp"
#include <toolkit/video/common.hpp>

namespace toolkit
{
    // Description for new video player creation by backend.
    struct VideoBackendPlayerInfo
    {
        TkStringView url;
        bool loop;
        bool spatial;
        float volume;
        float pan;
        float minDistance;
        float attenuation;
        const Transform *transform;
    };

    class VNativePlayer 
    {
    public:
        static bool create(const VideoBackendPlayerInfo &info, VideoBackendPlayer &player);
        static void destroy(VideoBackendPlayer &player);
        static void updateInfo(VideoBackendPlayer &player, const VideoBackendPlayerInfo &info);
        static void play(VideoBackendPlayer &player);
        static void pause(VideoBackendPlayer &player);
        static void stop(VideoBackendPlayer &player);
        static void seek(VideoBackendPlayer &player, TkTimeSpan time);
        static TkTimeSpan getTime(const VideoBackendPlayer &player);
    };
    
    // The helper class for that handles active Video backend operations.
    class VNativeManager
    {
    public:
        static const char *getName();
        static bool init();
        static void update(class TaskGraph *graph);
        static void dispose();
    };

}