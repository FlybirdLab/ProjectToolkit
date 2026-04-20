/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/math/typedef.hpp"
#include <toolkit/audio/common.hpp>
#include <toolkit/math/Vector3.hpp>
#include <toolkit/math/Quat.hpp>

namespace toolkit
{
    enum class Channels
    {
        kFrontLeft = 0,
        kFrontRight = 1,
        kFontCenter = 2,
        kBackLeft = 3,
        kBackRight = 4,
        kSideLeft = 5,
        kSideRight = 6,
        kMaxChannels
    };

    struct SoundMixParams
    {
        f32 volume = 1.0f;
        f32 dopplerFactor = 1.0f;
    };

    class SoundMix
    {
    public:
        f32 pitch;
        f32 volume;
        f32 channels[(i32)Channels::kMaxChannels];

    public:
        void volumeIntoChannels()
        {
            for (f32 &c : channels)
                c *= volume;
            volume = 1.0f;
        }

    public:
        static SoundMix calSoundMix(const SoundMixParams &p, const class ANativeListener &listener, const class ANativeSource &source, i32 channelCount = 2);
        static void mapChannels(i32 sourceChannels, i32 outputChannels, f32 channels[(i32)Channels::kMaxChannels], f32 *outputMatrix);
    };

    // Supports multi-channel (incl. stereo) audio playback for spatial sources (3D), otherwise 3d audio needs to be in mono format.
    enum class AudioFeatureFlags
    {
        kNone = 0,
        kSpatialMultiChannel = 1,
    };

    class ANativeListener
    {
    public:
        Vec3 velocity;
        Vec3 pos;
        Quat orientation;

    public:
        void resetCurrent()
        {
            velocity = Vec3::kZero;
            pos = Vec3::kZero;
            orientation = Quat::kIdentity;
        }

    public:
        static void reset();
        static void velocityChanged(const Vec3 &velocity);
        static void transformChanged(const Vec3 &pos, const Quat &orientation);
        static void reinitAll();
    };

    class ANativeSource
    {
    public:
        bool is3D;
        f32 volume;
        f32 pitch;
        f32 pan;
        f32 minDistance;
        f32 attenuation;
        f32 dopplerFactor;
        Vec3 velocity;
        Vec3 pos;
        Quat orientation;

    public:
        static u32 add(const AudioDataInfo &format, const Vec3 &pos, const Quat &orientation, f32 volume, f32 pitch, f32 pan, bool loop, bool spatial, f32 attenuation, f32 minDistance, f32 doppler);
        static void remove(u32 sourceID);
        static void velocityChanged(u32 sourceID, const Vec3 &velocity);
        static void transformChanged(u32 sourceID, const Vec3 &pos, const Quat &orientation);
        static void volumeChanged(u32 sourceID, f32 volume);
        static void pitchChanged(u32 sourceID, f32 pitch);
        static void panChanged(u32 sourceID, f32 pan);
        static void isLoopingChanged(u32 sourceID, bool loop);
        static void spatialSetupChanged(u32 sourceID, bool spatial, f32 attenuation, f32 minDistance, f32 doppler);
        static void play(u32 sourceID);
        static void pause(u32 sourceID);
        static void stop(u32 sourceID);
        static void setCurrentBufferTime(u32 sourceID, f32 value);
        static f32 getCurrentBufferTime(u32 sourceID);
        static void setNonStreamingBuffer(u32 sourceID, u32 bufferID);
        static void getProcessedBuffersCount(u32 sourceID, i32 &processedBuffersCount);
        static void getQueuedBuffersCount(u32 sourceID, i32 &queuedBuffersCount);
        static void queueBuffer(u32 sourceID, u32 bufferID);
        static void dequeueProcessedBuffers(u32 sourceID);
    };

    class ANativeBuffer
    {
    public:
        static u32 create();
        static void destroy(u32 bufferID);
        static void write(u32 bufferID, byte *samples, const AudioDataInfo &info);
    };

    class AudioSystem
    {
    public:
        // If checked, audio playback will be disabled in build game. Can be used if game uses custom audio playback engine.
        inline static bool s_disableAudio = false;
        // The doppler effect factor. Scale for source and listener velocities. Default is 1.
        inline static float s_dopplerFactor = 1.0f;
        // True if mute all audio playback when game has no use focus.
        inline static bool s_muteOnFocusLoss = true;
        // Enables or disables HRTF audio for in-engine processing of 3D audio (if supported by platform).
        // If enabled, the user should be using two-channel/headphones audio output and have all other surround virtualization disabled (Atmos, DTS:X, vendor specific, etc.)
        inline static bool s_enableHRTF = false;

    public:
        static const char *getName();
        static AudioFeatureFlags getFeatures();
        static void onActiveDeviceChanged();
        static void setDopplerFactor(f32 value);
        static void setVolume(f32 value);
        static bool init();
        static void update();
        static void dispose();
    };

}
