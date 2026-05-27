/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/IAudioClip.hpp>
#include <toolkit/audio/AudioSystem.hpp>
#include <toolkit/base/Math.hpp>
#include <toolkit/math/Quat.hpp>
#include <toolkit/math/Vector3.hpp>

namespace toolkit
{
    class TK_API AudioSource final : public NonCopyable
    {
    public:
        enum class State
        {
            kPlaying = 0,
            kPaused = 1,
            kStopped = 2
        };

    public:
        AudioSource();
        ~AudioSource();

        IAudioClip *getClip() const;
        void setClip(IAudioClip *clip);

        const Vec3 &getPosition() const;
        void setPosition(const Vec3 &position);
        const Quat &getOrientation() const;
        void setOrientation(const Quat &orientation);
        const Vec3 &getVelocity() const;
        void setVelocity(const Vec3 &velocity);

        f32 getVolume() const;
        void setVolume(f32 value);
        f32 getPitch() const;
        void setPitch(f32 value);
        f32 getPan() const;
        void setPan(f32 value);
        bool getIsLooping() const;
        void setIsLooping(bool value);
        bool getPlayOnStart() const;
        void setPlayOnStart(bool value);
        f32 getStartTime() const;
        void setStartTime(f32 value);
        f32 getMinDistance() const;
        void setMinDistance(f32 value);
        f32 getAttenuation() const;
        void setAttenuation(f32 value);
        f32 getDopplerFactor() const;
        void setDopplerFactor(f32 value);
        bool getAllowSpatialization() const;
        void setAllowSpatialization(bool value);

        void play();
        void pause();
        void stop();

        State getState() const;
        f32 getTime() const;
        void setTime(f32 time);
        bool is3D() const;
        bool useStreaming() const;
        bool isActuallyPlaying() const;

        void requestStreamingBuffersUpdate();
        void update();
        void updateVelocityFromPositionDelta(f32 deltaTime);
        void resetMotionTracking();

        u32 getSourceId() const;
        // Internal backend hook used when native devices/contexts are rebuilt.
        // Regular gameplay code must not mutate native source ids directly.
        void setSourceId(u32 id) {
            m_sourceId = id;
        }

    private:
        bool ensureClipLoaded();
        bool ensureSourceCreated();
        void syncNativeSpatialState();
        void requestClipStreamingUpdate();
        bool syncStreamingBuffers();
        void playInternal();
        void destroyNativeSource();

    private:
        IAudioClip *m_clip = nullptr;
        Vec3 m_position = Vec3::kZero;
        Vec3 m_velocity = Vec3::kZero;
        Vec3 m_previousPosition = Vec3::kZero;
        Quat m_orientation = Quat::kIdentity;
        f32 m_volume = 1.0f;
        f32 m_pitch = 1.0f;
        f32 m_pan = 0.0f;
        f32 m_minDistance = 1000.0f;
        f32 m_attenuation = 1.0f;
        f32 m_dopplerFactor = 1.0f;
        f32 m_startTime = 0.0f;
        bool m_loop = false;
        bool m_playOnStart = false;
        bool m_allowSpatialization = true;
        bool m_isActuallyPlaying = false;
        bool m_startingPlayback = false;
        bool m_pendingStreamingPlay = false;
        bool m_needToUpdateStreamingBuffers = false;
        State m_state = State::kStopped;
        i32 m_streamingFirstChunk = 0;
        u32 m_sourceId = 0;
    };
}
