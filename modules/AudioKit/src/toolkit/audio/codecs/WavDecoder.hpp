/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/AudioDecoder.hpp>

namespace toolkit
{
    // Decodes .wav audio data into raw PCM format.
    class WavDecoder : public AudioDecoder
    {
    public:
        WavDecoder()
        {
            m_stream = nullptr;
            mFormat = 0;
            mDataOffset = 0;
            mDataSize = 0;
            mBytesPerSample = 0;
        }

    private:
        // Parses the WAVE header and output audio file meta-data. Returns false if the header is not valid.
        bool parseHeader(AudioDataInfo &info);

    public:
        bool open(ITkStream *stream, AudioDataInfo &info, u32 offset = 0) override;
        void seek(u32 offset) override;
        void read(byte *samples, u32 numSamples) override;
        u32 readSamples(byte *samples, u32 numSamples) override;
        bool isValid(ITkStream *stream, u32 offset = 0) override;

    private:
        ITkStream *m_stream;
        u16 mFormat;
        u32 mDataOffset;
        u32 mDataSize;
        u32 mBytesPerSample;
    };

}
