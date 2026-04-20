/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/audio/AudioSystem.hpp>

#if TK_AUDIO_BACKEND_NONE

#include "toolkit/audio/Audio.hpp"
#include "toolkit/collection/Array.hpp"

namespace toolkit
{
    void ANativeListener::reset()
    {
    }
    void ANativeListener::velocityChanged(const Vec3 &velocity)
    {
        (void)velocity;
    }
    void ANativeListener::transformChanged(const Vec3 &pos, const Quat &orientation)
    {
        (void)pos;
        (void)orientation;
    }
    void ANativeListener::reinitAll()
    {
    }

    u32 ANativeSource::add(const AudioDataInfo &format, const Vec3 &pos, const Quat &orientation, f32 volume, f32 pitch, f32 pan, bool loop, bool spatial, f32 attenuation, f32 minDistance, f32 doppler)
    {
        return 1;
    }
    void ANativeSource::remove(u32 sourceID)
    {
    }
    void ANativeSource::velocityChanged(u32 sourceID, const Vec3 &velocity)
    {
    }
    void ANativeSource::transformChanged(u32 sourceID, const Vec3 &pos, const Quat &orientation)
    {
    }
    void ANativeSource::volumeChanged(u32 sourceID, f32 volume)
    {
    }
    void ANativeSource::pitchChanged(u32 sourceID, f32 pitch)
    {
    }
    void ANativeSource::panChanged(u32 sourceID, f32 pan)
    {
    }
    void ANativeSource::isLoopingChanged(u32 sourceID, bool loop)
    {
    }
    void ANativeSource::spatialSetupChanged(u32 sourceID, bool spatial, f32 attenuation, f32 minDistance, f32 doppler)
    {
    }
    void ANativeSource::play(u32 sourceID)
    {
    }
    void ANativeSource::pause(u32 sourceID)
    {
    }
    void ANativeSource::stop(u32 sourceID)
    {
    }
    void ANativeSource::setCurrentBufferTime(u32 sourceID, f32 value)
    {
    }
    f32 ANativeSource::getCurrentBufferTime(u32 sourceID)
    {
        return 0.0f;
    }
    void ANativeSource::setNonStreamingBuffer(u32 sourceID, u32 bufferID)
    {
    }
    void ANativeSource::getProcessedBuffersCount(u32 sourceID, i32 &processedBuffersCount)
    {
    }
    void ANativeSource::getQueuedBuffersCount(u32 sourceID, i32 &queuedBuffersCount)
    {
    }
    void ANativeSource::queueBuffer(u32 sourceID, u32 bufferID)
    {
    }
    void ANativeSource::dequeueProcessedBuffers(u32 sourceID)
    {
    }

    u32 ANativeBuffer::create()
    {
        return 1;
    }
    void ANativeBuffer::destroy(u32 bufferID)
    {
    }
    void ANativeBuffer::write(u32 bufferID, byte *samples, const AudioDataInfo &info)
    {
    }

    const char *AudioSystem::getName()
    {
        return "None";
    }
    AudioFeatureFlags AudioSystem::getFeatures()
    {
        return AudioFeatureFlags::kNone;
    }
    void AudioSystem::onActiveDeviceChanged() {}
    void AudioSystem::setDopplerFactor(f32 value) {}
    void AudioSystem::setVolume(f32 value) {}
    bool AudioSystem::init()
    {

        auto &devices = Audio::Devices;

        // Dummy device
        devices.resize(1);
        devices[0].name = "Dummy device";
        Audio::setActiveDeviceIdx(0);

        return true;
    }
    void AudioSystem::update() {}
    void AudioSystem::dispose() {}
}

#endif