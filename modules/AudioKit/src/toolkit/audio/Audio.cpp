/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Audio.hpp"
#include "AudioSystem.hpp"
#include "AudioListener.hpp"
#include "AudioSource.hpp"

#include "toolkit/collection/Array.hpp"
#include "toolkit/base/Function.hpp"
#include <chrono>

namespace toolkit
{
    TkArray<AudioListener *> Audio::Listeners;
    TkArray<AudioSource *> Audio::Sources;
    TkArray<AudioDevice> Audio::Devices;
    TkAction<> Audio::onDevicesChanged;
    TkAction<> Audio::onActiveDeviceChanged;

    f32 g_masterVolume = 1.0f;
    f32 g_volume = 1.0f;
    i32 g_activeDeviceIdx = -1;
    bool g_enableHRTF = true;
    bool g_initialized = false;
    bool g_hasFocus = true;
    bool g_paused = false;
    bool g_hasUpdateTimestamp = false;
    std::chrono::steady_clock::time_point g_lastUpdateTimestamp;

    TK_FORCE_INLINE f32 calcTargetVolume()
    {
        return (g_paused || (AudioSystem::s_muteOnFocusLoss && !g_hasFocus)) ? 0.0f : g_masterVolume;
    }

    TK_FORCE_INLINE void applyRuntimeVolume()
    {
        if (!g_initialized)
            return;

        const f32 targetVolume = calcTargetVolume();
        if (TkMath::notNearEqual(g_volume, targetVolume))
        {
            g_volume = targetVolume;
            AudioSystem::setVolume(g_volume);
        }
    }

    bool Audio::init()
    {
        if (g_initialized)
            return true;

        g_enableHRTF = AudioSystem::s_enableHRTF;
        const bool result = AudioSystem::init();
        if (!result)
            return false;

        setDopplerFactor(AudioSystem::s_dopplerFactor);
        setEnableHRTF(g_enableHRTF);
        g_volume = g_masterVolume;
        AudioSystem::setVolume(g_volume);
        g_initialized = true;
        onDevicesChanged();

        // Sync already active listeners that were created before audio backend init.
        if (!Listeners.isEmpty())
        {
            ANativeListener::reset();
            for (i32 i = 0; i < Listeners.count(); ++i)
            {
                AudioListener *listener = Listeners[i];
                if (listener != nullptr && listener->isActive())
                {
                    ANativeListener::transformChanged(listener->getPosition(), listener->getOrientation());
                    ANativeListener::velocityChanged(listener->getVelocity());
                }
            }
        }
        return true;
    }

    void Audio::update()
    {
        const auto now = std::chrono::steady_clock::now();
        f32 deltaTime = 0.0f;
        if (g_hasUpdateTimestamp)
        {
            const auto delta = now - g_lastUpdateTimestamp;
            deltaTime = std::chrono::duration_cast<std::chrono::duration<f32>>(delta).count();
        }
        g_lastUpdateTimestamp = now;
        g_hasUpdateTimestamp = true;

        update(deltaTime);
    }

    void Audio::update(f32 deltaTime)
    {
        if (!g_initialized)
            return;

        if (deltaTime > TkMath::ZeroTolerance)
        {
            for (i32 i = 0; i < Listeners.count(); ++i)
            {
                AudioListener *listener = Listeners[i];
                if (listener != nullptr && listener->isActive())
                    listener->update(deltaTime);
            }

            TkArray<AudioSource *> sources;
            sources.ensureCapacity(Sources.count());
            for (i32 i = 0; i < Sources.count(); ++i)
                sources.add(Sources[i]);

            for (i32 i = 0; i < sources.count(); ++i)
            {
                AudioSource *source = sources[i];
                if (source != nullptr)
                    source->updateVelocityFromPositionDelta(deltaTime);
            }
        }

        TkArray<AudioSource *> sources;
        sources.ensureCapacity(Sources.count());
        for (i32 i = 0; i < Sources.count(); ++i)
            sources.add(Sources[i]);

        for (i32 i = 0; i < sources.count(); ++i)
        {
            AudioSource *source = sources[i];
            if (source != nullptr)
                source->update();
        }

        // Keep runtime volume in sync with master/pause/focus-loss policy.
        applyRuntimeVolume();

        AudioSystem::update();
    }

    void Audio::dispose()
    {
        if (!g_initialized)
            return;

        TK_ASSERT(Sources.isEmpty() && Listeners.isEmpty());
        Devices.resize(0);
        g_activeDeviceIdx = -1;
        g_volume = 1.0f;
        g_masterVolume = 1.0f;
        g_hasFocus = true;
        g_paused = false;
        g_hasUpdateTimestamp = false;
        onDevicesChanged();
        AudioSystem::dispose();
        g_initialized = false;
    }

    bool Audio::isInitialized()
    {
        return g_initialized;
    }

    void Audio::setAppHasFocus(bool hasFocus)
    {
        g_hasFocus = hasFocus;
        applyRuntimeVolume();
    }

    void Audio::setPaused(bool paused)
    {
        g_paused = paused;
        applyRuntimeVolume();
    }

    AudioDevice *Audio::getActiveDevice()
    {
        if (g_activeDeviceIdx < 0 || g_activeDeviceIdx >= Devices.count())
            return nullptr;
        return &Devices[g_activeDeviceIdx];
    }

    i32 Audio::getActiveDeviceIdx()
    {
        return g_activeDeviceIdx;
    }

    void Audio::setActiveDeviceIdx(i32 index)
    {
        const i32 maxIndex = Devices.count() - 1;
        index = TkMath::clamp(index, -1, maxIndex);
        if (g_activeDeviceIdx == index)
            return;

        g_activeDeviceIdx = index;
        AudioSystem::onActiveDeviceChanged();
        onActiveDeviceChanged();
    }

    float Audio::getMasterVolume()
    {
        return g_masterVolume;
    }

    void Audio::setMasterVolume(float value)
    {
        g_masterVolume = TkMath::saturate(value);
        applyRuntimeVolume();
    }

    float Audio::getVolume()
    {
        return g_volume;
    }

    bool Audio::getMuteOnFocusLoss()
    {
        return AudioSystem::s_muteOnFocusLoss;
    }

    void Audio::setMuteOnFocusLoss(bool value)
    {
        AudioSystem::s_muteOnFocusLoss = value;
        applyRuntimeVolume();
    }

    void Audio::setDopplerFactor(float value)
    {
        AudioSystem::s_dopplerFactor = TkMath::max(0.0f, value);
        AudioSystem::setDopplerFactor(AudioSystem::s_dopplerFactor);
    }

    bool Audio::getEnableHRTF()
    {
        return g_enableHRTF;
    }

    void Audio::setEnableHRTF(bool value)
    {
        if (g_enableHRTF == value)
            return;

        g_enableHRTF = value;
        AudioSystem::s_enableHRTF = value;
        if (g_initialized)
            ANativeListener::reinitAll();
    }
}
