/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"
#include "toolkit/collection/DataCollection.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/math/Quat.hpp"
#include "toolkit/math/Vector3.hpp"
#include "toolkit/types/Span.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/types/TimeSpan.hpp"

namespace toolkit
{
    // Description for new video player creation by backend.
    struct VideoBackendPlayerInfo
    {
        TkStringView url;
        bool loop = false;
        bool spatial = false;
        f32 volume = 1.0f;
        f32 pan = 0.0f;
        f32 minDistance = 1000.0f;
        f32 attenuation = 1.0f;
        bool hasTransform = false;
        Vec3 position = Vec3::kZero;
        Quat orientation = Quat::kIdentity;
    };

    // Per-player runtime state managed by the active video backend.
    // The backend reads/writes m_backendData to attach its private payload.
    struct TK_API VideoBackendPlayer
    {
    public:
        // True if the active backend has claimed this player.
        bool isInitialized = false;
        // Backend-private payload (cast inside the backend implementation).
        void *backendData = nullptr;

        // Video properties (filled by backend on open).
        i32 width = 0;
        i32 height = 0;
        TkTimeSpan duration;
        f32 frameRate = 0.0f;
        i32 framesCount = 0;
        PixelFormat format = PixelFormat::kUnknown;
        bool playbackEnded = false;

        // Video frame upload state.
        i32 videoFrameWidth = 0;
        i32 videoFrameHeight = 0;
        TkTimeSpan videoFrameTime;
        TkTimeSpan videoFrameDuration;
        TkDataCollection<byte> videoFrameMemory;
        GfxTexture *frame = nullptr;
        GfxBuffer *frameUpload = nullptr;
        class VFrameUploadTask *uploadVideoFrameTask = nullptr;

        // Audio playback state (routed through AudioKit's native source/buffer API).
        AudioDataInfo audioInfo {};
        TkTimeSpan audioBufferTime;
        TkTimeSpan audioBufferDuration;
        u32 audioSource = 0;
        u32 audioBuffers[AUDIO_STREAM_RING_SIZE] = {};
        i32 nextAudioBuffer = 0;
        bool isAudioSpatial = false;
        u8 isAudioPlayPending = 0;
        bool hasSubmittedAudioBuffer = false;
        f32 audioVolume = 1.0f;
        f32 audioPan = 0.0f;
        f32 audioMinDistance = 1000.0f;
        f32 audioAttenuation = 1.0f;
        bool hasTransform = false;
        Vec3 position = Vec3::kZero;
        Quat orientation = Quat::kIdentity;

    public:
        void created(const VideoBackendPlayerInfo &info);
        void updated(const VideoBackendPlayerInfo &info);

        void playAudio();
        void pauseAudio();
        void stopAudio();

        void initVideoFrame();
        void updateVideoFrame(TkSpan<byte> data, TkTimeSpan time, TkTimeSpan duration, i32 dataRowPitch = 0);
        i32 getFreeAudioBufferCount();
        bool updateAudioBuffer(TkSpan<byte> data, TkTimeSpan time, TkTimeSpan duration);

        void tick();
        void releaseResources();
    };

    // Per-player operations exposed by the active video backend.
    // Implementations live in the platform-specific source files.
    class TK_API VNativePlayer
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

    // Video backend lifecycle and global configuration.
    class TK_API VideoSystem
    {
    public:
        // If true, video playback will be disabled in the runtime.
        inline static bool s_disableVideo = false;

    public:
        static const char *getName();
        static bool init();
        // Updates the backend with a host-supplied frame delta (seconds).
        static void update(f32 deltaTime);
        static void dispose();
    };

}
