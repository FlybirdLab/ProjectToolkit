/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/common.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit
{
    class ITkStream;

    // Interface used for implementations that parse audio formats into a set of PCM samples.
    class AudioDecoder
    {
    public:
        virtual ~AudioDecoder() = default;

    public:
        // Try to open the specified stream with audio data and loads the whole audio data.
        virtual bool convert(ITkStream *stream, AudioDataInfo &info, TkArray<byte> &result, u32 offset = 0)
        {
            if (!isValid(stream, offset))
                return false;
            if (!open(stream, info, offset))
                return false;

            // Load the whole audio data
            const i32 bytesPerSample = info.bitDepth / 8;
            const i32 bufferSize = info.numSamples * bytesPerSample;
            result.resize(bufferSize);
            read(result.get(), info.numSamples);

            return true;
        }

    public:
        // Try to open the specified stream with audio data. Must be called before any reads or seeks.
        virtual bool open(ITkStream *stream, AudioDataInfo &info, u32 offset = 0) = 0;

        // Move the read pointer to the specified offset. Any further Read() calls will read from this location. User must ensure not to seek past the end of the data.
        virtual void seek(u32 offset) = 0;

        // Read a set of samples from the audio data.
        virtual void read(byte *samples, u32 numSamples) = 0;

        // Check if the data in the provided stream valid audio data for the current format. You should check this before calling Open().
        virtual bool isValid(ITkStream *stream, u32 offset = 0) = 0;

    public:
        static AudioDecoder *createDecoder(AudioCodecs codec);
        static void destroyDecoder(AudioDecoder *decoder);
    };

}
