/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/IAudioClip.hpp>
#include <toolkit/audio/AudioDecoder.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/StaticArray.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/threading/Task.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit
{
    class ITkStream;

    // Audio clip for long-form audio (music, ambient, etc.) that streams directly from a file.
    // Unlike AudioClip (which loads all PCM into RAM), AudioStreamClip keeps only
    // AUDIO_STREAM_RING_SIZE physical native buffers in memory at any time.
    // Attach to an AudioSource via setClip() — the same way as AudioClip.
    class TK_API AudioStreamClip : public IAudioClip
    {
    public:
        struct Descriptor
        {
            AudioCodecs codec = AudioCodecs::kWav;
            TkString    filePath;
            bool        is3D = false;
            u32         chunkDurationMs = 1000; // PCM window per chunk in milliseconds
        };

        class StreamDecodeTask final : public ThreadPoolTask
        {
        public:
            explicit StreamDecodeTask(AudioStreamClip *clip);

        protected:
            bool run() override;
            void onEnd() override;

        private:
            AudioStreamClip *m_clip = nullptr;
        };

    public:
        AudioStreamClip() = default;
        ~AudioStreamClip();

        const Descriptor &getDescriptor() const;
        void setDescriptor(const Descriptor &descriptor);

        // IAudioClip
        const AudioDataInfo &getInfo() const override;
        bool is3D() const override;
        bool isStreamable() const override { return true; }
        bool isLoaded() const override { return m_isLoaded; }
        f32 getLength() const override;
        i32 getTotalChunks() const override { return m_totalChunks; }
        f32 getBufferStartTime(i32 chunkIndex) const override;
        i32 getFirstBufferIndex(f32 time, f32 &offset) const override;

        bool isChunkLoaded(i32 virtualChunkIndex) const override;
        u32  getChunkBufferId(i32 virtualChunkIndex) const override;

        const TkStaticArray<u32, AUDIO_CLIP_MAX_CHUNKS> &getBuffers() const override { return m_buffers; }
        TkStaticArray<i32, AUDIO_CLIP_MAX_CHUNKS> &getStreamingQueue() override { return m_streamingQueue; }
        const TkStaticArray<i32, AUDIO_CLIP_MAX_CHUNKS> &getStreamingQueue() const override { return m_streamingQueue; }

        bool load() override;
        void unload() override;

        void cancelStreaming() override;
        bool canBeUpdated() const override;
        bool isStreamingTaskActive() const override { return m_streamingTask != nullptr; }
        TkTask *createStreamingTask(i32 requestedChunks) override;

    private:
        // Decode one virtual chunk from the open decoder into the ring-buffer slot.
        // Must be called with m_mutex held. Returns true on error.
        bool decodeChunk(i32 virtualChunk);
        void cancelStreamingTask();
        void releaseNativeBuffers();

    private:
        Descriptor    m_descriptor{};
        AudioDataInfo m_info{};

        // Ring buffer — kStreamRingSize physical native audio buffers reused in round-robin
        u32 m_physicalBuffers[AUDIO_STREAM_RING_SIZE]{};
        i32 m_slotChunk[AUDIO_STREAM_RING_SIZE]{};  // which virtual chunk each slot holds (-1 = empty)

        u32 m_chunkSampleCount = 0;   // total samples (all channels) per chunk
        i32 m_totalChunks = 0;        // total virtual chunk count for the full file

        ITkStream    *m_stream = nullptr;
        AudioDecoder *m_decoder = nullptr;

        // IAudioClip interface arrays — m_buffers holds physical IDs (only kStreamRingSize valid entries)
        TkStaticArray<u32, AUDIO_CLIP_MAX_CHUNKS> m_buffers{};
        TkStaticArray<i32, AUDIO_CLIP_MAX_CHUNKS> m_streamingQueue{};

        mutable TkMutex m_mutex{};
        StreamDecodeTask *m_streamingTask = nullptr;
        bool m_isLoaded = false;
    };
}
