/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/common.hpp>
#include <toolkit/collection/DataCollection.hpp>

namespace toolkit
{
    // Interface used for implementations that encodes set of PCM samples into a target audio format.
    class AudioEncoder
    {
    public:
        virtual ~AudioEncoder() = default;

    public:
        // Convert the input PCM samples buffer into the encoder audio format.
        virtual bool convert(byte *samples, AudioDataInfo &info, ByteCollection &result, float quality = 0.5f) = 0;
    public:
        static AudioEncoder *createEncoder(AudioCodecs codec);
        static void destroyEncoder(AudioEncoder *encoder);
    };

}