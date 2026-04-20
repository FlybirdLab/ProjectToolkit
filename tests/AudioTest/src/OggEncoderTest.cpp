/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#if TK_CODECS_VORBIS
using namespace audio_test;

TEST(OggEncoderTest, FactoryAndRoundTripEncodingWork)
{
    AudioEncoder *encoder = AudioEncoder::createEncoder(AudioCodecs::kOgg);
    ASSERT_NE(encoder, nullptr);

    AudioDataInfo info = makeInfo(8, 22050, 2, 16);
    TkArray<byte> pcm = makeStereo16PcmBytes();
    ASSERT_EQ(pcm.count(), static_cast<i32>(info.numSamples * (info.bitDepth / 8)));

    ByteCollection encoded;
    ASSERT_TRUE(encoder->convert(pcm.get(), info, encoded, 0.5f));
    EXPECT_GT(encoded.length(), 0);

    MemoryIStream stream(encoded.get(), static_cast<u32>(encoded.length()));

    OggVorbisDecoder decoder;
    AudioDataInfo decodedInfo{};
    ASSERT_TRUE(decoder.open(&stream, decodedInfo));
    EXPECT_EQ(decodedInfo.sampleRate, info.sampleRate);
    EXPECT_EQ(decodedInfo.numChannels, info.numChannels);
    EXPECT_EQ(decodedInfo.bitDepth, 16u);
    EXPECT_GT(decodedInfo.numSamples, 0u);

    AudioEncoder::destroyEncoder(encoder);
}

#endif