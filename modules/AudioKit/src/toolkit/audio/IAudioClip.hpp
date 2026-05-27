/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/common.hpp>
#include <toolkit/collection/StaticArray.hpp>
#include <toolkit/threading/Task.hpp>

namespace toolkit
{
    // Common interface shared by AudioClip (short, pre-loaded) and AudioStreamClip (long, streamed).
    // AudioSource operates on this interface so the same source can drive either clip type.
    class TK_API IAudioClip
    {
    public:
        virtual ~IAudioClip() = default;

        // Metadata
        virtual const AudioDataInfo &getInfo() const = 0;
        virtual bool is3D() const = 0;
        virtual bool isStreamable() const = 0;
        virtual bool isLoaded() const = 0;
        virtual f32 getLength() const = 0;
        virtual i32 getTotalChunks() const = 0;
        virtual f32 getBufferStartTime(i32 chunkIndex) const = 0;
        virtual i32 getFirstBufferIndex(f32 time, f32 &offset) const = 0;

        // Raw buffer array — used by AudioSource for non-streaming play and isEmpty checks.
        // For streaming clips this array holds only the physical ring-buffer slots.
        virtual const TkStaticArray<u32, AUDIO_CLIP_MAX_CHUNKS> &getBuffers() const = 0;

        // Mutable streaming queue — AudioSource writes virtual chunk indices to request.
        virtual TkStaticArray<i32, AUDIO_CLIP_MAX_CHUNKS> &getStreamingQueue() = 0;
        virtual const TkStaticArray<i32, AUDIO_CLIP_MAX_CHUNKS> &getStreamingQueue() const = 0;

        // Ring-buffer safe accessors — use these instead of getBuffers()[virtualIdx].
        // For AudioClip: maps directly to the chunk buffer array.
        // For AudioStreamClip: maps through virtualIdx % ringSize.
        virtual bool isChunkLoaded(i32 virtualChunkIndex) const = 0;
        virtual u32 getChunkBufferId(i32 virtualChunkIndex) const = 0;

        // Lifecycle
        virtual bool load() = 0;
        virtual void unload() = 0;

        // Streaming control
        virtual void cancelStreaming() = 0;
        virtual bool canBeUpdated() const = 0;
        virtual bool isStreamingTaskActive() const = 0;
        virtual TkTask *createStreamingTask(i32 requestedChunks) = 0;
    };
}
