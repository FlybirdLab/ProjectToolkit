/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AudioEncoder.hpp"
#include "codecs/OggEncorder.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{
    AudioEncoder *AudioEncoder::createEncoder(AudioCodecs codec)
    {
        switch (codec)
        {
        case AudioCodecs::kOgg:
#if TK_CODECS_VORBIS
            return TkMemory::newObject<OggVorbisEncoder>();
#endif
        default:
            LogError("Audio codec not supported for encoding: %d", (int)codec);
            return nullptr;
        }
    }

    void AudioEncoder::destroyEncoder(AudioEncoder *encoder)
    {
        TkMemory::deletePtr(encoder);
    }
}