/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/AudioDecoder.hpp>

#if TK_CODECS_VORBIS
#include <vorbis/vorbisfile.h>

namespace toolkit
{
    // Decode .ogg audio data into raw PCM format.
    class OggVorbisDecoder : public AudioDecoder
    {
    public:
        OggVorbisDecoder()
        {
            stream = nullptr;
            offset = 0;
            channelCount = 0;
            file = {};
        }

        ~OggVorbisDecoder()
        {
            if (file.datasource != nullptr)
                ov_clear(&file);
        }

    public:
        bool open(ITkStream *stream, AudioDataInfo &info, u32 offset = 0) override;
        void seek(u32 offset) override;
        void read(byte *samples, u32 numSamples) override;
        u32 readSamples(byte *samples, u32 numSamples) override;
        bool isValid(ITkStream *stream, u32 offset = 0) override;

    public:
        ITkStream *stream;
        u32 offset;
        u32 channelCount;
        OggVorbis_File file;
    };

}

#endif
