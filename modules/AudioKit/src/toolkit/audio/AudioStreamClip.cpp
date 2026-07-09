/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/types/StringView.hpp"
#include <toolkit/audio/AudioStreamClip.hpp>

#include <toolkit/audio/AudioSystem.hpp>
#include <toolkit/audio/AudioTool.hpp>
#include <toolkit/base/Math.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/io/FileStream.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/threading/Sync.hpp>

namespace toolkit
{
    // -------------------------------------------------------------------------
    // StreamDecodeTask
    // -------------------------------------------------------------------------

    AudioStreamClip::StreamDecodeTask::StreamDecodeTask(AudioStreamClip *clip)
        : m_clip(clip)
    {
    }

    bool AudioStreamClip::StreamDecodeTask::run()
    {
        if (m_clip == nullptr)
            return false;

        TkMutexGuard guard(m_clip->m_mutex);
        for (i32 i = 0; i < m_clip->m_streamingQueue.count(); ++i)
        {
            if (m_clip->decodeChunk(m_clip->m_streamingQueue[i]))
                return true;
        }
        return false;
    }

    void AudioStreamClip::StreamDecodeTask::onEnd()
    {
        if (m_clip != nullptr)
        {
            TkMutexGuard guard(m_clip->m_mutex);
            if (m_clip->m_streamingTask == this)
                m_clip->m_streamingTask = nullptr;
        }
        m_clip = nullptr;
        ThreadPoolTask::onEnd();
    }

    // -------------------------------------------------------------------------
    // AudioStreamClip
    // -------------------------------------------------------------------------

    AudioStreamClip::~AudioStreamClip()
    {
        cancelStreamingTask();
        unload();
    }

    const AudioStreamClip::Descriptor &AudioStreamClip::getDescriptor() const
    {
        return m_descriptor;
    }

    void AudioStreamClip::setDescriptor(const Descriptor &descriptor)
    {
        cancelStreamingTask();
        {
            TkMutexGuard guard(m_mutex);
            if (m_isLoaded)
            {
                releaseNativeBuffers();
                m_isLoaded = false;
            }
            m_descriptor = descriptor;
        }
    }

    const AudioDataInfo &AudioStreamClip::getInfo() const
    {
        return m_info;
    }

    bool AudioStreamClip::is3D() const
    {
        return m_descriptor.is3D;
    }

    f32 AudioStreamClip::getLength() const
    {
        return m_info.length();
    }

    f32 AudioStreamClip::getBufferStartTime(i32 chunkIndex) const
    {
        if (m_info.numChannels == 0 || m_info.sampleRate == 0 || m_chunkSampleCount == 0)
            return 0.0f;
        const f32 framesPerChunk = static_cast<f32>(m_chunkSampleCount / m_info.numChannels);
        return static_cast<f32>(chunkIndex) * framesPerChunk / static_cast<f32>(m_info.sampleRate);
    }

    i32 AudioStreamClip::getFirstBufferIndex(f32 time, f32 &offset) const
    {
        if (m_totalChunks <= 0 || m_chunkSampleCount == 0 || m_info.numChannels == 0 || m_info.sampleRate == 0)
        {
            offset = 0.0f;
            return 0;
        }

        const f32 framesPerChunk = static_cast<f32>(m_chunkSampleCount / m_info.numChannels);
        const f32 chunkDuration  = framesPerChunk / static_cast<f32>(m_info.sampleRate);

        time = TkMath::clamp(time, 0.0f, getLength());
        const i32 chunkIndex = TkMath::clamp(static_cast<i32>(time / chunkDuration), 0, m_totalChunks - 1);
        offset = time - static_cast<f32>(chunkIndex) * chunkDuration;
        return chunkIndex;
    }

    bool AudioStreamClip::isChunkLoaded(i32 virtualChunkIndex) const
    {
        if (virtualChunkIndex < 0 || virtualChunkIndex >= m_totalChunks)
            return false;
        return m_slotChunk[virtualChunkIndex % AUDIO_STREAM_RING_SIZE] == virtualChunkIndex;
    }

    u32 AudioStreamClip::getChunkBufferId(i32 virtualChunkIndex) const
    {
        if (virtualChunkIndex < 0 || virtualChunkIndex >= m_totalChunks)
            return 0;
        return m_physicalBuffers[virtualChunkIndex % AUDIO_STREAM_RING_SIZE];
    }

