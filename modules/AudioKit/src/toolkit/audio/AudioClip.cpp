/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/audio/AudioClip.hpp>

#include <toolkit/audio/AudioSystem.hpp>
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
        if (info.bitDepth == 8)
        {
            for (i32 i = 0; i < info.numSamples; ++i)
            {
                const f32 sample = static_cast<f32>(data[i]);
                result[i] = (sample - 128.0f) / 127.0f;
            }
            return false;
        }

        if (info.bitDepth == 16)
        {
            const auto *samples = reinterpret_cast<const i16 *>(data.get());
            for (i32 i = 0; i < info.numSamples; ++i)
            {
                result[i] = static_cast<f32>(samples[i]) / 32767.0f;
            }
            return false;
        }

        const auto *samples = reinterpret_cast<const i32 *>(data.get());
        for (i32 i = 0; i < info.numSamples; ++i)
        {
            result[i] = static_cast<f32>(samples[i] / 2147483647.0);
        }
        return false;
    }

    static void convertToMono(const TkArray<byte> &data, AudioDataInfo &info, TkArray<byte> &monoData)
    {
        if (info.numChannels <= 1)
            return;

        const u32 bytesPerSample = info.bitDepth / 8;
        const u32 frameCount = info.numSamples / info.numChannels;
        monoData.resize(static_cast<i32>(frameCount * bytesPerSample));

        if (info.bitDepth == 8)
        {
            const auto *src = data.get();
            auto *dst = monoData.get();
            for (u32 frame = 0; frame < frameCount; ++frame)
            {
                u32 sum = 0;
                for (u32 channel = 0; channel < info.numChannels; ++channel)
                {
                    sum += src[frame * info.numChannels + channel];
                }
                dst[frame] = static_cast<byte>(sum / info.numChannels);
            }
        }
        else if (info.bitDepth == 16)
        {
            const auto *src = reinterpret_cast<const i16 *>(data.get());
            auto *dst = reinterpret_cast<i16 *>(monoData.get());
            for (u32 frame = 0; frame < frameCount; ++frame)
            {
                i32 sum = 0;
                for (u32 channel = 0; channel < info.numChannels; ++channel)
                {
                    sum += src[frame * info.numChannels + channel];
                }
                dst[frame] = static_cast<i16>(sum / static_cast<i32>(info.numChannels));
            }
        }
        else if (info.bitDepth == 32)
        {
            const auto *src = reinterpret_cast<const i32 *>(data.get());
            auto *dst = reinterpret_cast<i32 *>(monoData.get());
            for (u32 frame = 0; frame < frameCount; ++frame)
            {
                i64 sum = 0;
                for (u32 channel = 0; channel < info.numChannels; ++channel)
                {
                    sum += src[frame * info.numChannels + channel];
                }
                dst[frame] = static_cast<i32>(sum / static_cast<i64>(info.numChannels));
            }
        }

        info.numChannels = 1;
        info.numSamples = frameCount;
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
        TkMutexGuard guard(m_mutex);
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

        offset = 0.0f;
        return 0;
    }

    bool AudioClip::setChunkData(i32 chunkIndex, const TkSpan<const byte> &chunkData)
    {
        if (chunkIndex < 0 || chunkIndex >= AUDIO_CLIP_MAX_CHUNKS)
            return true;

        TkMutexGuard guard(m_mutex);

        if (m_chunks.count() <= chunkIndex)
            m_chunks.resize(chunkIndex + 1);

        auto &chunk = m_chunks[chunkIndex];
        chunk.resize(chunkData.length());
        if (chunkData.length() > 0)
            TkMemory::memoryCopy(chunk.get(), chunkData.get(), static_cast<psize>(chunkData.length()));

        m_totalChunks = m_chunks.count();
        m_totalChunksSize = 0;
        for (i32 i = 0; i < m_totalChunks; ++i)
            m_totalChunksSize += m_chunks[i].count();

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

        resultData.resize(m_totalChunksSize);
        i32 offset = 0;
        for (i32 i = 0; i < m_totalChunks; ++i)
        {
            const auto &chunk = m_chunks[i];
            if (chunk.count() > 0)
            {
                TkMemory::memoryCopy(resultData.get() + offset, chunk.get(), static_cast<psize>(chunk.count()));
                offset += chunk.count();
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

        releaseBuffers();
        m_buffers.resize(m_totalChunks);
        m_buffers.setAll(0);
        rebuildBufferStartTimes();

        if (!m_descriptor.streamable)
        {
            m_buffers[0] = ANativeBuffer::create();
            if (writeBuffer(0))
                return true;
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
        (void)residency;
        return nullptr;
    }

    TkTask *AudioClip::createStreamingTask(i32 residency)
    {
        TkMutexGuard guard(m_mutex);
        if (!TkMath::isInRange(residency, 0, m_totalChunks) || m_streamingTask != nullptr)
            return nullptr;

        m_streamingTask = TkMemory::newObject<StreamingTask>(TkMemory::getDefault(), this);
        return m_streamingTask;
    }

    void AudioClip::cancelStreamingTasks()
    {
        StreamingTask *task = nullptr;
        {
            TkMutexGuard guard(m_mutex);
            task = m_streamingTask;
        }

        if (task != nullptr)
        {
            task->cancel();
            task->wait();
        }
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

        if (m_buffers[chunkIndex] == 0)
            m_buffers[chunkIndex] = ANativeBuffer::create();

        const auto &chunk = m_chunks[chunkIndex];
        if (chunk.isEmpty())
        {
            LogWarn("AudioClip chunk {0} has no data.", chunkIndex);
            return true;
        }

        AudioDataInfo info = m_descriptor.info;
        TkArray<byte> uploadData(chunk);
        const u32 bytesPerSample = info.bitDepth / 8;
        if (bytesPerSample == 0 || info.numChannels == 0)
            return true;

        info.numSamples = static_cast<u32>(uploadData.count()) / bytesPerSample;

        TkArray<byte> monoData;
        if (info.numChannels > 1 && is3D() && (AudioSystem::getFeatures() != AudioFeatureFlags::kSpatialMultiChannel))
        {
            convertToMono(uploadData, info, monoData);
            uploadData = monoData;
        }

        ANativeBuffer::write(m_buffers[chunkIndex], uploadData.get(), info);
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
