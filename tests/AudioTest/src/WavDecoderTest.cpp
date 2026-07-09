/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"
#include "toolkit/types/StringView.hpp"

using namespace audio_test;

namespace
{
    void addU16(TkArray<byte> &bytes, u16 value)
    {
        bytes.add(static_cast<byte>(value & 0xff));
        bytes.add(static_cast<byte>((value >> 8) & 0xff));
    }

    void addU32(TkArray<byte> &bytes, u32 value)
    {
        bytes.add(static_cast<byte>(value & 0xff));
        bytes.add(static_cast<byte>((value >> 8) & 0xff));
        bytes.add(static_cast<byte>((value >> 16) & 0xff));
        bytes.add(static_cast<byte>((value >> 24) & 0xff));
    }

    void addFourCC(TkArray<byte> &bytes, const char (&id)[5])
    {
        bytes.add(static_cast<byte>(id[0]));
        bytes.add(static_cast<byte>(id[1]));
        bytes.add(static_cast<byte>(id[2]));
        bytes.add(static_cast<byte>(id[3]));
    }

    TkArray<byte> makeTinyWavWithOptionalUnknownChunk(bool oddUnknownChunk)
    {
        TkArray<byte> bytes;
        addFourCC(bytes, "RIFF");
        addU32(bytes, oddUnknownChunk ? 43 : 40);
        addFourCC(bytes, "WAVE");

        if (oddUnknownChunk)
        {
            addFourCC(bytes, "JUNK");
            addU32(bytes, 1);
            bytes.add(0xaa);
            bytes.add(0x00);
        }

        addFourCC(bytes, "fmt ");
        addU32(bytes, 16);
        addU16(bytes, 1);
        addU16(bytes, 1);
        addU32(bytes, 8000);
        addU32(bytes, 16000);
        addU16(bytes, 2);
        addU16(bytes, 16);

        addFourCC(bytes, "data");
        addU32(bytes, 4);
        addU16(bytes, 123);
        addU16(bytes, 456);
        return bytes;
    }
}

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

TEST(WavDecoderTest, IsValidPreservesStreamPosition)
{
    TkArray<byte> wav = makeTinyWavWithOptionalUnknownChunk(false);
    MemoryIStream stream(wav);
    stream.setPos(5);

    WavDecoder decoder;
    EXPECT_TRUE(decoder.isValid(&stream));
    EXPECT_EQ(stream.pos(), 5u);
}

TEST(WavDecoderTest, UnknownOddSizedChunkPaddingIsSkipped)
{
    TkArray<byte> wav = makeTinyWavWithOptionalUnknownChunk(true);
    MemoryIStream stream(wav);

    WavDecoder decoder;
    AudioDataInfo info{};
    ASSERT_TRUE(decoder.open(&stream, info));
    EXPECT_EQ(info.numSamples, 2u);

    i16 samples[2] = {};
    EXPECT_EQ(decoder.readSamples(reinterpret_cast<byte *>(samples), 2), 2u);
    EXPECT_EQ(samples[0], 123);
    EXPECT_EQ(samples[1], 456);
}

TEST(WavDecoderTest, TruncatedReadZeroFillsAndReportsActualSamples)
{
    TkArray<byte> wav = makeTinyWavWithOptionalUnknownChunk(false);
    wav.resize(wav.count() - 2);
    MemoryIStream stream(wav);

    WavDecoder decoder;
    AudioDataInfo info{};
    EXPECT_FALSE(decoder.open(&stream, info));
}
