/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/audio/AudioClip.hpp>

#include <toolkit/audio/AudioSystem.hpp>
#include <toolkit/audio/AudioTool.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/Span.hpp>
#include <toolkit/collection/StaticArray.hpp>

namespace toolkit
{
    static bool convertPcmToFloat(const TkArray<byte> &data, const AudioDataInfo &info, TkArray<f32> &result)
    {
        if (info.bitDepth != 8 && info.bitDepth != 16 && info.bitDepth != 32)
        {
            LogWarn("AudioClip only supports 8/16/32-bit PCM float conversion right now.");
            return true;
        }

        result.resize(info.numSamples);
        AudioTool::convertToFloat(data.get(), info.bitDepth, result.get(), info.numSamples);
        return false;
    }

    AudioClip::StreamingTask::StreamingTask(AudioClip *clip)
        : m_clip(clip)
    {
    }

    bool AudioClip::StreamingTask::run()
    {
        if (m_clip == nullptr)
            return false;

        TkMutexGuard guard(m_clip->m_mutex);
        for (i32 i = 0; i < m_clip->m_streamingQueue.count(); ++i)
        {
            if (m_clip->writeBuffer(m_clip->m_streamingQueue[i]))
                return true;
        }
        return false;
    }

    void AudioClip::StreamingTask::onEnd()
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

    AudioClip::~AudioClip()
    {
        cancelStreamingTasks();
        unload();
    }

    const AudioClip::Descriptor &AudioClip::getDescriptor() const
    {
        return m_descriptor;
    }

    void AudioClip::setDescriptor(const Descriptor &descriptor)
    {
        cancelStreamingTasks();

        TkMutexGuard guard(m_mutex);
        if (m_isLoaded)
        {
            releaseBuffers();
            m_isLoaded = false;
        }
        m_descriptor = descriptor;
        rebuildBufferStartTimes();
    }

    const AudioDataInfo &AudioClip::getInfo() const
    {
        return m_descriptor.info;
    }

    bool AudioClip::is3D() const
    {
        return m_descriptor.is3D;
    }

    bool AudioClip::isStreamable() const
    {
        return m_descriptor.streamable;
    }

    bool AudioClip::isStreamingTaskActive() const
    {
        return m_streamingTask != nullptr;
    }

    f32 AudioClip::getLength() const
    {
        return m_descriptor.info.length();
    }

    f32 AudioClip::getBufferStartTime(i32 bufferIndex) const
    {
        TK_ASSERT(bufferIndex >= 0 && bufferIndex <= AUDIO_CLIP_MAX_CHUNKS);
        return m_bufferStartTimes[bufferIndex];
    }

    i32 AudioClip::getFirstBufferIndex(f32 time, f32 &offset) const
    {
        if (m_totalChunks <= 0)
        {
            offset = 0.0f;
            return 0;
        }

        time = TkMath::clamp(time, 0.0f, getLength());
        for (i32 i = 0; i < m_totalChunks; ++i)
        {
            if (m_bufferStartTimes[i + 1] > time)
            {
                offset = time - m_bufferStartTimes[i];
                return i;
            }
        }

        const i32 lastChunk = m_totalChunks - 1;
        offset = time - m_bufferStartTimes[lastChunk];
        return lastChunk;
    }

    bool AudioClip::setChunkData(i32 chunkIndex, const TkSpan<const byte> &chunkData)
    {
        if (chunkIndex < 0 || chunkIndex >= AUDIO_CLIP_MAX_CHUNKS)
            return true;

        cancelStreamingTasks();

        TkMutexGuard guard(m_mutex);
        if (m_isLoaded)
        {
            releaseBuffers();
            m_isLoaded = false;
        }

        if (m_chunks.count() <= chunkIndex)
            m_chunks.resize(chunkIndex + 1);

        auto &chunk = m_chunks[chunkIndex];
        chunk.resize(chunkData.length());
        if (chunkData.length() > 0)
            TkMemory::memoryCopy(chunk.get(), chunkData.get(), static_cast<psize>(chunkData.length()));

        m_totalChunks = m_chunks.count();
        m_totalChunksSize = 0;
        for (i32 i = 0; i < m_totalChunks; ++i)
            m_totalChunksSize += static_cast<psize>(m_chunks[i].count());

        rebuildBufferStartTimes();
        return false;
    }

