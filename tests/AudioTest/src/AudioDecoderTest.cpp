/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

using namespace audio_test;

TEST(AudioDecoderTest, FactoryCreatesSupportedDecoders)
{
    AudioDecoder *mp3 = AudioDecoder::createDecoder(AudioCodecs::kMp3);
    AudioDecoder *wav = AudioDecoder::createDecoder(AudioCodecs::kWav);

    ASSERT_NE(mp3, nullptr);
    ASSERT_NE(wav, nullptr);

#if TK_CODECS_M4A
    AudioDecoder *m4a = AudioDecoder::createDecoder(AudioCodecs::kM4a);
    ASSERT_NE(m4a, nullptr);
    AudioDecoder::destroyDecoder(m4a);
#else
    EXPECT_EQ(AudioDecoder::createDecoder(AudioCodecs::kM4a), nullptr);
#endif

#if TK_CODECS_VORBIS
    AudioDecoder *ogg = AudioDecoder::createDecoder(AudioCodecs::kOgg);
    ASSERT_NE(ogg, nullptr);
    AudioDecoder::destroyDecoder(ogg);
#else
    EXPECT_EQ(AudioDecoder::createDecoder(AudioCodecs::kOgg), nullptr);
#endif

    AudioDecoder::destroyDecoder(mp3);
    AudioDecoder::destroyDecoder(wav);
}

TEST(AudioDecoderTest, FactoryRejectsUnsupportedCodec)
{
    EXPECT_EQ(AudioDecoder::createDecoder(AudioCodecs::kUnknown), nullptr);
}
