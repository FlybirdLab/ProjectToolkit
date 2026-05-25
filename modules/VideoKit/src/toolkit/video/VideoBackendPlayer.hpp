/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once 

#include "toolkit/collection/DataCollection.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/types/TimeSpan.hpp"
#include <toolkit/video/common.hpp>
#include <toolkit/math/Transform.hpp>

namespace toolkit {
    
    // Video player instance created by backend.
    struct VideoBackendPlayer
    {
    public:
        VideoBackend *backend;
        GfxTexture *frame;
        GfxBuffer *frameUpload;
        class GfxUploadVideoFrameTask *uploadVideoFrameTask;
        const Transform *transform;
#ifdef TK_TRACY_ENABLE
        char* debugUrl;
        i32 debugUrlLen;
#endif
        i32 width, height, framesCount;
        i32 videoFrameWidth, videoFrameHeight;
        PixelFormat format;
        f32 frameRate;
        f32 audioVolume;
        f32 audioPan;
        f32 audioMinDistance;
        f32 audioAttenuation;
        u8 isAudioSpatial : 1;
        u8 isAudioPlayPending : 1;
        TkTimeSpan duration;
        TkTimeSpan videoFrameTime, videoFrameDuration;
        TkTimeSpan audioBufferTime, audioBufferDuration;
        //TODO AudioDataInfo audioInfo;
        ByteCollection videoFrameMemory;
        u32 audioSource;
        u32 nextAudioBuffer;
        u32 audioBuffers[30];
        uintptr backendState[8];

    public:
        VideoBackendPlayer()
        {
            TkMemory::memoryClear(this, sizeof(VideoBackendPlayer));
        }
        POD_COPYABLE(VideoBackendPlayer);

        template <typename T>
        TK_FORCE_INLINE T &getBackendState()
        {
            static_assert(sizeof(T) <= sizeof(backendState), "Increase state data to fit per-backend storage.");
            return *(T *)backendState;
        }

        template <typename T>
        TK_FORCE_INLINE const T &getBackendState() const
        {
            static_assert(sizeof(T) <= sizeof(backendState), "Increase state data to fit per-backend storage.");
            return *(const T *)backendState;
        }

        void created(const VideoBackendPlayerInfo &info);
        void updated(const VideoBackendPlayerInfo &info);
        void playAudio();
        void pauseAudio();
        void stopAudio();
        void initVideoFrame();
        void updateVideoFrame(TkSpan<byte> data, TkTimeSpan time, TkTimeSpan duration);
        void updateAudioBuffer(TkSpan<byte> data, TkTimeSpan time, TkTimeSpan duration);
        void tick();
        void releaseResources();
    };

}