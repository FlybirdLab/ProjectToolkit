/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/audio/typedef.hpp>
#include <toolkit/audio/environment.hpp>

namespace toolkit
{
    inline static constexpr i32 AUDIO_CLIP_MAX_CHUNKS = 16;
    inline static constexpr i32 AUDIO_STREAM_RING_SIZE = 4;

// The maximum amount of listeners used at once
#define AUDIO_MAX_LISTENERS 1
// The maximum amount of audio emitter buffers
#define AUDIO_MAX_SOURCE_BUFFERS (AUDIO_CLIP_MAX_CHUNKS)

    // Audio data storage format used by the runtime.
    enum class AudioFormat
    {
        kRaw,
        kVorbis,
    };

    enum class AudioCodecs {
        kUnknown = 0,
        kMp3,
        kOgg,
        kWav,
        kM4a
    };

    // Meta-data describing a chunk of audio.
    struct AudioDataInfo
    {
        // The total number of audio samples in the audio data (includes all channels).
        u32 numSamples;
        // The number of audio samples per second, per channel.
        u32 sampleRate;
        // The number of channels. Each channel has its own set of samples.
        u32 numChannels;
        // The number of bits per sample.
        u32 bitDepth;
        // Get the length of the audio data (in seconds).
        f32 length() const;
        f32 GetLength() const;
    };

}