    bool AudioStreamClip::load()
    {
        TkMutexGuard guard(m_mutex);
        if (m_isLoaded)
            return false;

        if (m_descriptor.filePath.isEmpty())
        {
            LogWarn("AudioStreamClip cannot load: no file path set.");
            return true;
        }

        // Open the file stream and keep it alive for the lifetime of this clip.
        m_stream = FileIStream::open(m_descriptor.filePath.getText());
        if (m_stream == nullptr)
        {
            LogWarn("AudioStreamClip failed to open file: %s", m_descriptor.filePath.getText());
            return true;
        }

        // Create decoder and parse header to fill AudioDataInfo.
        m_decoder = AudioDecoder::createDecoder(m_descriptor.codec);
        if (m_decoder == nullptr)
        {
            LogWarn("AudioStreamClip failed to create decoder for codec %i.", static_cast<i32>(m_descriptor.codec));
            m_stream->close();
            TkMemory::deletePtr(TkMemory::getDefault(), m_stream);
            m_stream = nullptr;
            return true;
        }

        AudioDataInfo info{};
        if (!m_decoder->open(m_stream, info))
        {
            LogWarn("AudioStreamClip failed to parse audio header.");
            AudioDecoder::destroyDecoder(m_decoder);
            m_decoder = nullptr;
            m_stream->close();
            TkMemory::deletePtr(TkMemory::getDefault(), m_stream);
            m_stream = nullptr;
            return true;
        }

        if (info.sampleRate == 0 || info.numChannels == 0 || info.bitDepth == 0 || info.numSamples == 0)
        {
            LogWarn("AudioStreamClip got invalid audio info from decoder.");
            AudioDecoder::destroyDecoder(m_decoder);
            m_decoder = nullptr;
            m_stream->close();
            TkMemory::deletePtr(TkMemory::getDefault(), m_stream);
            m_stream = nullptr;
            return true;
        }

        m_info = info;

        // Compute chunk dimensions.
        const u32 chunkDurationMs = TkMath::max(m_descriptor.chunkDurationMs, 100u);
        const u32 framesPerChunk  = (m_info.sampleRate * chunkDurationMs) / 1000u;
        m_chunkSampleCount        = framesPerChunk * m_info.numChannels;
        const u32 totalFrames     = m_info.numSamples / m_info.numChannels;
        m_totalChunks             = static_cast<i32>((totalFrames + framesPerChunk - 1) / framesPerChunk);
        if (m_totalChunks <= 0)
            m_totalChunks = 1;

        // Allocate physical ring-buffer slots.
        for (i32 i = 0; i < AUDIO_STREAM_RING_SIZE; ++i)
        {
            m_physicalBuffers[i] = ANativeBuffer::create();
            if (m_physicalBuffers[i] == 0)
            {
                LogWarn("AudioStreamClip failed to create native audio buffer (slot %i).", i);
                for (i32 j = 0; j < i; ++j)
                    ANativeBuffer::destroy(m_physicalBuffers[j]);
                TkMemory::memoryClear(m_physicalBuffers, sizeof(m_physicalBuffers));
                AudioDecoder::destroyDecoder(m_decoder);
                m_decoder = nullptr;
                m_stream->close();
                TkMemory::deletePtr(TkMemory::getDefault(), m_stream);
                m_stream = nullptr;
                return true;
            }
            m_slotChunk[i] = -1;
        }

        // Expose physical buffer IDs via the IAudioClip interface array.
        m_buffers.clear();
        m_buffers.resize(AUDIO_STREAM_RING_SIZE);
        for (i32 i = 0; i < AUDIO_STREAM_RING_SIZE; ++i)
            m_buffers[i] = m_physicalBuffers[i];

        m_isLoaded = true;
        return false;
    }

    void AudioStreamClip::unload()
    {
        TkMutexGuard guard(m_mutex);
        releaseNativeBuffers();

        if (m_decoder != nullptr)
        {
            AudioDecoder::destroyDecoder(m_decoder);
            m_decoder = nullptr;
        }

        if (m_stream != nullptr)
        {
            m_stream->close();
            TkMemory::deletePtr(TkMemory::getDefault(), m_stream);
            m_stream = nullptr;
        }

        m_streamingQueue.clear();
        m_isLoaded = false;
    }

