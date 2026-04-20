/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"
#include "toolkit/types/StringView.hpp"

using namespace audio_test;

TEST(WavDecoderTest, KnownSampleIsValidAndDecodesExactly)
{
    FileIStream *stream = FileIStream::open(audioDocPath("audio_known_16bit_mono.wav"));
    ASSERT_NE(stream, nullptr);

    WavDecoder decoder;
    EXPECT_TRUE(decoder.isValid(stream));

    AudioDataInfo info{};
    ASSERT_TRUE(decoder.open(stream, info));
    EXPECT_EQ(info.sampleRate, 8000u);
    EXPECT_EQ(info.numChannels, 1u);
    EXPECT_EQ(info.bitDepth, 16u);
    EXPECT_EQ(info.numSamples, 4u);

    i16 samples[4] = {};
    decoder.seek(0);
    decoder.read(reinterpret_cast<byte *>(samples), 4);
    EXPECT_EQ(samples[0], 0);
    EXPECT_EQ(samples[1], 16384);
    EXPECT_EQ(samples[2], -16384);
    EXPECT_EQ(samples[3], 32767);

    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}

TEST(WavDecoderTest, FactoryConvertMatchesKnownSample)
{
    FileIStream *stream = FileIStream::open(audioDocPath("audio_known_16bit_mono.wav"));
    ASSERT_NE(stream, nullptr);

    AudioDecoder *decoder = AudioDecoder::createDecoder(AudioCodecs::kWav);
    ASSERT_NE(decoder, nullptr);

    AudioDataInfo info{};
    TkArray<byte> pcm;
    ASSERT_TRUE(decoder->convert(stream, info, pcm));
    EXPECT_EQ(info.sampleRate, 8000u);
    EXPECT_EQ(info.numChannels, 1u);
    EXPECT_EQ(info.bitDepth, 16u);
    EXPECT_EQ(info.numSamples, 4u);
    EXPECT_EQ(pcm.count(), 8);

    const auto *samples = reinterpret_cast<const i16 *>(pcm.get());
    EXPECT_EQ(samples[0], 0);
    EXPECT_EQ(samples[1], 16384);
    EXPECT_EQ(samples[2], -16384);
    EXPECT_EQ(samples[3], 32767);

    AudioDecoder::destroyDecoder(decoder);
    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}

TEST(WavDecoderTest, InvalidFileIsRejected)
{
    FileIStream *stream = FileIStream::open(audioDocPath("audio_invalid_magic.bin"));
    ASSERT_NE(stream, nullptr);

    WavDecoder decoder;
    EXPECT_FALSE(decoder.isValid(stream));

    AudioDataInfo info{};
    EXPECT_FALSE(decoder.open(stream, info));

    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}

TEST(WavDecoderTest, PublicSampleLoadsFromDocData)
{
    FileIStream *stream = FileIStream::open(audioDocPath("audio_noise_tone.wav"));
    ASSERT_NE(stream, nullptr);

    WavDecoder decoder;
    EXPECT_TRUE(decoder.isValid(stream));

    AudioDataInfo info{};
    ASSERT_TRUE(decoder.open(stream, info));
    EXPECT_GT(info.sampleRate, 0u);
    EXPECT_GT(info.numChannels, 0u);
    EXPECT_GT(info.numSamples, 0u);
    EXPECT_EQ(info.bitDepth, 16u);

    TkMemory::deletePtr(TkMemory::getDefault(), stream);
}