    void AudioClip::clearChunkData()
    {
        cancelStreamingTasks();

        TkMutexGuard guard(m_mutex);
        releaseBuffers();
        m_chunks.clear();
        m_streamingQueue.clear();
        m_totalChunks = 0;
        m_totalChunksSize = 0;
        m_isLoaded = false;
        TkMemory::memoryClear(m_bufferStartTimes, sizeof(m_bufferStartTimes));
    }

    bool AudioClip::extractData(TkArray<byte> &resultData, AudioDataInfo &resultDataInfo) const
    {
        TkMutexGuard guard(m_mutex);
        resultData.clear();

        if (m_totalChunksSize <= 0)
            return false;

        if (m_totalChunksSize > static_cast<psize>(kMaxI32))
        {
            LogWarn("AudioClip data is too large to extract into TkArray.");
            return true;
        }

        resultData.resize(static_cast<i32>(m_totalChunksSize));
        psize offset = 0;
        for (i32 i = 0; i < m_totalChunks; ++i)
        {
            const auto &chunk = m_chunks[i];
            if (chunk.count() > 0)
            {
                TkMemory::memoryCopy(resultData.get() + offset, chunk.get(), static_cast<psize>(chunk.count()));
                offset += static_cast<psize>(chunk.count());
            }
        }

        resultDataInfo = m_descriptor.info;
        return false;
    }

    bool AudioClip::extractDataFloat(TkArray<f32> &resultData, AudioDataInfo &resultDataInfo) const
    {
        TkArray<byte> pcmData;
        if (extractData(pcmData, resultDataInfo))
            return true;

        if (convertPcmToFloat(pcmData, resultDataInfo, resultData))
            return true;

        resultDataInfo.bitDepth = 32;
        return false;
    }

    bool AudioClip::load()
    {
        TkMutexGuard guard(m_mutex);
        if (m_totalChunks <= 0)
            return true;

        if (!m_descriptor.streamable && m_totalChunks != 1)
        {
            LogWarn("Non-streaming AudioClip must contain exactly one chunk.");
            releaseBuffers();
            m_isLoaded = false;
            return true;
        }

        releaseBuffers();
        m_buffers.resize(m_totalChunks);
        m_buffers.setAll(0);
        rebuildBufferStartTimes();

        if (!m_descriptor.streamable)
        {
            m_buffers[0] = ANativeBuffer::create();
            if (writeBuffer(0))
            {
                releaseBuffers();
                m_isLoaded = false;
                return true;
            }
        }

        m_isLoaded = true;
        return false;
    }

    void AudioClip::unload()
    {
        TkMutexGuard guard(m_mutex);
        releaseBuffers();
        m_streamingQueue.clear();
        m_isLoaded = false;
    }

    void AudioClip::cancelStreaming()
    {
        cancelStreamingTasks();
    }

    i32 AudioClip::getMaxResidency() const
    {
        return m_totalChunks;
    }

    i32 AudioClip::getCurrentResidency() const
    {
        return m_buffers.count();
    }

    i32 AudioClip::getAllocatedResidency() const
    {
        return m_buffers.count();
    }

    bool AudioClip::canBeUpdated() const
    {
        return m_totalChunks > 0 && m_streamingTask == nullptr;
    }

    TkTask *AudioClip::updateAllocation(i32 residency)
    {
        // Dynamic residency changes are not implemented yet. Streaming currently
        // uploads chunks requested through createStreamingTask().
        (void)residency;
        return nullptr;
    }

    TkTask *AudioClip::createStreamingTask(i32 requestedChunks)
    {
        TkMutexGuard guard(m_mutex);
        if (!TkMath::isInRange(requestedChunks, 0, m_totalChunks) || m_streamingTask != nullptr)
            return nullptr;

        // The current streaming task consumes the explicit chunk queue built by
        // AudioSource. requestedChunks documents that queue size until dynamic
        // residency management is implemented.
        m_streamingTask = TkMemory::newObject<StreamingTask>(TkMemory::getDefault(), this);
        return m_streamingTask;
    }