    void AudioStreamClip::cancelStreaming()
    {
        cancelStreamingTask();
    }

    bool AudioStreamClip::canBeUpdated() const
    {
        return m_isLoaded && m_streamingTask == nullptr;
    }

    TkTask *AudioStreamClip::createStreamingTask(i32 requestedChunks)
    {
        TkMutexGuard guard(m_mutex);
        if (requestedChunks <= 0 || m_streamingTask != nullptr || !m_isLoaded)
            return nullptr;

        m_streamingTask = TkMemory::newObject<StreamDecodeTask>(TkMemory::getDefault(), this);
        return m_streamingTask;
    }

    bool AudioStreamClip::decodeChunk(i32 virtualChunk)
    {
        if (m_decoder == nullptr || m_stream == nullptr)
            return true;
        if (virtualChunk < 0 || virtualChunk >= m_totalChunks)
            return true;

        const i32  slot         = virtualChunk % AUDIO_STREAM_RING_SIZE;
        const u64  sampleOffset = static_cast<u64>(virtualChunk) * m_chunkSampleCount;

        if (sampleOffset > static_cast<u64>(kMaxU32))
            return true;

        m_decoder->seek(static_cast<u32>(sampleOffset));

        const u64 totalSamples = static_cast<u64>(m_info.numSamples);
        const u64 remaining64  = (sampleOffset < totalSamples) ? (totalSamples - sampleOffset) : 0;
        if (remaining64 == 0)
            return true;

        const u32 samplesToRead = static_cast<u32>(TkMath::min(remaining64, static_cast<u64>(m_chunkSampleCount)));
        const u32 bytesPerSample = m_info.bitDepth / 8;
        if (bytesPerSample == 0 || samplesToRead == 0)
            return true;

        TkArray<byte> tempBuffer;
        tempBuffer.resize(static_cast<i32>(samplesToRead * bytesPerSample));

        const u32 decoded = m_decoder->readSamples(tempBuffer.get(), samplesToRead);
        if (decoded == 0)
            return true;

        if (decoded < samplesToRead)
            tempBuffer.resize(static_cast<i32>(decoded * bytesPerSample));

        AudioDataInfo chunkInfo = m_info;
        chunkInfo.numSamples    = decoded;
        const byte *uploadData  = tempBuffer.get();

        // Convert to mono for 3D audio if the backend does not support spatial multi-channel
        TkArray<byte> monoData;
        if (chunkInfo.numChannels > 1 && m_descriptor.is3D &&
            (AudioSystem::getFeatures() != AudioFeatureFlags::kSpatialMultiChannel))
        {
            const u32 frameCount = decoded / chunkInfo.numChannels;
            if (frameCount == 0)
                return true;
            monoData.resize(static_cast<i32>(frameCount * bytesPerSample));
            AudioTool::convertToMono(uploadData, monoData.get(), chunkInfo.bitDepth, frameCount, chunkInfo.numChannels);
            uploadData = monoData.get();
            chunkInfo.numChannels = 1;
            chunkInfo.numSamples  = frameCount;
        }

        if (!ANativeBuffer::write(m_physicalBuffers[slot], uploadData, chunkInfo))
        {
            LogWarn("AudioStreamClip failed to upload native buffer for virtual chunk %i (slot %i).", virtualChunk, slot);
            return true;
        }

        m_slotChunk[slot] = virtualChunk;
        return false;
    }

    void AudioStreamClip::cancelStreamingTask()
    {
        StreamDecodeTask *task = nullptr;
        {
            TkMutexGuard guard(m_mutex);
            task = m_streamingTask;
            m_streamingTask = nullptr;
        }
        if (task != nullptr)
            task->cancel();
    }

    void AudioStreamClip::releaseNativeBuffers()
    {
        for (i32 i = 0; i < AUDIO_STREAM_RING_SIZE; ++i)
        {
            if (m_physicalBuffers[i] != 0)
            {
                ANativeBuffer::destroy(m_physicalBuffers[i]);
                m_physicalBuffers[i] = 0;
            }
            m_slotChunk[i] = -1;
        }
        m_buffers.clear();
        m_totalChunks      = 0;
        m_chunkSampleCount = 0;
    }
}
