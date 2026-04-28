/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References:
 *    https://docs.microsoft.com/en-us/windows/desktop/xaudio2/xaudio2-apis-portal
 */

#include <toolkit/audio/AudioSystem.hpp>

#if TK_AUDIO_BACKEND_XAUDIO

#include <toolkit/base/Assert.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/collection/Array.hpp>
#include "toolkit/threading/Sync.hpp"
#include <toolkit/collection/ChunkedArray.hpp>
#include "toolkit/threading/Thread.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/audio/Audio.hpp"
#include <objbase.h>

#if TK_WINDOWS
// Tweak Win ver
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0602
#endif
#endif

#include <xaudio2.h>

// TODO: implement multi-channel support (eg. 5.1, 7.1)
#define MAX_INPUT_CHANNELS 6
#define MAX_OUTPUT_CHANNELS 2
#define MAX_CHANNELS_MATRIX_SIZE (MAX_INPUT_CHANNELS * MAX_OUTPUT_CHANNELS)
#if TK_DEBUG
#define AUDIO_ERROR_CHECK(method)                                                                                      \
    if (hr != 0)                                                                                                       \
    {                                                                                                                  \
        LogError("[xaudio] method %s failed with error 0x%x (at line %d)", #method, (u32)hr, __LINE__ - 1);            \
    }
#else
#define AUDIO_ERROR_CHECK(method)
#endif

namespace toolkit
{
    namespace xaudio
    {
        class VoiceCallback : public IXAudio2VoiceCallback
        {
            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnVoiceProcessingPassStart(THIS_ UINT32 BytesRequired) override
            {
            }

            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnVoiceProcessingPassEnd(THIS) override
            {
            }

            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnStreamEnd(THIS_) override
            {
            }

            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnBufferStart(THIS_ void *pBufferContext) override
            {
                peekSamples();
            }

            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnBufferEnd(THIS_ void *pBufferContext) override;

            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnLoopEnd(THIS_ void *pBufferContext) override
            {
            }

            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnVoiceError(THIS_ void *pBufferContext, HRESULT Error) override
            {
#if TK_DEBUG
                LogWarn("[xaudio] IXAudio2VoiceCallback::OnVoiceError! Error: 0x%x", (u32)Error);
#endif
            }

        public:
            u32 sourceID;
            void peekSamples();
        };

        struct Source : public ANativeSource
        {
            IXAudio2SourceVoice *voice;
            WAVEFORMATEX format;
            AudioDataInfo info;
            XAUDIO2_SEND_DESCRIPTOR destination;
            float startTimeForQueueBuffer;
            float lastBufferStartTime;
            u64 lastBufferStartSamplesPlayed;
            i32 buffersProcessed;
            i32 channels;
            bool isDirty;
            bool isPlaying;
            bool isLoop;
            u32 lastBufferID;
            VoiceCallback callback;

            Source()
            {
                init();
            }

            void init()
            {
                voice = nullptr;
                destination.Flags = 0;
                destination.pOutputVoice = nullptr;
                pitch = 1.0f;
                pan = 0.0f;
                startTimeForQueueBuffer = 0.0f;
                lastBufferStartTime = 0.0f;
                isDirty = false;
                is3D = false;
                isPlaying = false;
                isLoop = false;
                lastBufferID = 0;
                lastBufferStartSamplesPlayed = 0;
                buffersProcessed = 0;
            }

            bool isFree() const
            {
                return voice == nullptr;
            }
        };

        struct Listener : public ANativeListener
        {
        };

        struct Buffer
        {
            AudioDataInfo info;
            TkArray<byte> data;
        };

        class EngineCallback : public IXAudio2EngineCallback
        {
            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnProcessingPassEnd() override
            {
            }

            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnProcessingPassStart() override
            {
            }

            COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OnCriticalError(HRESULT error) override
            {
                LogWarn("[xaudio] IXAudio2EngineCallback::OnCriticalError! Error: 0x%x", (u32)error);
            }
        };

        IXAudio2 *s_instance = nullptr;
        IXAudio2MasteringVoice *s_masteringVoice = nullptr;
        i32 s_channels;
        DWORD s_channelMask;
        bool s_forceDirty = true;
        bool s_comInitialized = false;
        SoundMixParams s_settings{
            .volume = 1.0f,
            .dopplerFactor = 1.0f,
        };
        Listener s_listener;
        TkMutex s_mtx;
        TkChunkedArray<Source, 32> s_src;
        TkChunkedArray<Buffer *, 64> s_buf; // TODO: use ChunkedArray for better performance or use buffers pool?
        EngineCallback s_callback;

        Source *getSource(u32 sourceID)
        {
            if (sourceID == 0)
                return nullptr;
            return &s_src[sourceID - 1]; // 0 is invalid ID so shift them
        }

        void markAllDirty()
        {
            s_forceDirty = true;
        }

        void QueueBuffer(Source *aSource, const i32 bufferID, XAUDIO2_BUFFER &buffer)
        {
            Buffer *aBuffer = s_buf[bufferID - 1];
            buffer.pAudioData = aBuffer->data.get();
            buffer.AudioBytes = aBuffer->data.count();

            if (aSource->startTimeForQueueBuffer > TkMath::ZeroTolerance)
            {
                // Offset start position when playing buffer with a custom time offset
                const u32 bytesPerSample = aBuffer->info.bitDepth / 8 * aBuffer->info.numChannels;
                buffer.PlayBegin = (UINT32)(aSource->startTimeForQueueBuffer * aBuffer->info.sampleRate);
                buffer.PlayLength = (buffer.AudioBytes / bytesPerSample) - buffer.PlayBegin;
                aSource->lastBufferStartTime = aSource->startTimeForQueueBuffer;
                aSource->startTimeForQueueBuffer = 0;
            }

            const HRESULT hr = aSource->voice->SubmitSourceBuffer(&buffer);
            AUDIO_ERROR_CHECK(SubmitSourceBuffer);
        }

        void VoiceCallback::OnBufferEnd(void *pBufferContext)
        {
            auto aSource = getSource(sourceID);
            if (aSource->isPlaying)
                aSource->buffersProcessed++;
        }

        void VoiceCallback::peekSamples()
        {
            auto aSource = getSource(sourceID);
            XAUDIO2_VOICE_STATE state;
            aSource->voice->GetState(&state);
            aSource->lastBufferStartSamplesPlayed = state.SamplesPlayed;
        }
    }

    void ANativeListener::reset()
    {
        xaudio::s_listener.reset();
        xaudio::markAllDirty();
    }

    void ANativeListener::velocityChanged(const Vec3 &velocity)
    {
        xaudio::s_listener.velocity = velocity;
        xaudio::markAllDirty();
    }

    void ANativeListener::transformChanged(const Vec3 &pos, const Quat &orientation)
    {
        xaudio::s_listener.pos = pos;
        xaudio::s_listener.orientation = orientation;
        xaudio::markAllDirty();
    }

    void ANativeListener::reinitAll()
    {
        // TODO: Implement xaudio reinitialization; read HRTF audio value from Audio class
    }

    u32 ANativeSource::add(const AudioDataInfo &format, const Vec3 &pos, const Quat &orientation, f32 volume, f32 pitch, f32 pan, bool loop, bool spatial, f32 attenuation, f32 minDistance, f32 doppler)
    {
        TkMutexGuard lock(xaudio::s_mtx);

        // Get first free source
        xaudio::Source *aSource = nullptr;
        u32 sourceID = 0;
        for (i32 i = 0; i < xaudio::s_src.count(); i++)
        {
            if (xaudio::s_src[i].isFree())
            {
                sourceID = i;
                aSource = &xaudio::s_src[i];
                break;
            }
        }
        if (aSource == nullptr)
        {
            // Add new
            const xaudio::Source src;
            sourceID = xaudio::s_src.count();
            xaudio::s_src.add(src);
            aSource = &xaudio::s_src[sourceID];
        }
        sourceID++; // 0 is invalid ID so shift them

        // Initialize audio data format information (from clip)
        aSource->info = format;
        auto &aFormat = aSource->format;
        aFormat.wFormatTag = WAVE_FORMAT_PCM;
        aFormat.nChannels = spatial ? 1 : format.numChannels; // 3d audio is always mono (AudioClip auto-converts before buffer write if FeatureFlags::SpatialMultiChannel is unset)
        aFormat.nSamplesPerSec = format.sampleRate;
        aFormat.wBitsPerSample = format.bitDepth;
        aFormat.nBlockAlign = (WORD)(aFormat.nChannels * (aFormat.wBitsPerSample / 8));
        aFormat.nAvgBytesPerSec = aFormat.nSamplesPerSec * aFormat.nBlockAlign;
        aFormat.cbSize = 0;

        // Setup dry effect
        aSource->destination.pOutputVoice = xaudio::s_masteringVoice;

        // Create voice
        const XAUDIO2_VOICE_SENDS sendList = {1, &aSource->destination};
        HRESULT hr = xaudio::s_instance->CreateSourceVoice(&aSource->voice, &aSource->format, 0, 2.0f, &aSource->callback, &sendList);
        AUDIO_ERROR_CHECK(CreateSourceVoice);
        if (FAILED(hr))
            return 0;

        // Prepare source state
        aSource->callback.sourceID = sourceID;
        aSource->isDirty = true;
        aSource->isLoop = loop;
        aSource->is3D = spatial;
        aSource->pitch = pitch;
        aSource->pan = pan;
        aSource->dopplerFactor = doppler;
        aSource->volume = volume;
        aSource->minDistance = minDistance;
        aSource->attenuation = attenuation;
        aSource->channels = aFormat.nChannels;
        aSource->pos = pos;
        aSource->orientation = orientation;
        aSource->velocity = Vec3::kZero;
        hr = aSource->voice->SetVolume(volume);
        AUDIO_ERROR_CHECK(SetVolume);

        return sourceID;
    }

    void ANativeSource::remove(u32 sourceID)
    {
        TkMutexGuard lock(xaudio::s_mtx);
        auto aSource = xaudio::getSource(sourceID);
        if (!aSource)
            return;

        // Free source
        if (aSource->voice)
        {
            aSource->voice->DestroyVoice();
        }
        aSource->init();
    }

    void ANativeSource::velocityChanged(u32 sourceID, const Vec3 &velocity)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource)
        {
            aSource->velocity = velocity;
            aSource->isDirty = true;
        }
    }

    void ANativeSource::transformChanged(u32 sourceID, const Vec3 &pos, const Quat &orientation)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource)
        {
            aSource->pos = pos;
            aSource->orientation = orientation;
            aSource->isDirty = true;
        }
    }

    void ANativeSource::volumeChanged(u32 sourceID, f32 volume)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource && aSource->voice)
        {
            aSource->volume = volume;
            const HRESULT hr = aSource->voice->SetVolume(volume);
            AUDIO_ERROR_CHECK(SetVolume);
        }
    }

    void ANativeSource::pitchChanged(u32 sourceID, f32 pitch)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource)
        {
            aSource->pitch = pitch;
            aSource->isDirty = true;
        }
    }

    void ANativeSource::panChanged(u32 sourceID, f32 pan)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource)
        {
            aSource->pan = pan;
            aSource->isDirty = true;
        }
    }

    void ANativeSource::isLoopingChanged(u32 sourceID, bool loop)
    {
        TkMutexGuard lock(xaudio::s_mtx);

        auto aSource = xaudio::getSource(sourceID);
        if (!aSource || !aSource->voice)
            return;
        aSource->isLoop = loop;

        // Skip if has no buffers (waiting for data or sth)
        XAUDIO2_VOICE_STATE state;
        aSource->voice->GetState(&state);
        if (state.BuffersQueued == 0)
            return;

        // Looping is defined during buffer submission so reset source buffer (this is called only for non-streamable sources that use a single buffer)
        const u32 bufferID = aSource->lastBufferID;
        xaudio::Buffer *aBuffer = xaudio::s_buf[bufferID - 1];

        HRESULT hr;
        const bool isPlaying = aSource->isPlaying;
        if (isPlaying)
        {
            hr = aSource->voice->Stop();
            AUDIO_ERROR_CHECK(Stop);
        }

        hr = aSource->voice->FlushSourceBuffers();
        AUDIO_ERROR_CHECK(FlushSourceBuffers);
        aSource->lastBufferStartSamplesPlayed = 0;
        aSource->lastBufferStartTime = 0;
        aSource->buffersProcessed = 0;

        XAUDIO2_BUFFER buffer = {0};
        buffer.pContext = aBuffer;
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        if (loop)
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

        // Restore play position
        const UINT32 totalSamples = aBuffer->info.numSamples / aBuffer->info.numChannels;
        buffer.PlayBegin = state.SamplesPlayed % totalSamples;
        buffer.PlayLength = totalSamples - buffer.PlayBegin;
        aSource->startTimeForQueueBuffer = 0;

        xaudio::QueueBuffer(aSource, bufferID, buffer);

        if (isPlaying)
        {
            hr = aSource->voice->Start();
            AUDIO_ERROR_CHECK(Start);
        }
    }

    void ANativeSource::spatialSetupChanged(u32 sourceID, bool spatial, f32 attenuation, f32 minDistance, f32 doppler)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource)
        {
            aSource->is3D = spatial;
            aSource->minDistance = minDistance;
            aSource->attenuation = attenuation;
            aSource->dopplerFactor = doppler;
            aSource->isDirty = true;
        }
    }

    void ANativeSource::play(u32 sourceID)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource && aSource->voice && !aSource->isPlaying)
        {
            // Play
            const HRESULT hr = aSource->voice->Start();
            AUDIO_ERROR_CHECK(Start);
            aSource->isPlaying = true;
        }
    }

    void ANativeSource::pause(u32 sourceID)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource && aSource->voice && aSource->isPlaying)
        {
            // Pause
            const HRESULT hr = aSource->voice->Stop();
            AUDIO_ERROR_CHECK(Stop);
            aSource->isPlaying = false;
        }
    }

    void ANativeSource::stop(u32 sourceID)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource && aSource->voice)
        {
            aSource->startTimeForQueueBuffer = 0.0f;
            aSource->lastBufferStartTime = 0.0f;

            // Pause
            HRESULT hr = aSource->voice->Stop();
            AUDIO_ERROR_CHECK(Stop);
            aSource->isPlaying = false;

            aSource->buffersProcessed = 0;
            aSource->callback.peekSamples();
        }
    }

    void ANativeSource::setCurrentBufferTime(u32 sourceID, f32 value)
    {
        const auto aSource = xaudio::getSource(sourceID);
        if (aSource)
        {
            // Store start time so next buffer submitted will start from here (assumes audio is stopped)
            aSource->startTimeForQueueBuffer = value;
        }
    }

    f32 ANativeSource::getCurrentBufferTime(u32 sourceID)
    {
        float time = 0;
        auto aSource = xaudio::getSource(sourceID);
        if (aSource)
        {
            const auto &clipInfo = aSource->info;
            XAUDIO2_VOICE_STATE state;
            aSource->voice->GetState(&state);
            const u32 totalSamples = clipInfo.numSamples / clipInfo.numChannels;
            const u32 sampleRate = clipInfo.sampleRate; // / clipInfo.NumChannels;
            u64 lastBufferStartSamplesPlayed = aSource->lastBufferStartSamplesPlayed;
            if (totalSamples > 0)
                lastBufferStartSamplesPlayed %= totalSamples;
            state.SamplesPlayed -= lastBufferStartSamplesPlayed % totalSamples; // Offset by the last buffer start to get time relative to its begin
            if (totalSamples > 0)
                state.SamplesPlayed %= totalSamples;
            time = aSource->lastBufferStartTime + state.SamplesPlayed / static_cast<float>(TkMath::max(1U, sampleRate));
        }
        return time;
    }

    void ANativeSource::setNonStreamingBuffer(u32 sourceID, u32 bufferID)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (!aSource)
            return;
        aSource->lastBufferID = bufferID; // Use for looping change

        xaudio::s_mtx.lock();
        xaudio::Buffer *aBuffer = xaudio::s_buf[bufferID - 1];
        xaudio::s_mtx.unlock();

        XAUDIO2_BUFFER buffer = {0};
        buffer.pContext = aBuffer;
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        if (aSource->isLoop)
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

        // Queue single buffer
        xaudio::QueueBuffer(aSource, bufferID, buffer);
    }

    void ANativeSource::getProcessedBuffersCount(u32 sourceID, i32 &processedBuffersCount)
    {
        processedBuffersCount = 0;
        auto aSource = xaudio::getSource(sourceID);
        if (aSource && aSource->voice)
        {
            processedBuffersCount = aSource->buffersProcessed;
        }
    }

    void ANativeSource::getQueuedBuffersCount(u32 sourceID, i32 &queuedBuffersCount)
    {
        queuedBuffersCount = 0;
        auto aSource = xaudio::getSource(sourceID);
        if (aSource && aSource->voice)
        {
            XAUDIO2_VOICE_STATE state;
            aSource->voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
            queuedBuffersCount = state.BuffersQueued;
        }
    }

    void ANativeSource::queueBuffer(u32 sourceID, u32 bufferID)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (!aSource)
            return;
        aSource->lastBufferID = bufferID; // Use for looping change

        xaudio::Buffer *aBuffer = xaudio::s_buf[bufferID - 1];

        XAUDIO2_BUFFER buffer = {0};
        buffer.pContext = aBuffer;

        xaudio::QueueBuffer(aSource, bufferID, buffer);
    }

    void ANativeSource::dequeueProcessedBuffers(u32 sourceID)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource && aSource->voice)
        {
            aSource->buffersProcessed = 0;
        }
    }

    void ANativeSource::clearQueuedBuffers(u32 sourceID)
    {
        auto aSource = xaudio::getSource(sourceID);
        if (aSource && aSource->voice)
        {
            const HRESULT hr = aSource->voice->FlushSourceBuffers();
            AUDIO_ERROR_CHECK(FlushSourceBuffers);
            aSource->startTimeForQueueBuffer = 0.0f;
            aSource->lastBufferStartTime = 0.0f;
            aSource->lastBufferStartSamplesPlayed = 0;
            aSource->buffersProcessed = 0;
        }
    }

    u32 ANativeBuffer::create()
    {
        u32 bufferID;
        TkMutexGuard lock(xaudio::s_mtx);

        // Get first free buffer slot
        xaudio::Buffer *aBuffer = nullptr;
        for (i32 i = 0; i < xaudio::s_buf.count(); i++)
        {
            if (xaudio::s_buf[i] == nullptr)
            {
                aBuffer = TkMemory::newObject<xaudio::Buffer>();
                xaudio::s_buf[i] = aBuffer;
                bufferID = i + 1;
                break;
            }
        }
        if (!aBuffer)
        {
            // Add new slot
            aBuffer = TkMemory::newObject<xaudio::Buffer>();
            xaudio::s_buf.add(aBuffer);
            bufferID = xaudio::s_buf.count();
        }

        aBuffer->data.resize(0);
        return bufferID;
    }

    void ANativeBuffer::destroy(u32 bufferID)
    {
        TkMutexGuard lock(xaudio::s_mtx);
        xaudio::Buffer *&aBuffer = xaudio::s_buf[bufferID - 1];
        aBuffer->data.resize(0);
        TkMemory::deletePtr(aBuffer);
        aBuffer = nullptr;
    }

    bool ANativeBuffer::write(u32 bufferID, const byte *samples, const AudioDataInfo &info)
    {
        if (bufferID == 0 || samples == nullptr || info.numSamples == 0 || info.bitDepth == 0 || info.numChannels == 0 || info.numChannels > MAX_INPUT_CHANNELS)
            return false;

        xaudio::s_mtx.lock();
        xaudio::Buffer *aBuffer = xaudio::s_buf[bufferID - 1];
        xaudio::s_mtx.unlock();
        if (aBuffer == nullptr)
            return false;

        const u32 samplesLength = info.numSamples * info.bitDepth / 8;

        aBuffer->info = info;
        aBuffer->data.set(samples, samplesLength);
        return true;
    }

    const char *AudioSystem::getName()
    {
        return "XAudio2";
    }

    AudioFeatureFlags AudioSystem::getFeatures()
    {
        return AudioFeatureFlags::kNone;
    }

    void AudioSystem::onActiveDeviceChanged() {}

    void AudioSystem::setDopplerFactor(f32 value)
    {
        xaudio::s_settings.dopplerFactor = value;
        xaudio::markAllDirty();
    }

    void AudioSystem::setVolume(f32 value)
    {
        if (xaudio::s_masteringVoice)
        {
            xaudio::s_settings.volume = 1.0f; // Volume is applied via MasteringVoice
            const HRESULT hr = xaudio::s_masteringVoice->SetVolume(value);
            AUDIO_ERROR_CHECK(SetVolume);
        }
    }

    bool AudioSystem::init()
    {
        auto &devices = Audio::Devices;

        const HRESULT comHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(comHr) && comHr != RPC_E_CHANGED_MODE)
        {
            LogError("Failed to initialize COM for XAudio2. Error: 0x%x", (u32)comHr);
            return false;
        }
        xaudio::s_comInitialized = (comHr == S_OK || comHr == S_FALSE);

        // Initialize XAudio backend
        HRESULT hr = XAudio2Create(&xaudio::s_instance, 0, XAUDIO2_DEFAULT_PROCESSOR);
        if (FAILED(hr))
        {
            LogError("Failed to initalize XAudio2. Error: 0x%x", (u32)hr);
            return false;
        }
        xaudio::s_instance->RegisterForCallbacks(&xaudio::s_callback);

        // Initialize master voice
        hr = xaudio::s_instance->CreateMasteringVoice(&xaudio::s_masteringVoice);
        if (FAILED(hr))
        {
            LogError("Failed to initalize XAudio2 mastering voice. Error: 0x%x", (u32)hr);
            return false;
        }
        XAUDIO2_VOICE_DETAILS details;
        xaudio::s_masteringVoice->GetVoiceDetails(&details);
