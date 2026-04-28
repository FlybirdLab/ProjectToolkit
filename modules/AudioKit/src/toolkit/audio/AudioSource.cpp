/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AudioSource.hpp"

#include "Audio.hpp"
#include <toolkit/debug/Logger.hpp>

namespace toolkit
{
    static constexpr i32 kDefaultStreamingResidency = 2;

    AudioSource::AudioSource()
    {
        Audio::Sources.add(this);
    }

    AudioSource::~AudioSource()
    {
        stop();
        destroyNativeSource();
        Audio::Sources.remove(this);
    }

    AudioClip *AudioSource::getClip() const
    {
        return m_clip;
    }

    void AudioSource::setClip(AudioClip *clip)
    {
        if (m_clip == clip)
            return;

        const State previousState = m_state;
        const f32 previousTime = getTime();

        stop();
        destroyNativeSource();
        m_clip = clip;
        m_streamingFirstChunk = 0;
        m_needToUpdateStreamingBuffers = false;

        if (m_clip != nullptr && previousState != State::kStopped)
        {
            setTime(previousTime);
            play();
            if (previousState == State::kPaused)
                pause();
        }
    }

    const Vec3 &AudioSource::getPosition() const
    {
        return m_position;
    }

    void AudioSource::setPosition(const Vec3 &position)
    {
        m_position = position;
        if (m_sourceId != 0 && is3D())
            ANativeSource::transformChanged(m_sourceId, m_position, m_orientation);
    }

    const Quat &AudioSource::getOrientation() const
    {
        return m_orientation;
    }

    void AudioSource::setOrientation(const Quat &orientation)
    {
        m_orientation = orientation;
        if (m_sourceId != 0 && is3D())
            ANativeSource::transformChanged(m_sourceId, m_position, m_orientation);
    }

    const Vec3 &AudioSource::getVelocity() const
    {
        return m_velocity;
    }

    void AudioSource::setVelocity(const Vec3 &velocity)
    {
        m_velocity = velocity;
        if (m_sourceId != 0 && is3D())
            ANativeSource::velocityChanged(m_sourceId, m_velocity);
    }

    f32 AudioSource::getVolume() const
    {
        return m_volume;
    }

    void AudioSource::setVolume(f32 value)
    {
        value = TkMath::saturate(value);
        if (TkMath::nearEqual(m_volume, value))
            return;

        m_volume = value;
        if (m_sourceId != 0)
            ANativeSource::volumeChanged(m_sourceId, m_volume);
    }

    f32 AudioSource::getPitch() const
    {
        return m_pitch;
    }

    void AudioSource::setPitch(f32 value)
    {
        value = TkMath::clamp(value, 0.5f, 2.0f);
        if (TkMath::nearEqual(m_pitch, value))
            return;

        m_pitch = value;
        if (m_sourceId != 0)
            ANativeSource::pitchChanged(m_sourceId, m_pitch);
    }

    f32 AudioSource::getPan() const
    {
        return m_pan;
    }

    void AudioSource::setPan(f32 value)
    {
        value = TkMath::clamp(value, -1.0f, 1.0f);
        if (TkMath::nearEqual(m_pan, value))
            return;

        m_pan = value;
        if (m_sourceId != 0)
            ANativeSource::panChanged(m_sourceId, m_pan);
    }

    bool AudioSource::getIsLooping() const
    {
        return m_loop;
    }

    void AudioSource::setIsLooping(bool value)
    {
        if (m_loop == value)
            return;

        m_loop = value;
        if (m_sourceId != 0 && !useStreaming())
            ANativeSource::isLoopingChanged(m_sourceId, m_loop);
    }

    bool AudioSource::getPlayOnStart() const
    {
        return m_playOnStart;
    }

    void AudioSource::setPlayOnStart(bool value)
    {
        m_playOnStart = value;
    }

    f32 AudioSource::getStartTime() const
    {
        return m_startTime;
    }

    void AudioSource::setStartTime(f32 value)
    {
        m_startTime = TkMath::max(value, 0.0f);
    }

    f32 AudioSource::getMinDistance() const
    {
        return m_minDistance;
    }

    void AudioSource::setMinDistance(f32 value)
    {
        value = TkMath::max(0.0f, value);
        if (TkMath::nearEqual(m_minDistance, value))
            return;

        m_minDistance = value;
        syncNativeSpatialState();
    }

    f32 AudioSource::getAttenuation() const
    {
        return m_attenuation;
    }

    void AudioSource::setAttenuation(f32 value)
    {
        value = TkMath::max(0.0f, value);
        if (TkMath::nearEqual(m_attenuation, value))
            return;

        m_attenuation = value;
        syncNativeSpatialState();
    }

    f32 AudioSource::getDopplerFactor() const
    {
        return m_dopplerFactor;
    }

    void AudioSource::setDopplerFactor(f32 value)
    {
        value = TkMath::max(0.0f, value);
        if (TkMath::nearEqual(m_dopplerFactor, value))
            return;

        m_dopplerFactor = value;
        syncNativeSpatialState();
    }

    bool AudioSource::getAllowSpatialization() const
    {
        return m_allowSpatialization;
    }