    void AudioClip::cancelStreamingTasks()
    {
        StreamingTask *task = nullptr;
        {
            TkMutexGuard guard(m_mutex);
            task = m_streamingTask;
            m_streamingTask = nullptr;
        }

        if (task != nullptr)
            task->cancel();
    }

    void AudioClip::rebuildBufferStartTimes()
    {
        TkMemory::memoryClear(m_bufferStartTimes, sizeof(m_bufferStartTimes));
        if (m_totalChunks <= 0)
            return;

        const auto &info = m_descriptor.info;
        const u32 bytesPerSample = info.bitDepth / 8;
        const u32 bytesPerFrame = bytesPerSample * TkMath::clamp(info.numChannels, 1U, kMaxU32);
        if (bytesPerFrame == 0 || info.sampleRate == 0)
            return;

        for (i32 i = 0; i < m_totalChunks; ++i)
        {
            const u32 frameCount = static_cast<u32>(m_chunks[i].count()) / bytesPerFrame;
            m_bufferStartTimes[i + 1] = m_bufferStartTimes[i] + static_cast<f32>(frameCount) / static_cast<f32>(info.sampleRate);
        }
    }

    bool AudioClip::writeBuffer(i32 chunkIndex)
    {
        if (chunkIndex < 0 || chunkIndex >= m_totalChunks || chunkIndex >= m_buffers.count())
            return true;

        const auto &chunk = m_chunks[chunkIndex];
        if (chunk.isEmpty())
        {
            LogWarn("AudioClip chunk %i has no data.", chunkIndex);
            return true;
        }

        AudioDataInfo info = m_descriptor.info;
        const byte *uploadData = chunk.get();
        const u32 bytesPerSample = info.bitDepth / 8;
        if ((info.bitDepth != 8 && info.bitDepth != 16 && info.bitDepth != 24 && info.bitDepth != 32) || bytesPerSample == 0 || info.numChannels == 0)
            return true;

        if ((chunk.count() % static_cast<i32>(bytesPerSample)) != 0)
            return true;

        info.numSamples = static_cast<u32>(chunk.count()) / bytesPerSample;
        if (info.numSamples == 0 || (info.numSamples % info.numChannels) != 0)
            return true;

        TkArray<byte> monoData;
        if (info.numChannels > 1 && is3D() && (AudioSystem::getFeatures() != AudioFeatureFlags::kSpatialMultiChannel))
        {
            const u32 frameCount = info.numSamples / info.numChannels;
            const u64 monoBytes = static_cast<u64>(frameCount) * bytesPerSample;
            if (monoBytes > static_cast<u64>(kMaxI32))
                return true;
            monoData.resize(static_cast<i32>(monoBytes));
            AudioTool::convertToMono(uploadData, monoData.get(), info.bitDepth, frameCount, info.numChannels);
            uploadData = monoData.get();
            info.numChannels = 1;
            info.numSamples = frameCount;
        }

        bool createdBuffer = false;
        if (m_buffers[chunkIndex] == 0)
        {
            m_buffers[chunkIndex] = ANativeBuffer::create();
            createdBuffer = true;
        }

        if (m_buffers[chunkIndex] == 0)
            return true;

        if (!ANativeBuffer::write(m_buffers[chunkIndex], uploadData, info))
        {
            LogWarn("AudioClip failed to upload native buffer for chunk %i.", chunkIndex);
            if (createdBuffer)
            {
                ANativeBuffer::destroy(m_buffers[chunkIndex]);
                m_buffers[chunkIndex] = 0;
            }
            return true;
        }
        return false;
    }

    void AudioClip::releaseBuffers()
    {
        for (i32 i = 0; i < m_buffers.count(); ++i)
        {
            if (m_buffers[i] != 0)
                ANativeBuffer::destroy(m_buffers[i]);
        }
        m_buffers.clear();
    }
}
