/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/IAudioClip.hpp>
#include <toolkit/base/Math.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/StaticArray.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/threading/Task.hpp>

namespace toolkit
{
    class TK_API AudioClip : public IAudioClip
    {
    public:
        struct Descriptor
        {
            AudioDataInfo info{};
            bool is3D = false;
            bool streamable = false;
        };

        class StreamingTask final : public ThreadPoolTask
        {
        public:
            explicit StreamingTask(AudioClip *clip);

        protected:
            bool run() override;
            void onEnd() override;

        private:
            AudioClip *m_clip = nullptr;
        };

    public:
        AudioClip() = default;
        ~AudioClip();

        const Descriptor &getDescriptor() const;
        void setDescriptor(const Descriptor &descriptor);
        const AudioDataInfo &getInfo() const override;
        bool is3D() const override;
        bool isStreamable() const override;
        bool isLoaded() const override { return m_isLoaded; }
        bool isStreamingTaskActive() const override;
        f32 getLength() const override;
        TK_FORCE_INLINE i32 getTotalChunks() const override
        {
            return m_totalChunks;
        }

        f32 getBufferStartTime(i32 bufferIndex) const override;
        i32 getFirstBufferIndex(f32 time, f32 &offset) const override;

        bool setChunkData(i32 chunkIndex, const TkSpan<const byte> &chunkData);
        void clearChunkData();

        bool extractData(TkArray<byte> &resultData, AudioDataInfo &resultDataInfo) const;
        bool extractDataFloat(TkArray<f32> &resultData, AudioDataInfo &resultDataInfo) const;

        bool load() override;
        void unload() override;

        void cancelStreaming() override;
        i32 getMaxResidency() const;
        i32 getCurrentResidency() const;
        i32 getAllocatedResidency() const;
        bool canBeUpdated() const override;
        TkTask *updateAllocation(i32 residency);
        TkTask *createStreamingTask(i32 requestedChunks) override;
        void cancelStreamingTasks();

        bool isChunkLoaded(i32 virtualChunkIndex) const override
        {
            return virtualChunkIndex >= 0 && virtualChunkIndex < m_buffers.count() && m_buffers[virtualChunkIndex] != 0;
        }

        u32 getChunkBufferId(i32 virtualChunkIndex) const override
        {
            return (virtualChunkIndex >= 0 && virtualChunkIndex < m_buffers.count()) ? m_buffers[virtualChunkIndex] : 0;
        }

        TK_FORCE_INLINE const TkStaticArray<u32, AUDIO_CLIP_MAX_CHUNKS> &getBuffers() const override
        {
            return m_buffers;
        }

        TK_FORCE_INLINE TkStaticArray<i32, AUDIO_CLIP_MAX_CHUNKS> &getStreamingQueue() override
        {
            return m_streamingQueue;
        }

        TK_FORCE_INLINE const TkStaticArray<i32, AUDIO_CLIP_MAX_CHUNKS> &getStreamingQueue() const override
        {
            return m_streamingQueue;
        }

    private:
        void rebuildBufferStartTimes();
        bool writeBuffer(i32 chunkIndex);
        void releaseBuffers();

    private:
        Descriptor m_descriptor{};
        TkStaticArray<u32, AUDIO_CLIP_MAX_CHUNKS> m_buffers{};
        TkStaticArray<i32, AUDIO_CLIP_MAX_CHUNKS> m_streamingQueue{};
        TkStaticArray<TkArray<byte>, AUDIO_CLIP_MAX_CHUNKS> m_chunks{};
        mutable TkMutex m_mutex{};
        StreamingTask *m_streamingTask = nullptr;
        i32 m_totalChunks = 0;
        psize m_totalChunksSize = 0;
        f32 m_bufferStartTimes[AUDIO_CLIP_MAX_CHUNKS + 1]{};
        bool m_isLoaded = false;
    };
}