    void AudioSource::setAllowSpatialization(bool value)
    {
        if (m_allowSpatialization == value)
            return;

        m_allowSpatialization = value;
        syncNativeSpatialState();
    }

    void AudioSource::play()
    {
        if (m_state == State::kPlaying)
            return;

        if (!ensureClipLoaded() || !ensureSourceCreated())
            return;

        const State previousState = m_state;

        if (useStreaming())
        {
            if (previousState == State::kStopped)
                setTime(m_startTime);

            requestClipStreamingUpdate();
            requestStreamingBuffersUpdate();
            m_isActuallyPlaying = false;
            m_pendingStreamingPlay = true;
            syncStreamingBuffers();
        }
        else
        {
            const auto &buffers = m_clip->getBuffers();
            if (buffers.count() <= 0 || buffers[0] == 0)
            {
                LogWarn("AudioSource cannot play clip without prepared buffers.");
                return;
            }

            ANativeSource::setNonStreamingBuffer(m_sourceId, buffers[0]);
            if (previousState == State::kStopped && m_startTime > 0.0f)
                ANativeSource::setCurrentBufferTime(m_sourceId, m_startTime);
            m_state = State::kPlaying;
            m_isActuallyPlaying = false;
            playInternal();
        }
    }

    void AudioSource::pause()
    {
        if (m_state != State::kPlaying)
            return;

        m_state = State::kPaused;
        if (m_isActuallyPlaying && m_sourceId != 0)
        {
            ANativeSource::pause(m_sourceId);
            m_isActuallyPlaying = false;
        }
    }

    void AudioSource::stop()
    {
        m_state = State::kStopped;
        m_isActuallyPlaying = false;
        m_startingPlayback = false;
        m_pendingStreamingPlay = false;
        m_streamingFirstChunk = 0;
        m_needToUpdateStreamingBuffers = false;
        if (m_sourceId != 0)
        {
            ANativeSource::stop(m_sourceId);
            ANativeSource::clearQueuedBuffers(m_sourceId);
        }
    }

    AudioSource::State AudioSource::getState() const
    {
        return m_state;
    }

    f32 AudioSource::getTime() const
    {
        if (m_clip == nullptr || m_sourceId == 0)
            return 0.0f;

        f32 time = ANativeSource::getCurrentBufferTime(m_sourceId);
        if (useStreaming())
        {
            i32 processedBuffers = 0;
            ANativeSource::getProcessedBuffersCount(m_sourceId, processedBuffers);
            const i32 maxIndex = TkMath::clamp(m_clip->getTotalChunks(), 0, AUDIO_CLIP_MAX_CHUNKS);
            const i32 index = TkMath::clamp(m_streamingFirstChunk + processedBuffers, 0, maxIndex);
            time += m_clip->getBufferStartTime(index);
        }

        return TkMath::clamp(time, 0.0f, m_clip->getLength());
    }

    void AudioSource::setTime(f32 time)
    {
        if (m_clip == nullptr)
            return;

        time = TkMath::clamp(time, 0.0f, m_clip->getLength());
        m_startTime = time;

        if (!ensureClipLoaded() || !ensureSourceCreated())
            return;

        if (useStreaming())
        {
            f32 relativeTime = 0.0f;
            const bool wasPlaying = m_state == State::kPlaying;
            m_streamingFirstChunk = m_clip->getFirstBufferIndex(time, relativeTime);
            ANativeSource::stop(m_sourceId);
            ANativeSource::clearQueuedBuffers(m_sourceId);
            ANativeSource::setCurrentBufferTime(m_sourceId, relativeTime);
            m_isActuallyPlaying = false;
            m_startingPlayback = false;
            m_pendingStreamingPlay = false;
            if (wasPlaying)
            {
                requestClipStreamingUpdate();
                requestStreamingBuffersUpdate();
                syncStreamingBuffers();
            }
        }
        else
        {
            ANativeSource::setCurrentBufferTime(m_sourceId, time);
        }
    }

    bool AudioSource::is3D() const
    {
        return m_allowSpatialization && m_clip != nullptr && m_clip->is3D();
    }

    bool AudioSource::useStreaming() const
    {
        return m_clip != nullptr && m_clip->isStreamable();
    }

    bool AudioSource::isActuallyPlaying() const
    {
        return m_isActuallyPlaying;
    }

    void AudioSource::requestStreamingBuffersUpdate()
    {
        m_needToUpdateStreamingBuffers = true;
    }

    void AudioSource::update()
    {
        if (m_startingPlayback && getTime() > 0.0f)
            m_startingPlayback = false;

        if (useStreaming())
        {
            syncStreamingBuffers();

            if (m_sourceId == 0 || !m_isActuallyPlaying)
                return;

            i32 processedBuffers = 0;
            ANativeSource::getProcessedBuffersCount(m_sourceId, processedBuffers);
            if (processedBuffers <= 0)
                return;

            ANativeSource::dequeueProcessedBuffers(m_sourceId);
            m_streamingFirstChunk += processedBuffers;

            const i32 totalBuffers = m_clip->getBuffers().count();
            if (m_streamingFirstChunk >= totalBuffers)
            {
                if (m_loop)
                {
                    m_streamingFirstChunk = 0;
                    requestClipStreamingUpdate();
                    requestStreamingBuffersUpdate();
                    syncStreamingBuffers();
                }
                else
                {
                    stop();
                }
                return;
            }

            requestClipStreamingUpdate();
            requestStreamingBuffersUpdate();
            syncStreamingBuffers();
        }
    }

