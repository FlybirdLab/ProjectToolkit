/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Reference:
 *     https://github.com/kcat/openal-soft
 */

#include <toolkit/audio/AudioSystem.hpp>

#if TK_AUDIO_BACKEND_OPENAL

#include "toolkit/audio/Audio.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/threading/Sync.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/audio/Audio.hpp"
#include "toolkit/audio/AudioListener.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/audio/AudioSource.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/audio/AudioTool.hpp"

// #define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

namespace toolkit
{

#define UNITS_TO_METERS_SCALE 0.01f
#define TK_DST_TO_OAL(x) x *UNITS_TO_METERS_SCALE
#define TK_POS_TO_OAL(vec) ((ALfloat)vec.x * -UNITS_TO_METERS_SCALE), ((ALfloat)vec.y * UNITS_TO_METERS_SCALE), ((ALfloat)vec.z * UNITS_TO_METERS_SCALE)
#define TK_VEL_TO_OAL(vec) ((ALfloat)vec.x * -(UNITS_TO_METERS_SCALE * UNITS_TO_METERS_SCALE)), ((ALfloat)vec.y * (UNITS_TO_METERS_SCALE * UNITS_TO_METERS_SCALE)), ((ALfloat)vec.z * (UNITS_TO_METERS_SCALE * UNITS_TO_METERS_SCALE))
#if TK_DEBUG
#define ALC_CHECK_ERROR(method)                                                                                                    \
    {                                                                                                                              \
        int alError = alGetError();                                                                                                \
        if (alError != 0)                                                                                                          \
        {                                                                                                                          \
            const char *errorStr = openal::getOpenALErrorString(alError);                                                          \
            LogError("[OpenAL] method %s failed with error 0x%X:%s (at line %i)", #method, alError, errorStr, __LINE__ - 1); \
        }                                                                                                                          \
    }
#else
#define ALC_CHECK_ERROR(method)
#endif

    namespace openal
    {
        const char *getOpenALErrorString(int error)
        {
            switch (error)
            {
            case AL_NO_ERROR:
                return "AL_NO_ERROR";
            case AL_INVALID_NAME:
                return "AL_INVALID_NAME";
            case AL_INVALID_ENUM:
                return "AL_INVALID_ENUM";
            case AL_INVALID_VALUE:
                return "AL_INVALID_VALUE";
            case AL_INVALID_OPERATION:
                return "AL_INVALID_OPERATION";
            case AL_OUT_OF_MEMORY:
                return "AL_OUT_OF_MEMORY";
            default:
                break;
            }
            return "???";
        }

        struct SourceData
        {
            AudioDataInfo format;
            f32 pan;
            bool spatial;
        };

        ALCdevice *s_device = nullptr;
        ALCcontext *s_ctx = nullptr;
        AudioFeatureFlags s_features = AudioFeatureFlags::kNone;
        TkMutex s_mtx;
        TkHashMap<u32, SourceData> s_srcData;

        bool isExtSupported(const char *extension)
        {
            if (s_device == nullptr)
                return false;
            const i32 length = TkStringUtils::length(extension);
            if ((length > 2) && (TkStringUtils::compareStrings(extension, "ALC", 3) == 0))
                return alcIsExtensionPresent(s_device, extension) != AL_FALSE;
            return alIsExtensionPresent(extension) != AL_FALSE;
        }

        void clearContext()
        {
            if (s_ctx)
            {
                alcMakeContextCurrent(nullptr);
                alcDestroyContext(s_ctx);
                s_ctx = nullptr;
            }
        }

        void alcListenerRebuild(const AudioListener *listener)
        {
            ANativeListener::reset();
            ANativeListener::transformChanged(listener->getPosition(), listener->getOrientation());
            ANativeListener::velocityChanged(listener->getVelocity());
        }

        void alcSourceSetupSpatial(u32 sourceID, f32 pan, bool spatial)
        {
            alSourcei(sourceID, AL_SOURCE_RELATIVE, !spatial); // Non-spatial sounds use AL_POSITION for panning
#ifdef AL_SOFT_source_spatialize
            alSourcei(sourceID, AL_SOURCE_SPATIALIZE_SOFT, spatial || TkMath::abs(pan) > TkMath::ZeroTolerance ? AL_TRUE : AL_FALSE); // Fix multi-channel sources played as spatial or non-spatial sources played with panning
#endif
            if (spatial)
            {
#ifdef AL_EXT_STEREO_ANGLES
                const f32 panAngle = pan * TkMath::PI_HALF;
                const ALfloat panAngles[2] = {(ALfloat)(TkMath::PI / 6.0 - panAngle), (ALfloat)(-TkMath::PI / 6.0 - panAngle)}; // Angles are specified counter-clockwise in radians
                alSourcefv(sourceID, AL_STEREO_ANGLES, panAngles);
#endif
            }
            else
            {
                alSource3f(sourceID, AL_POSITION, pan, 0, -sqrtf(1.0f - pan * pan));
            }
        }

        void alcSourceRebuild(u32 &sourceID, const Vec3 &position, const Quat &orientation, f32 volume, f32 pitch, f32 pan, bool loop, bool spatial, f32 attenuation, f32 minDistance, f32 doppler)
        {
            TK_ASSERT(sourceID == 0);
            alGenSources(1, &sourceID);
            TK_ASSERT(sourceID != 0);

            alSourcef(sourceID, AL_GAIN, volume);
            alSourcef(sourceID, AL_PITCH, pitch);
            alSourcef(sourceID, AL_SEC_OFFSET, 0.0f);
            alSourcei(sourceID, AL_LOOPING, loop);
            alSourcei(sourceID, AL_BUFFER, 0);
            alcSourceSetupSpatial(sourceID, pan, spatial);
            if (spatial)
            {
                alSourcef(sourceID, AL_ROLLOFF_FACTOR, attenuation);
                alSourcef(sourceID, AL_DOPPLER_FACTOR, doppler);
                alSourcef(sourceID, AL_REFERENCE_DISTANCE, TK_DST_TO_OAL(minDistance));
                alSource3f(sourceID, AL_POSITION, TK_POS_TO_OAL(position));
                alSource3f(sourceID, AL_VELOCITY, TK_VEL_TO_OAL(Vec3::kZero));
            }
            else
            {
                alSourcef(sourceID, AL_ROLLOFF_FACTOR, 0.0f);
                alSourcef(sourceID, AL_DOPPLER_FACTOR, 1.0f);
                alSourcef(sourceID, AL_REFERENCE_DISTANCE, 0.0f);
                alSource3f(sourceID, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
            }
        }

        struct AudioSourceState
        {
            AudioSource::State State;
            f32 Time;
        };

        void rebuildContext(const TkArray<AudioSourceState> &states)
        {
            LogInfo("[OpenAL] Rebuilding audio contexts");

            clearContext();

            if (s_device == nullptr)
                return;

            ALCint attrList[] = {ALC_HRTF_SOFT, ALC_FALSE};
            if (Audio::getEnableHRTF())
            {
                LogInfo("[OpenAL] Enabling HRTF");
                attrList[1] = ALC_TRUE;
            }

            s_ctx = alcCreateContext(s_device, attrList);
            alcMakeContextCurrent(s_ctx);

            for (AudioListener *listener : Audio::Listeners)
                alcListenerRebuild(listener);

            for (i32 i = 0; i < states.count(); i++)
            {
                AudioSource *source = Audio::Sources[i];
                u32 id = source->getSourceId();
                alcSourceRebuild(id, source->getPosition(), source->getOrientation(), source->getVolume(), source->getPitch(), source->getPan(), source->getIsLooping() && !source->useStreaming(), source->is3D(), source->getAttenuation(), source->getMinDistance(), source->getDopplerFactor());
                source->setSourceId(id);

                if (states.hasItems())
                {
                    // Restore playback state
                    auto &state = states[i];
                    if (state.State != AudioSource::State::kStopped)
                        source->play();
                    if (state.State == AudioSource::State::kPaused)
                        source->pause();
                    if (state.State != AudioSource::State::kStopped)
                        source->setTime(state.Time);
                }
            }
        }

        void rebuildContext(bool isChangingDevice)
        {
            TkArray<AudioSourceState> states;
            if (!isChangingDevice)
            {
                states.ensureCapacity(Audio::Sources.count());
                for (AudioSource *source : Audio::Sources)
                {
                    states.add({source->getState(), source->getTime()});
                    source->stop();
                    if (source->getSourceId())
                    {
                        u32 id = source->getSourceId();
                        alDeleteSources(1, &id);
                        ALC_CHECK_ERROR(alDeleteSources);
                        source->setSourceId(0);
                    }
                }
            }

            rebuildContext(states);
        }

        ALenum getOpenALBufferFormat(u32 numChannels, u32 bitDepth)
        {
            // TODO: cache enum values in Init()??
            switch (bitDepth)
            {
            case 8:
                switch (numChannels)
                {
                case 1:
                    return AL_FORMAT_MONO8;
                case 2:
                    return AL_FORMAT_STEREO8;
                case 4:
                    return alGetEnumValue("AL_FORMAT_QUAD8");
                case 6:
                    return alGetEnumValue("AL_FORMAT_51CHN8");
                case 7:
                    return alGetEnumValue("AL_FORMAT_61CHN8");
                case 8:
                    return alGetEnumValue("AL_FORMAT_71CHN8");
                }
            case 16:
                switch (numChannels)
                {
                case 1:
                    return AL_FORMAT_MONO16;
                case 2:
                    return AL_FORMAT_STEREO16;
                case 4:
                    return alGetEnumValue("AL_FORMAT_QUAD16");
                case 6:
                    return alGetEnumValue("AL_FORMAT_51CHN16");
                case 7:
                    return alGetEnumValue("AL_FORMAT_61CHN16");
                case 8:
                    return alGetEnumValue("AL_FORMAT_71CHN16");
                }
            case 32:
                switch (numChannels)
                {
                case 1:
#ifdef AL_FORMAT_MONO_FLOAT32
                    return AL_FORMAT_MONO_FLOAT32;
#else
                    return alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
#endif
                case 2:
#ifdef AL_FORMAT_STEREO_FLOAT32
                    return AL_FORMAT_STEREO_FLOAT32;
#else
                    return alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
#endif
                case 4:
                    return alGetEnumValue("AL_FORMAT_QUAD32");
                case 6:
                    return alGetEnumValue("AL_FORMAT_51CHN32");
                case 7:
                    return alGetEnumValue("AL_FORMAT_61CHN32");
                case 8:
                    return alGetEnumValue("AL_FORMAT_71CHN32");
                }
            }
            return 0;
        }

    }

    void ANativeListener::reset()
    {
        alListenerf(AL_GAIN, Audio::getVolume());
    }

    void ANativeListener::velocityChanged(const Vec3 &velocity)
    {
        alListener3f(AL_VELOCITY, TK_VEL_TO_OAL(velocity));
    }

    void ANativeListener::transformChanged(const Vec3 &pos, const Quat &orientation)
    {
        const FVec3 flipX(-1, 1, 1);
        const FVec3 alOrientation[2] =
            {
                orientation * FVec3::kForward * flipX,
                orientation * FVec3::kUp * flipX};
        alListenerfv(AL_ORIENTATION, (float *)alOrientation);
        alListener3f(AL_POSITION, TK_POS_TO_OAL(pos));
    }

    void ANativeListener::reinitAll()
    {
        openal::rebuildContext(false);
    }

    u32 ANativeSource::add(const AudioDataInfo &format, const Vec3 &pos, const Quat &orientation, f32 volume, f32 pitch, f32 pan, bool loop, bool spatial, f32 attenuation, f32 minDistance, f32 doppler)
    {
        u32 sourceID = 0;
        openal::alcSourceRebuild(sourceID, pos, orientation, volume, pitch, pan, loop, spatial, attenuation, minDistance, doppler);

        // Cache audio data format assigned on source (used in Source_GetCurrentBufferTime)
        openal::s_mtx.lock();
        auto &data = openal::s_srcData[sourceID];
        data.format = format;
        data.spatial = spatial;
        data.pan = pan;
        openal::s_mtx.unlock();

        return sourceID;
    }

    void ANativeSource::remove(u32 sourceID)
    {
        alSourcei(sourceID, AL_BUFFER, 0);
        ALC_CHECK_ERROR(alSourcei);
        alDeleteSources(1, &sourceID);
        ALC_CHECK_ERROR(alDeleteSources);

        openal::s_mtx.lock();
        openal::s_srcData.remove(sourceID);
        openal::s_mtx.unlock();
    }

    void ANativeSource::velocityChanged(u32 sourceID, const Vec3 &velocity)
    {
        openal::s_mtx.lock();
        const bool spatial = openal::s_srcData[sourceID].spatial;
        openal::s_mtx.unlock();
        if (spatial)
        {
            alSource3f(sourceID, AL_VELOCITY, TK_VEL_TO_OAL(velocity));
        }
    }

    void ANativeSource::transformChanged(u32 sourceID, const Vec3 &pos, const Quat &orientation)
    {
        openal::s_mtx.lock();
        const bool spatial = openal::s_srcData[sourceID].spatial;
        openal::s_mtx.unlock();
        if (spatial)
        {
            alSource3f(sourceID, AL_POSITION, TK_POS_TO_OAL(pos));
        }
    }

    void ANativeSource::volumeChanged(u32 sourceID, f32 volume)
    {
        alSourcef(sourceID, AL_GAIN, volume);
    }

    void ANativeSource::pitchChanged(u32 sourceID, f32 pitch)
    {
        alSourcef(sourceID, AL_PITCH, pitch);
    }

    void ANativeSource::panChanged(u32 sourceID, f32 pan)
    {
        openal::s_mtx.lock();
        auto &e = openal::s_srcData[sourceID];
        e.pan = pan;
        const bool spatial = e.spatial;
        openal::s_mtx.unlock();
        openal::alcSourceSetupSpatial(sourceID, pan, spatial);
    }

    void ANativeSource::isLoopingChanged(u32 sourceID, bool loop)
    {
        alSourcei(sourceID, AL_LOOPING, loop);
    }

    void ANativeSource::spatialSetupChanged(u32 sourceID, bool spatial, f32 attenuation, f32 minDistance, f32 doppler)
    {
        openal::s_mtx.lock();
        const bool pan = openal::s_srcData[sourceID].spatial;
        openal::s_mtx.unlock();
        if (spatial)
        {
            alSourcef(sourceID, AL_ROLLOFF_FACTOR, attenuation);
            alSourcef(sourceID, AL_DOPPLER_FACTOR, doppler);
            alSourcef(sourceID, AL_REFERENCE_DISTANCE, TK_DST_TO_OAL(minDistance));
        }
        else
        {
            alSourcef(sourceID, AL_ROLLOFF_FACTOR, 0.0f);
            alSourcef(sourceID, AL_DOPPLER_FACTOR, 1.0f);
            alSourcef(sourceID, AL_REFERENCE_DISTANCE, 0.0f);
        }
        openal::alcSourceSetupSpatial(sourceID, pan, spatial);
    }

    void ANativeSource::play(u32 sourceID)
    {
        alSourcePlay(sourceID);
        ALC_CHECK_ERROR(alSourcePlay);
    }

    void ANativeSource::pause(u32 sourceID)
    {
        alSourcePause(sourceID);
        ALC_CHECK_ERROR(alSourcePause);
    }

    void ANativeSource::stop(u32 sourceID)
    {
        // Stop and rewind
        alSourceRewind(sourceID);
        ALC_CHECK_ERROR(alSourceRewind);
        alSourcef(sourceID, AL_SEC_OFFSET, 0.0f);
    }

    void ANativeSource::setCurrentBufferTime(u32 sourceID, f32 value)
    {
        alSourcef(sourceID, AL_SEC_OFFSET, value);
    }

    f32 ANativeSource::getCurrentBufferTime(u32 sourceID)
    {
        openal::s_mtx.lock();
        AudioDataInfo clipInfo = openal::s_srcData[sourceID].format;
        openal::s_mtx.unlock();
        ALint samplesPlayed;
        alGetSourcei(sourceID, AL_SAMPLE_OFFSET, &samplesPlayed);
        const u32 totalSamples = clipInfo.numSamples / clipInfo.numChannels;
        if (totalSamples > 0)
            samplesPlayed %= totalSamples;
        const f32 time = samplesPlayed / static_cast<float>(TkMath::max(1U, clipInfo.sampleRate));
        return time;
    }

    void ANativeSource::setNonStreamingBuffer(u32 sourceID, u32 bufferID)
    {
        alSourcei(sourceID, AL_BUFFER, bufferID);
        ALC_CHECK_ERROR(alSourcei);
    }

    void ANativeSource::getProcessedBuffersCount(u32 sourceID, i32 &processedBuffersCount)
    {
        // Check the first context only
        alGetSourcei(sourceID, AL_BUFFERS_PROCESSED, &processedBuffersCount);
        ALC_CHECK_ERROR(alGetSourcei);
    }

    void ANativeSource::getQueuedBuffersCount(u32 sourceID, i32 &queuedBuffersCount)
    {
        // Check the first context only
        alGetSourcei(sourceID, AL_BUFFERS_QUEUED, &queuedBuffersCount);
        ALC_CHECK_ERROR(alGetSourcei);
    }

    void ANativeSource::queueBuffer(u32 sourceID, u32 bufferID)
    {
        // Queue new buffer
        alSourceQueueBuffers(sourceID, 1, &bufferID);
        ALC_CHECK_ERROR(alSourceQueueBuffers);
    }

    void ANativeSource::dequeueProcessedBuffers(u32 sourceID)
    {
        i32 numProcessedBuffers = 0;
        alGetSourcei(sourceID, AL_BUFFERS_PROCESSED, &numProcessedBuffers);
        if (numProcessedBuffers <= 0)
            return;
        numProcessedBuffers = TkMath::clamp(numProcessedBuffers, 0, AUDIO_MAX_SOURCE_BUFFERS);
        TkStaticArray<ALuint, AUDIO_MAX_SOURCE_BUFFERS> buffers;
        buffers.resize(numProcessedBuffers);
        alSourceUnqueueBuffers(sourceID, numProcessedBuffers, buffers.get());
        ALC_CHECK_ERROR(alSourceUnqueueBuffers);
    }

    void ANativeSource::clearQueuedBuffers(u32 sourceID)
    {
        alSourceStop(sourceID);
        ALC_CHECK_ERROR(alSourceStop);

        i32 queuedBuffers = 0;
        alGetSourcei(sourceID, AL_BUFFERS_QUEUED, &queuedBuffers);
        queuedBuffers = TkMath::clamp(queuedBuffers, 0, AUDIO_MAX_SOURCE_BUFFERS);
        if (queuedBuffers > 0)
        {
            TkStaticArray<ALuint, AUDIO_MAX_SOURCE_BUFFERS> buffers;
            buffers.resize(queuedBuffers);
            alSourceUnqueueBuffers(sourceID, queuedBuffers, buffers.get());
            ALC_CHECK_ERROR(alSourceUnqueueBuffers);
        }

        alSourcei(sourceID, AL_BUFFER, 0);
        ALC_CHECK_ERROR(alSourcei);
        alSourcef(sourceID, AL_SEC_OFFSET, 0.0f);
    }

    u32 ANativeBuffer::create()
    {
        u32 bufferID;
        alGenBuffers(1, &bufferID);
        ALC_CHECK_ERROR(alGenBuffers);
        return bufferID;
    }

    void ANativeBuffer::destroy(u32 bufferID)
    {
        alDeleteBuffers(1, &bufferID);
        ALC_CHECK_ERROR(alDeleteBuffers);
    }

    bool ANativeBuffer::write(u32 bufferID, const byte *samples, const AudioDataInfo &info)
    {
        PROFILE();
        if (bufferID == 0 || samples == nullptr || info.numSamples == 0 || info.sampleRate == 0 || info.numChannels == 0 || info.bitDepth == 0)
            return false;

        // Pick the format for the audio data (it might not be supported natively)
        ALenum format = openal::getOpenALBufferFormat(info.numChannels, info.bitDepth);

        // Mono or stereo
        if (info.numChannels <= 2)
        {
            if (info.bitDepth > 16)
            {
                if (openal::isExtSupported("AL_EXT_float32"))
                {
                    const u32 bufferSize = info.numSamples * sizeof(float);
                    f32 *sampleBufferFloat = (float *)TkMemory::allocate(TkMemory::getDefault(), bufferSize);
                    AudioTool::convertToFloat(samples, info.bitDepth, sampleBufferFloat, info.numSamples);

                    format = openal::getOpenALBufferFormat(info.numChannels, 32);
                    alBufferData(bufferID, format, sampleBufferFloat, bufferSize, info.sampleRate);
                    ALC_CHECK_ERROR(alBufferData);
                    TkMemory::free(TkMemory::getDefault(), sampleBufferFloat);
                }
                else
                {
                    LogWarn("OpenAL doesn't support bit depth larger than 16. Audio data will be truncated.");
                    const u32 bufferSize = info.numSamples * 2;
                    byte *sampleBuffer16 = (byte *)TkMemory::allocate(TkMemory::getDefault(), bufferSize);
                    AudioTool::convertBitDepth(samples, info.bitDepth, sampleBuffer16, 16, info.numSamples);

                    format = openal::getOpenALBufferFormat(info.numChannels, 16);
                    alBufferData(bufferID, format, sampleBuffer16, bufferSize, info.sampleRate);
                    ALC_CHECK_ERROR(alBufferData);
                    TkMemory::free(TkMemory::getDefault(), sampleBuffer16);
                }
            }
            else if (info.bitDepth == 8)
            {
                // OpenAL expects unsigned 8-bit data, but engine stores it as signed, so convert
                const u32 bufferSize = info.numSamples * (info.bitDepth / 8);
                byte *sampleBuffer = (byte *)TkMemory::allocate(TkMemory::getDefault(), bufferSize);
                for (u32 i = 0; i < info.numSamples; i++)
                    sampleBuffer[i] = static_cast<byte>(static_cast<i8>(samples[i]) + 128);

                alBufferData(bufferID, format, sampleBuffer, bufferSize, info.sampleRate);
                ALC_CHECK_ERROR(alBufferData);
                TkMemory::free(TkMemory::getDefault(), sampleBuffer);
            }
            else if (format)
            {
                alBufferData(bufferID, format, samples, info.numSamples * (info.bitDepth / 8), info.sampleRate);
                ALC_CHECK_ERROR(alBufferData);
            }
        }
        // Multichannel
        else
        {
            // Note: Assuming AL_EXT_MCFORMATS is supported. If it's not, channels should be reduced to mono or stereo.

            // 24-bit not supported, convert to 32-bit
            if (info.bitDepth == 24)
            {
                const u32 bufferSize = info.numChannels * sizeof(i32);
                byte *sampleBuffer32 = (byte *)TkMemory::allocate(TkMemory::getDefault(), bufferSize);
                AudioTool::convertBitDepth(samples, info.bitDepth, sampleBuffer32, 32, info.numSamples);

                format = openal::getOpenALBufferFormat(info.numChannels, 32);
                alBufferData(bufferID, format, sampleBuffer32, bufferSize, info.sampleRate);
                ALC_CHECK_ERROR(alBufferData);

                TkMemory::free(TkMemory::getDefault(), sampleBuffer32);
            }
            else if (info.bitDepth == 8)
            {
                // OpenAL expects unsigned 8-bit data, but engine stores it as signed, so convert
                const u32 bufferSize = info.numSamples * (info.bitDepth / 8);
                byte *sampleBuffer = (byte *)TkMemory::allocate(TkMemory::getDefault(), bufferSize);

                for (u32 i = 0; i < info.numSamples; i++)
                    sampleBuffer[i] = static_cast<byte>(static_cast<i8>(samples[i]) + 128);

                format = openal::getOpenALBufferFormat(info.numChannels, 16);
                alBufferData(bufferID, format, sampleBuffer, bufferSize, info.sampleRate);
                ALC_CHECK_ERROR(alBufferData);

                TkMemory::free(TkMemory::getDefault(), sampleBuffer);
            }
            else if (format)
            {
                alBufferData(bufferID, format, samples, info.numSamples * (info.bitDepth / 8), info.sampleRate);
                ALC_CHECK_ERROR(alBufferData);
            }
        }

        if (!format)
        {
            LogError("Not supported audio data format for OpenAL device: BitDepth=%i, NumChannels=%i", info.bitDepth, info.numChannels);
            return false;
        }
        return true;
    }

    const char *AudioSystem::getName()
    {
        return "OpenAL";
    }

    AudioFeatureFlags AudioSystem::getFeatures()
    {
        return openal::s_features;
    }

    void AudioSystem::onActiveDeviceChanged()
    { // Cleanup
        TkArray<openal::AudioSourceState> states;
        states.ensureCapacity(Audio::Sources.count());
        for (AudioSource *source : Audio::Sources)
        {
            states.add({source->getState(), source->getTime()});
            source->stop();
            if (source->getSourceId())
            {
                ANativeSource::remove(source->getSourceId());
                source->setSourceId(0);
            }
        }
        openal::clearContext();
        if (openal::s_device != nullptr)
        {
            alcCloseDevice(openal::s_device);
            openal::s_device = nullptr;
        }

        // Open device
        const AudioDevice *activeDevice = Audio::getActiveDevice();
        if (activeDevice == nullptr)
            return;

        const TkString &name = activeDevice->internalName;
        openal::s_device = alcOpenDevice(name.get());
        if (openal::s_device == nullptr)
        {
            LogError("Failed to open OpenAL device (%s).", name.get());
            return;
        }

        // Setup
        openal::rebuildContext(states);
    }

    void AudioSystem::setDopplerFactor(f32 value)
    {

        alDopplerFactor(value);
    }

    void AudioSystem::setVolume(f32 value)
    {
        alListenerf(AL_GAIN, value);
    }

    bool AudioSystem::init()
    {
        auto &devices = Audio::Devices;

        // Init default device
        i32 activeDeviceIndex;
        openal::s_device = alcOpenDevice(nullptr);

        if (openal::s_device == nullptr)
        {
            activeDeviceIndex = -1;
            const auto err = alGetError();
            LogWarn("[OpenAL] Failed to open default OpenAL device. Error: 0x%X", err);
        }
        else
        {
            activeDeviceIndex = 0;
        }

// Enumerate devices
#if ALC_ENUMERATE_ALL_EXT
        const ALCchar *defaultDevice = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);

        if (openal::isExtSupported("ALC_ENUMERATE_ALL_EXT") && defaultDevice != nullptr)
        {
            const ALCchar *devicesStr = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
            const TkString defaultDeviceName(defaultDevice);

            devices.clear();
            devices.ensureCapacity(8);

            activeDeviceIndex = -1;

            while (devicesStr && *devicesStr)
            {
                const i32 i = devices.count();
                devices.resize(i + 1);
                auto &device = devices[i];

                device.internalName = devicesStr;
                device.name = TkString(device.internalName).trimmed();
                device.name.replace("OpenAL Soft on ", "");

                if (device.internalName == defaultDeviceName)
                {
                    activeDeviceIndex = i;
                }

                devicesStr += (device.internalName.length() + 1) * sizeof(ALCchar);
            }

            if (activeDeviceIndex == -1)
            {
                LogWarn("Failed to pick a default device");
                LogWarn("%s", defaultDeviceName.get());

                for (i32 i = 0; i < devices.count(); i++)
                    LogWarn("%s", devices[i].name.get());

                if (devices.isEmpty())
                    return false;

                LogWarn("[OpenAL] Using the first audio device");
                activeDeviceIndex = 0;
            }

            // Reopen selected device
            if (openal::s_device)
                alcCloseDevice(openal::s_device);

            const auto &name = devices[activeDeviceIndex].internalName;
            openal::s_device = alcOpenDevice(name.get());

            if (openal::s_device == nullptr)
            {
                LogWarn("Failed to open OpenAL device (%s).", name.get());
                return false;
            }
        }
        else
#endif
        {
            if (openal::s_device)
            {
                devices.resize(1);
                devices[0].name = "Default device";
            }
            else
            {
                devices.resize(0);
            }
        }

        // Init params
        AudioSystem::setDopplerFactor(AudioSystem::s_dopplerFactor);
        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

        i32 clampedIndex = TkMath::clamp(activeDeviceIndex, -1, Audio::Devices.count() - 1);

        if (clampedIndex == Audio::getActiveDeviceIdx())
        {
            openal::rebuildContext(true);
        }

        Audio::setActiveDeviceIdx(activeDeviceIndex);

#ifdef AL_SOFT_source_spatialize
        if (openal::isExtSupported("AL_SOFT_source_spatialize"))
        {
            openal::s_features = enumAddFlags(openal::s_features, AudioFeatureFlags::kSpatialMultiChannel);
        }
#endif

        // Log
        LogInfo("%s (%s)", alGetString(AL_RENDERER), alGetString(AL_VERSION));

        for (i32 i = 0; i < devices.count(); i++)
        {
            LogInfo("%s%s",
                    i == activeDeviceIndex ? "[active] " : "",
                    devices[i].name.get());
        }

        return true;
    }
    void AudioSystem::update() {}
    void AudioSystem::dispose()
    {
        if (openal::s_device != nullptr)
        {
            alcCloseDevice(openal::s_device);
            openal::s_device = nullptr;
        }
    }
}

#endif
