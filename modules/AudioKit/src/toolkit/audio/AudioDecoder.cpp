/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AudioDecoder.hpp"

#include "codecs/Mp3Decoder.hpp"
#include "codecs/OggDecoder.hpp"
#include "codecs/WavDecoder.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/memory/Memory.hpp"

namespace toolkit
{
    AudioDecoder *AudioDecoder::createDecoder(AudioCodecs c)
    {
        switch (c)
        {
        case AudioCodecs::kMp3:
            return TkMemory::newObject<MP3Decoder>();
        case AudioCodecs::kOgg:
#if TK_CODECS_VORBIS
            return TkMemory::newObject<OggVorbisDecoder>();
#else
            LogError("Audio codec not supported for decoding: %d", (int)c);
            return nullptr;
#endif
        case AudioCodecs::kWav:
            return TkMemory::newObject<WavDecoder>();
        default:
            LogError("Unsupported audio codec: %d", (int)c);
            return nullptr;
        }
    }

    void AudioDecoder::destroyDecoder(AudioDecoder *decoder)
    {
        TkMemory::deletePtr(decoder);
    }
}
