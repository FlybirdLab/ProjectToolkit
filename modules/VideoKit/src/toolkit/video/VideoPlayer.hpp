/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "VideoBackend.hpp"
#include "common.hpp"
#include "toolkit/math/Quat.hpp"
#include "toolkit/math/typedef.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{
    // Valid states in which VideoPlayer can be in.
    enum class VideoPlayerState
    {
        // The video is currently stopped (play will resume from start).
        kStopped = 0,
        // The video is currently playing.
        kPlaying = 1,
        // The video is currently paused (play will resume from paused point).
        kPaused = 2,
    };

    // Video playback utility. Video content can be presented in UI (via VideoBrush), used in materials (via texture parameter bind) or used manually in shaders.
    class TK_API VideoPlayer
    {
    private:
        VideoBackendPlayer m_player;
        VideoPlayerState m_state = VideoPlayerState::kStopped;
        Vec3 m_position = Vec3::kZero;
        Quat m_orientation = Quat::kIdentity;
        bool m_loop = false;
        bool m_isSpatial = false;
        f32 m_volume = 1.0f;
        f32 m_pan = 0.0f;
        f32 m_minDistance = 1000.0f;
        f32 m_attenuation = 1.0f;

    public:
        VideoPlayer() = default;
        ~VideoPlayer();

        // The video clip Url path used as a source of the media. Can be local file (absolute or relative path), or streamed resource ('http://').
        TkString url;

        // Determines whether the video clip should autoplay on creation.
        bool playOnStart = false;
        // Determines the time (in seconds) at which the video clip starts playing if Play On Start is enabled.
        f32 startTime = 0.0f;

        // World-space position used by the spatial audio backend.
        TK_FORCE_INLINE const Vec3 &getPosition() const
        {
            return m_position;
        }

        void setPosition(const Vec3 &position);

        // World-space orientation used by the spatial audio backend.
        TK_FORCE_INLINE const Quat &getOrientation() const
        {
            return m_orientation;
        }

        void setOrientation(const Quat &orientation);

        // Sets position and orientation in one call to avoid double backend sync.
        void setTransform(const Vec3 &position, const Quat &orientation);

        // Determines whether the video clip should loop when it finishes playing.
        TK_FORCE_INLINE bool getIsLooping() const
        {
            return m_loop;
        }

        void setIsLooping(bool value);

        // If checked, video player us using spatialization to play 3d audio, otherwise will always play as 2d sound.
        TK_FORCE_INLINE bool getIsAudioSpatial() const
        {
            return m_isSpatial;
        }

        void setIsAudioSpatial(bool value);

        // Get the volume of the audio played from this video, in [0, 1] range.
        TK_FORCE_INLINE f32 getAudioVolume() const
        {
            return m_volume;
        }

        void setAudioVolume(f32 value);

        // Get the stereo pan of the played audio (-1 is left speaker, 1 is right speaker, 0 is balanced). The default is 0. Used by non-spatial audio only.
        TK_FORCE_INLINE f32 getAudioPan() const
        {
            return m_pan;
        }

        void setAudioPan(f32 value);

        // Get the minimum distance at which audio attenuation starts. When the listener is closer to the video player than this value, audio is heard at full volume. Once farther away the audio starts attenuating. Used by spatial audio only.
        TK_FORCE_INLINE f32 getAudioMinDistance() const
        {
            return m_minDistance;
        }

        void setAudioMinDistance(f32 value);

        // Get the attenuation that controls how quickly does audio volume drop off as the listener moves further from the video player. Used by spatial audio only.
        TK_FORCE_INLINE f32 getAudioAttenuation() const
        {
            return m_attenuation;
        }

        void setAudioAttenuation(f32 value);

    public:
        // Starts playing the currently assigned video Url.
        void play();

        // Pauses the video playback.
        void pause();

        // Stops video playback, rewinding it to the start.
        void stop();

        // Get the current state of the video playback (playing/paused/stopped).
        TK_FORCE_INLINE VideoPlayerState getState() const
        {
            return m_state;
        }

        // Get the current time of playback. The time is in seconds, in range [0, Duration].
        f32 getTime() const;

        // Set the current time of playback. The time is in seconds, in range [0, Duration].
        void setTime(f32 time);

        // Get the media duration of playback (in seconds). Valid only when media was loaded by the video backend.
        f32 getDuration() const;

        // Get the media frame rate of playback. Valid only when media was loaded by the video backend.
        f32 getFrameRate() const;

        // Get the amount of video frames decoded and sent to GPU during playback.
        i32 getFramesCount() const;

        // Get the video frame dimensions (in pixels). Valid only when media was loaded by the video backend.
        IVec2 getSize() const;

        // Get the video frame texture (GPU resource). Created on the playback start.
        GfxTexture *getFrame() const;

        // Per-frame tick. Should be called by the host once per frame for spatial audio updates.
        void tick();

    private:
        void getInfo(VideoBackendPlayerInfo &info) const;
        void updateInfo();
    };

}
