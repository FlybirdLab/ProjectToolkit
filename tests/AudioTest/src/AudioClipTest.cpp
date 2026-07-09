/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

using namespace audio_test;

namespace
{
    class AudioClipTest : public AudioTestBase
    {
    };
}

TEST_F(AudioClipTest, ChunkDataExtractionAndFloatConversionWork)
{
    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 4, 1, 16), false, false));

    TkArray<byte> chunk0 = packPcm<i16>(-32767, 0);
    TkArray<byte> chunk1 = packPcm<i16>(16384, 32767);
    EXPECT_FALSE(clip.setChunkData(0, asSpan(chunk0)));
    EXPECT_FALSE(clip.setChunkData(1, asSpan(chunk1)));
    EXPECT_TRUE(clip.canBeUpdated());
    EXPECT_EQ(clip.getDescriptor().info.sampleRate, 4u);
    EXPECT_EQ(clip.getInfo().sampleRate, 4u);
    EXPECT_FALSE(clip.is3D());
    EXPECT_FALSE(clip.isStreamable());
    EXPECT_FALSE(clip.isStreamingTaskActive());

    TkArray<byte> pcm;
    AudioDataInfo info{};
    EXPECT_FALSE(clip.extractData(pcm, info));
    EXPECT_EQ(info.sampleRate, 4u);
    EXPECT_EQ(info.numChannels, 1u);
    EXPECT_EQ(info.bitDepth, 16u);
    EXPECT_EQ(info.numSamples, 4u);
    EXPECT_EQ(pcm.count(), 8);

    const auto *samples = reinterpret_cast<const i16 *>(pcm.get());
    EXPECT_EQ(samples[0], -32767);
    EXPECT_EQ(samples[1], 0);
    EXPECT_EQ(samples[2], 16384);
    EXPECT_EQ(samples[3], 32767);

    TkArray<f32> floats;
    AudioDataInfo floatInfo{};
    EXPECT_FALSE(clip.extractDataFloat(floats, floatInfo));
    EXPECT_EQ(floatInfo.bitDepth, 32u);
    EXPECT_EQ(floatInfo.sampleRate, 4u);
    EXPECT_EQ(floatInfo.numChannels, 1u);
    EXPECT_EQ(floats.count(), 4);
    EXPECT_NEAR(floats[0], -1.0f, 1e-6f);
    EXPECT_NEAR(floats[1], 0.0f, 1e-6f);
    EXPECT_NEAR(floats[2], 16384.0f / 32767.0f, 1e-6f);
    EXPECT_NEAR(floats[3], 1.0f, 1e-6f);
}

TEST_F(AudioClipTest, ClearChunkDataResetsStoredSamples)
{
    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 4, 1, 16), false, false));
    EXPECT_FALSE(clip.setChunkData(0, asSpan(packPcm<i16>(-32767, 0))));
    EXPECT_TRUE(clip.canBeUpdated());

    clip.clearChunkData();
    EXPECT_FALSE(clip.canBeUpdated());
    EXPECT_EQ(clip.getMaxResidency(), 0);
    EXPECT_EQ(clip.getCurrentResidency(), 0);
    EXPECT_EQ(clip.getAllocatedResidency(), 0);

    TkArray<byte> data;
    AudioDataInfo info{};
    EXPECT_FALSE(clip.extractData(data, info));
    EXPECT_EQ(data.count(), 0);
}

TEST_F(AudioClipTest, BufferStartTimesAndBufferIndexQueriesAreStable)
{
    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 4, 1, 16), false, false));
    clip.setChunkData(0, asSpan(packPcm<i16>(0, 0)));
    clip.setChunkData(1, asSpan(packPcm<i16>(0, 0)));

    EXPECT_FLOAT_EQ(clip.getLength(), 1.0f);
    EXPECT_FLOAT_EQ(clip.getBufferStartTime(0), 0.0f);
    EXPECT_FLOAT_EQ(clip.getBufferStartTime(1), 0.5f);
    EXPECT_FLOAT_EQ(clip.getBufferStartTime(2), 1.0f);

    f32 offset = -1.0f;
    EXPECT_EQ(clip.getFirstBufferIndex(0.25f, offset), 0);
    EXPECT_FLOAT_EQ(offset, 0.25f);
    EXPECT_EQ(clip.getFirstBufferIndex(0.75f, offset), 1);
    EXPECT_FLOAT_EQ(offset, 0.25f);
    EXPECT_EQ(clip.getFirstBufferIndex(-1.0f, offset), 0);
    EXPECT_FLOAT_EQ(offset, 0.0f);
    EXPECT_EQ(clip.getFirstBufferIndex(1.0f, offset), 1);
    EXPECT_FLOAT_EQ(offset, 0.5f);
    EXPECT_EQ(clip.getFirstBufferIndex(99.0f, offset), 1);
    EXPECT_FLOAT_EQ(offset, 0.5f);
}

