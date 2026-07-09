/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/AudioDecoder.hpp>

#if TK_CODECS_M4A && TK_WINDOWS
struct IMFSourceReader;
#endif

namespace toolkit
{
    // Decodes AAC audio stored in an M4A/MP4 container into 16-bit PCM.
    class M4aDecoder final : public AudioDecoder
    {
    public:
        ~M4aDecoder() override;

        bool open(ITkStream *stream, AudioDataInfo &info, u32 offset = 0) override;
        void seek(u32 offset) override;
        void read(byte *samples, u32 numSamples) override;
        u32 readSamples(byte *samples, u32 numSamples) override;
        bool isValid(ITkStream *stream, u32 offset = 0) override;
        bool isEOF() const override;

    private:
        void close();

#if TK_CODECS_M4A && TK_WINDOWS
        IMFSourceReader *m_reader = nullptr;
        TkArray<byte> m_pendingData{};
        AudioDataInfo m_info{};
        u32 m_pendingOffset = 0;
        u32 m_readSampleOffset = 0;
        bool m_mediaFoundationStarted = false;
#endif
    };
}
