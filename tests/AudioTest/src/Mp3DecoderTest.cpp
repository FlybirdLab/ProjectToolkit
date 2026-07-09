/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"
#include "toolkit/types/StringView.hpp"

using namespace audio_test;

TEST(Mp3DecoderTest, SampleIsValidAndConverts)
{
    FileIStream *stream = FileIStream::open(audioDocPath("audio_tone.mp3"));
    ASSERT_NE(stream, nullptr);

    MP3Decoder decoder;
    EXPECT_TRUE(decoder.isValid(stream));

    AudioDataInfo info{};
    ASSERT_TRUE(decoder.open(stream, info));
    EXPECT_GT(info.sampleRate, 0u);
    EXPECT_GT(info.numChannels, 0u);
    EXPECT_GT(info.numSamples, 0u);
    EXPECT_EQ(info.bitDepth, 16u);

    TkArray<byte> pcm;
    ASSERT_TRUE(decoder.convert(stream, info, pcm));
    EXPECT_EQ(info.bitDepth, 16u);
    EXPECT_GT(pcm.count(), 0);

    AudioDecoder *factoryDecoder = AudioDecoder::createDecoder(AudioCodecs::kMp3);
    ASSERT_NE(factoryDecoder, nullptr);

    AudioDataInfo factoryInfo{};
    TkArray<byte> factoryPcm;
    ASSERT_TRUE(factoryDecoder->convert(stream, factoryInfo, factoryPcm));
    EXPECT_EQ(factoryInfo.sampleRate, info.sampleRate);
    EXPECT_EQ(factoryInfo.numChannels, info.numChannels);
    EXPECT_EQ(factoryInfo.bitDepth, 16u);
    EXPECT_GT(factoryPcm.count(), 0);

    AudioDecoder::destroyDecoder(factoryDecoder);
    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}

TEST(Mp3DecoderTest, InvalidFileIsRejected)
{
    FileIStream *stream = FileIStream::open(audioDocPath("audio_invalid_magic.bin"));
    ASSERT_NE(stream, nullptr);

    MP3Decoder decoder;
    EXPECT_FALSE(decoder.isValid(stream));

    AudioDataInfo info{};
    EXPECT_FALSE(decoder.open(stream, info));

    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}