TEST_F(AudioClipTest, InvalidChunkIndicesAndEmptyClipsReportFailure)
{
    AudioClip clip;
    AudioDataInfo info{};
    TkArray<byte> data;
    EXPECT_TRUE(clip.setChunkData(-1, asSpan(data)));
    EXPECT_TRUE(clip.setChunkData(AUDIO_CLIP_MAX_CHUNKS, asSpan(data)));
    EXPECT_FALSE(clip.extractData(data, info));
    EXPECT_EQ(data.count(), 0);
    EXPECT_EQ(clip.updateAllocation(0), nullptr);
    EXPECT_FALSE(clip.canBeUpdated());
    clip.cancelStreaming();
    EXPECT_FALSE(clip.isStreamingTaskActive());
}

TEST_F(AudioClipTest, LoadAndUnloadNonStreamableClipUseBuffers)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    if (Audio::Devices.count() == 0)
        GTEST_SKIP() << "No audio devices available";

    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 4, 1, 16), false, false));
    clip.setChunkData(0, asSpan(packPcm<i16>(0, 16384, -16384, 32767)));

    EXPECT_FALSE(clip.load());
    EXPECT_EQ(clip.getBuffers().count(), 1);
    EXPECT_NE(clip.getBuffers()[0], 0u);
    EXPECT_EQ(clip.getMaxResidency(), 1);
    EXPECT_EQ(clip.getCurrentResidency(), 1);
    EXPECT_EQ(clip.getAllocatedResidency(), 1);

    clip.unload();
    EXPECT_TRUE(clip.getBuffers().isEmpty());
}

TEST_F(AudioClipTest, LoadedClipMutationReleasesOldBuffers)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    if (Audio::Devices.count() == 0)
        GTEST_SKIP() << "No audio devices available";

    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(2, 8000, 1, 16), false, false));
    EXPECT_FALSE(clip.setChunkData(0, asSpan(packPcm<i16>(0, 100))));
    ASSERT_FALSE(clip.load());
    ASSERT_FALSE(clip.getBuffers().isEmpty());
    EXPECT_NE(clip.getBuffers()[0], 0u);

    clip.setChunkData(0, asSpan(packPcm<i16>(200, 300)));
    EXPECT_TRUE(clip.getBuffers().isEmpty());

    ASSERT_FALSE(clip.load());
    EXPECT_EQ(clip.getBuffers().count(), 1);
    EXPECT_NE(clip.getBuffers()[0], 0u);

    clip.setDescriptor(makeClipDescriptor(makeInfo(2, 16000, 1, 16), false, false));
    EXPECT_TRUE(clip.getBuffers().isEmpty());
}

TEST_F(AudioClipTest, NonStreamingMultiChunkLoadIsRejected)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    if (Audio::Devices.count() == 0)
        GTEST_SKIP() << "No audio devices available";

    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 8000, 1, 16), false, false));
    EXPECT_FALSE(clip.setChunkData(0, asSpan(packPcm<i16>(0, 1))));
    EXPECT_FALSE(clip.setChunkData(1, asSpan(packPcm<i16>(2, 3))));

    EXPECT_TRUE(clip.load());
    EXPECT_TRUE(clip.getBuffers().isEmpty());
}

TEST_F(AudioClipTest, LoadStreamableClipKeepsAllChunksAllocated)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    if (Audio::Devices.count() == 0)
        GTEST_SKIP() << "No audio devices available";

    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 4, 1, 16), false, true));
    clip.setChunkData(0, asSpan(packPcm<i16>(0, 0)));
    clip.setChunkData(1, asSpan(packPcm<i16>(0, 0)));

    EXPECT_FALSE(clip.load());
    EXPECT_EQ(clip.getBuffers().count(), 2);
    EXPECT_EQ(clip.getBuffers()[0], 0u);
    EXPECT_EQ(clip.getBuffers()[1], 0u);
    EXPECT_EQ(clip.getMaxResidency(), 2);
    EXPECT_EQ(clip.getCurrentResidency(), 2);
    EXPECT_EQ(clip.getAllocatedResidency(), 2);

    clip.unload();
    EXPECT_TRUE(clip.getBuffers().isEmpty());
}