    void AudioSource::updateVelocityFromPositionDelta(f32 deltaTime)
    {
        if (deltaTime <= TkMath::ZeroTolerance)
            return;

        setVelocity((m_position - m_previousPosition) / deltaTime);
        m_previousPosition = m_position;
    }

    void AudioSource::resetMotionTracking()
    {
        m_previousPosition = m_position;
        setVelocity(Vec3::kZero);
    }

    u32 AudioSource::getSourceId() const
    {
        return m_sourceId;
    }

    bool AudioSource::ensureClipLoaded()
    {
        if (m_clip == nullptr)
        {
            LogWarn("AudioSource cannot play without an AudioClip.");
            return false;
        }

        if (m_clip->getBuffers().isEmpty() && m_clip->load())
        {
            LogWarn("AudioSource failed to load AudioClip before playback.");
            return false;
        }

        return true;
    }

    bool AudioSource::ensureSourceCreated()
    {
        if (m_sourceId != 0)
            return true;

        if (m_clip == nullptr)
            return false;

        m_sourceId = ANativeSource::add(
            m_clip->getInfo(),
            m_position,
            m_orientation,
            m_volume,
            m_pitch,
            m_pan,
            m_loop && !useStreaming(),
            is3D(),
            m_attenuation,
            m_minDistance,
            m_dopplerFactor);

        if (m_sourceId == 0)
        {
            LogWarn("AudioSource failed to create native source.");
            return false;
        }

        return true;
    }

    void AudioSource::syncNativeSpatialState()
    {
        if (m_sourceId == 0)
            return;

        ANativeSource::spatialSetupChanged(m_sourceId, is3D(), m_attenuation, m_minDistance, m_dopplerFactor);
        if (is3D())
            ANativeSource::transformChanged(m_sourceId, m_position, m_orientation);
    }

    void AudioSource::requestClipStreamingUpdate()
    {
        if (m_clip == nullptr || !useStreaming() || !m_clip->canBeUpdated())
            return;

        auto &queue = m_clip->getStreamingQueue();
        queue.clear();

        const auto &buffers = m_clip->getBuffers();
        const i32 residency = TkMath::clamp(kDefaultStreamingResidency, 1, AUDIO_CLIP_MAX_CHUNKS);
        for (i32 i = 0; i < residency; ++i)
        {
            const i32 chunkIndex = m_streamingFirstChunk + i;
            if (chunkIndex >= 0 && chunkIndex < buffers.count() && buffers[chunkIndex] == 0)
                queue.add(chunkIndex);
        }

        if (!queue.isEmpty())
        {
            if (TkTask *task = m_clip->createStreamingTask(queue.count()))
                task->start();
        }
    }

    bool AudioSource::syncStreamingBuffers()
    {
        if (!useStreaming() || m_sourceId == 0 || m_clip == nullptr || !m_needToUpdateStreamingBuffers)
            return false;

        const auto &buffers = m_clip->getBuffers();
        if (m_streamingFirstChunk < 0 || m_streamingFirstChunk >= buffers.count())
            return false;

        i32 queuedBuffers = 0;
        ANativeSource::getQueuedBuffersCount(m_sourceId, queuedBuffers);
        bool hasQueuedBuffer = queuedBuffers > 0;

        const i32 residency = TkMath::clamp(kDefaultStreamingResidency, 1, AUDIO_CLIP_MAX_CHUNKS);
        for (i32 i = queuedBuffers; i < residency; ++i)
        {
            const i32 chunkIndex = m_streamingFirstChunk + i;
            if (chunkIndex >= buffers.count())
                break;

            const u32 bufferId = buffers[chunkIndex];
            if (bufferId == 0)
            {
                requestClipStreamingUpdate();
                m_needToUpdateStreamingBuffers = true;
                return false;
            }

            ANativeSource::queueBuffer(m_sourceId, bufferId);
            hasQueuedBuffer = true;
        }

        m_needToUpdateStreamingBuffers = false;
        if (hasQueuedBuffer && !m_isActuallyPlaying && (m_state == State::kPlaying || m_pendingStreamingPlay))
        {
            m_state = State::kPlaying;
            m_pendingStreamingPlay = false;
            playInternal();
        }
        return hasQueuedBuffer;
    }

    void AudioSource::playInternal()
    {
        if (m_sourceId == 0)
            return;

        ANativeSource::play(m_sourceId);
        m_isActuallyPlaying = true;
        m_startingPlayback = true;
    }

    void AudioSource::destroyNativeSource()
    {
        if (m_sourceId != 0)
        {
            ANativeSource::remove(m_sourceId);
            m_sourceId = 0;
        }
    }
}
