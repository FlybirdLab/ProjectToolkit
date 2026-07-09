/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VideoPlayer.hpp"

#include "VideoBackend.hpp"
#include "VideoMgr.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/math/Vector2.hpp"

namespace toolkit
{
    VideoPlayer::~VideoPlayer()
    {
        stop();
        if (m_player.isInitialized)
            VNativePlayer::destroy(m_player);
    }

    void VideoPlayer::setPosition(const Vec3 &position)
    {
        m_position = position;
        updateInfo();
    }

    void VideoPlayer::setOrientation(const Quat &orientation)
    {
        m_orientation = orientation;
        updateInfo();
    }

    void VideoPlayer::setTransform(const Vec3 &position, const Quat &orientation)
    {
        m_position = position;
        m_orientation = orientation;
        updateInfo();
    }

    void VideoPlayer::setIsLooping(bool value)
    {
        if (m_loop == value)
            return;
        m_loop = value;
        updateInfo();
    }

    void VideoPlayer::setIsAudioSpatial(bool value)
    {
        if (m_isSpatial == value)
            return;
        m_isSpatial = value;
        updateInfo();
    }

    void VideoPlayer::setAudioVolume(f32 value)
    {
        value = TkMath::saturate(value);
        if (TkMath::nearEqual(m_volume, value))
            return;
        m_volume = value;
        updateInfo();
    }

    void VideoPlayer::setAudioPan(f32 value)
    {
        value = TkMath::clamp(value, -1.0f, 1.0f);
        if (TkMath::nearEqual(m_pan, value))
            return;
        m_pan = value;
        updateInfo();
    }

    void VideoPlayer::setAudioMinDistance(f32 value)
    {
        value = TkMath::max(0.0f, value);
        if (TkMath::nearEqual(m_minDistance, value))
            return;
        m_minDistance = value;
        updateInfo();
    }

    void VideoPlayer::setAudioAttenuation(f32 value)
    {
        value = TkMath::max(0.0f, value);
        if (TkMath::nearEqual(m_attenuation, value))
            return;
        m_attenuation = value;
        updateInfo();
    }

    void VideoPlayer::play()
    {
        if (m_state == VideoPlayerState::kPlaying)
            return;

        if (!m_player.isInitialized)
        {
            if (url.isEmpty())
            {
                LogWarn("VideoPlayer: cannot play without a url");
                return;
            }

            VideoBackendPlayerInfo info;
            getInfo(info);
            if (Video::createPlayer(info, m_player))
                return;

            m_player.initVideoFrame();
        }

        VNativePlayer::play(m_player);
        m_state = VideoPlayerState::kPlaying;
    }

    void VideoPlayer::pause()
    {
        if (m_state != VideoPlayerState::kPlaying)
            return;
        m_state = VideoPlayerState::kPaused;
        if (m_player.isInitialized)
            VNativePlayer::pause(m_player);
    }

    void VideoPlayer::stop()
    {
        if (m_state == VideoPlayerState::kStopped)
            return;
        m_state = VideoPlayerState::kStopped;
        if (m_player.isInitialized)
            VNativePlayer::stop(m_player);
    }

    f32 VideoPlayer::getTime() const
    {
        if (m_state == VideoPlayerState::kStopped || !m_player.isInitialized)
            return 0.0f;
        return VNativePlayer::getTime(m_player).getTotalSeconds();
    }

    void VideoPlayer::setTime(f32 time)
    {
        if (m_state == VideoPlayerState::kStopped || !m_player.isInitialized)
            return;
        TkTimeSpan ts = TkTimeSpan::fromSeconds(time);
        ts.ticks = TkMath::clamp<i64>(ts.ticks, 0, m_player.duration.ticks);
        VNativePlayer::seek(m_player, ts);
    }

    f32 VideoPlayer::getDuration() const
    {
        return m_player.duration.getTotalSeconds();
    }

    f32 VideoPlayer::getFrameRate() const
    {
        return m_player.frameRate;
    }

    i32 VideoPlayer::getFramesCount() const
    {
        return m_player.framesCount;
    }

    IVec2 VideoPlayer::getSize() const
    {
        return IVec2 {m_player.width, m_player.height};
    }

    GfxTexture *VideoPlayer::getFrame() const
    {
        return m_player.frame;
    }

    void VideoPlayer::tick()
    {
        m_player.tick();
        if (m_state == VideoPlayerState::kPlaying && m_player.playbackEnded)
            m_state = VideoPlayerState::kStopped;
    }

    void VideoPlayer::getInfo(VideoBackendPlayerInfo &info) const
    {
        info.url = TkStringView {url};
        info.loop = m_loop;
        info.spatial = m_isSpatial;
        info.volume = m_volume;
        info.pan = m_pan;
        info.minDistance = m_minDistance;
        info.attenuation = m_attenuation;
        info.hasTransform = true;
        info.position = m_position;
        info.orientation = m_orientation;
    }

    void VideoPlayer::updateInfo()
    {
        if (!m_player.isInitialized)
            return;
        VideoBackendPlayerInfo info;
        getInfo(info);
        VNativePlayer::updateInfo(m_player, info);
    }
}
