/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

using namespace audio_test;

TEST(AudioEncoderTest, FactoryCreatesSupportedEncoder)
{
#if TK_CODECS_VORBIS
    AudioEncoder *encoder = AudioEncoder::createEncoder(AudioCodecs::kOgg);
    ASSERT_NE(encoder, nullptr);
    AudioEncoder::destroyEncoder(encoder);
#else
    EXPECT_EQ(AudioEncoder::createEncoder(AudioCodecs::kOgg), nullptr);
#endif
}

TEST(AudioEncoderTest, FactoryRejectsUnsupportedCodec)
{
    EXPECT_EQ(AudioEncoder::createEncoder(AudioCodecs::kWav), nullptr);
    EXPECT_EQ(AudioEncoder::createEncoder(AudioCodecs::kMp3), nullptr);
    EXPECT_EQ(AudioEncoder::createEncoder(AudioCodecs::kUnknown), nullptr);
}