#if MAX_OUTPUT_CHANNELS > 2
        // TODO: implement multi-channel support (eg. 5.1, 7.1)
        xaudio::s_channels = details.InputChannels;
        hr = xaudio::s_masteringVoice->GetChannelMask(&xaudio::s_channelMask);
        if (FAILED(hr))
        {
            LogError("Failed to get XAudio2 mastering voice channel mask. Error: 0x%x", (u32)hr);
            return false;
        }
#else
        xaudio::s_channels = 2;
        xaudio::s_channelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
#endif
        LogInfo("xaudio: %d channels at %.1f kHz", xaudio::s_channels, details.InputSampleRate / 1000.0f);

        // Dummy device
        devices.resize(1);
        devices[0].name = TEXT("XAudio2 device");
        Audio::setActiveDeviceIdx(0);

        return true;
    }

    void AudioSystem::update()
    {
        // Update dirty voices
        float outputMatrix[MAX_CHANNELS_MATRIX_SIZE];
        for (i32 i = 0; i < xaudio::s_src.count(); i++)
        {
            auto &source = xaudio::s_src[i];
            if (source.isFree() || !(source.isDirty || xaudio::s_forceDirty))
                continue;

            auto mix = SoundMix::calSoundMix(xaudio::s_settings, xaudio::s_listener, source, xaudio::s_channels);
            mix.volumeIntoChannels();
            SoundMix::mapChannels(source.channels, xaudio::s_channels, mix.channels, outputMatrix);

            source.voice->SetFrequencyRatio(mix.pitch);
            source.voice->SetOutputMatrix(xaudio::s_masteringVoice, source.channels, xaudio::s_channels, outputMatrix);

            source.isDirty = false;
        }

        // Clear flag
        xaudio::s_forceDirty = false;
    }

    void AudioSystem::dispose()
    {
        // Cleanup stuff
        if (xaudio::s_masteringVoice)
        {
            xaudio::s_masteringVoice->DestroyVoice();
            xaudio::s_masteringVoice = nullptr;
        }
        if (xaudio::s_instance)
        {
            xaudio::s_instance->StopEngine();
            xaudio::s_instance->Release();
            xaudio::s_instance = nullptr;
        }
        if (xaudio::s_comInitialized)
        {
            CoUninitialize();
            xaudio::s_comInitialized = false;
        }
    }
}

#endif
