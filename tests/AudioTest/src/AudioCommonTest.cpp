/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

using namespace audio_test;

TEST(AudioCommonTest, AudioDataInfoLengthUsesAllChannels)
{
    AudioDataInfo info = makeInfo(88200, 44100, 2, 16);
    EXPECT_FLOAT_EQ(info.length(), 1.0f);
    EXPECT_FLOAT_EQ(info.GetLength(), 1.0f);
}

TEST(AudioCommonTest, AudioDataInfoLengthHandlesZeroValues)
{
    AudioDataInfo info = makeInfo(0, 0, 0, 16);
    EXPECT_FLOAT_EQ(info.length(), 0.0f);
    EXPECT_FLOAT_EQ(info.GetLength(), 0.0f);
}

TEST(AudioCommonTest, AudioDeviceCopyAndAssignmentPreserveFields)
{
    AudioDevice source;
    source.name = "Speakers";
    source.internalName = "backend.device.0";

    AudioDevice copy(source);
    EXPECT_EQ(copy.name, source.name);
    EXPECT_EQ(copy.internalName, source.internalName);

    AudioDevice assigned;
    assigned = source;
    EXPECT_EQ(assigned.name, source.name);
    EXPECT_EQ(assigned.internalName, source.internalName);
}

TEST(AudioCommonTest, AudioSystemMetadataIsQueryable)
{
    ASSERT_NE(AudioSystem::getName(), nullptr);
    EXPECT_TRUE(TkString(AudioSystem::getName()).hasChars());
    const AudioFeatureFlags features = AudioSystem::getFeatures();
    EXPECT_TRUE(features == AudioFeatureFlags::kNone || features == AudioFeatureFlags::kSpatialMultiChannel);
}
