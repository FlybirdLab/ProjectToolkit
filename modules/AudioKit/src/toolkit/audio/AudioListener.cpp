/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AudioListener.hpp"

#include "Audio.hpp"
#include "AudioSystem.hpp"
#include <toolkit/debug/Logger.hpp>
#include "toolkit/collection/Array.hpp"

namespace toolkit
{
    AudioListener::AudioListener()
    {
    }

    AudioListener::~AudioListener()
    {
        setActive(false);
    }

    const Vec3 &AudioListener::getVelocity() const
    {
        return m_velocity;
    }

    const Vec3 &AudioListener::getPosition() const
    {
        return m_position;
    }

    void AudioListener::setPosition(const Vec3 &position)
    {
        if (m_position == position)
            return;

        m_position = position;
        onTransformChanged();
    }

    const Quat &AudioListener::getOrientation() const
    {
        return m_orientation;
    }

    void AudioListener::setOrientation(const Quat &orientation)
    {
        if (m_orientation == orientation)
            return;

        m_orientation = orientation;
        onTransformChanged();
    }

    void AudioListener::setTransform(const Vec3 &position, const Quat &orientation)
    {
        const bool transformChanged = (m_position != position) || (m_orientation != orientation);
        if (!transformChanged)
            return;

        m_position = position;
        m_orientation = orientation;
        onTransformChanged();
    }

    void AudioListener::update(f32 deltaTime)
    {
        updateVelocityFromPositionDelta(deltaTime);
    }

    void AudioListener::updateVelocityFromPositionDelta(f32 deltaTime)
    {
        if (deltaTime <= TkMath::ZeroTolerance)
            return;

        const Vec3 previousVelocity = m_velocity;
        m_velocity = (m_position - m_previousPos) / deltaTime;
        m_previousPos = m_position;

        if (m_active && m_velocity != previousVelocity)
            ANativeListener::velocityChanged(m_velocity);
    }

    void AudioListener::resetMotionTracking()
    {
        m_previousPos = m_position;
        m_velocity = Vec3::kZero;
        if (m_active)
            ANativeListener::velocityChanged(m_velocity);
    }

    bool AudioListener::isActive() const
    {
        return m_active;
    }

    bool AudioListener::setActive(bool active)
    {
        if (m_active == active)
            return true;

        if (active)
        {
            if (Audio::Listeners.count() >= AUDIO_MAX_LISTENERS)
            {
                LogError("Unsupported amount of the audio listeners!");
                return false;
            }

            TK_ASSERT(!Audio::Listeners.contains(this));
            if (Audio::Listeners.count() > 0)
                LogWarn("There is more than one Audio Listener active. Please make sure only exactly one is active at any given time.");

            Audio::Listeners.add(this);
            m_active = true;
            m_previousPos = m_position;
            m_velocity = Vec3::kZero;

            if (Audio::isInitialized())
            {
                ANativeListener::reset();
                ANativeListener::transformChanged(m_position, m_orientation);
                ANativeListener::velocityChanged(m_velocity);
            }
            return true;
        }

        m_active = false;
        const bool removed = Audio::Listeners.remove(this);
        if (removed && Audio::isInitialized())
            ANativeListener::reset();
        return true;
    }

    void AudioListener::onTransformChanged()
    {
        if (m_active && Audio::isInitialized())
            ANativeListener::transformChanged(m_position, m_orientation);
    }
}
