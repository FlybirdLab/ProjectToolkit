/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/AudioDecoder.hpp>
#include <minimp3_ex.h>

namespace toolkit
{

    // Decode .mp3 audio data into raw PCM format.
    class MP3Decoder : public AudioDecoder
    {
    private:
        ITkStream *m_stream = nullptr;
        mp3dec_ex_t m_exd{};
        mp3dec_io_t m_io{};
        struct IOContext
        {
            ITkStream *stream = nullptr;
            u32 baseOffset = 0;
        } m_ioCtx;
        AudioDataInfo m_info{};
        u32 m_readSampleOffset = 0;
        bool m_isOpen = false;

    public:
        MP3Decoder() = default;
        ~MP3Decoder();

    public:
        bool convert(ITkStream *stream, AudioDataInfo &info, TkArray<byte> &result, u32 offset = 0) override;
        bool open(ITkStream *stream, AudioDataInfo &info, u32 offset = 0) override;
        void seek(u32 offset) override;
        void read(byte *samples, u32 numSamples) override;
        u32 readSamples(byte *samples, u32 numSamples) override;
        bool isValid(ITkStream *stream, u32 offset = 0) override;
        bool isEOF() const override;

    private:
        void close();
    };

}
