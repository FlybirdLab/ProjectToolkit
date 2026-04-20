/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

using namespace audio_test;

namespace
{
    class AudioSourceTest : public AudioTestBase
    {
    };
}

TEST_F(AudioSourceTest, ConstructorRegistersAndDestructorUnregisters)
{
    EXPECT_EQ(Audio::Sources.count(), 0);
    {
        AudioSource source;
        EXPECT_EQ(Audio::Sources.count(), 1);
        EXPECT_EQ(source.getState(), AudioSource::State::kStopped);
    }
    EXPECT_EQ(Audio::Sources.count(), 0);
}

TEST_F(AudioSourceTest, PropertySettersClampValues)
{
    AudioSource source;

    source.setVolume(2.0f);
    EXPECT_FLOAT_EQ(source.getVolume(), 1.0f);
    source.setVolume(-1.0f);
    EXPECT_FLOAT_EQ(source.getVolume(), 0.0f);

    source.setPitch(0.1f);
    EXPECT_FLOAT_EQ(source.getPitch(), 0.5f);
    source.setPitch(9.0f);
    EXPECT_FLOAT_EQ(source.getPitch(), 2.0f);

    source.setPan(-2.0f);
    EXPECT_FLOAT_EQ(source.getPan(), -1.0f);
    source.setPan(2.0f);
    EXPECT_FLOAT_EQ(source.getPan(), 1.0f);

    source.setVelocity(Vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(source.getVelocity(), Vec3(1.0f, 2.0f, 3.0f));

    source.setPosition(Vec3(4.0f, 5.0f, 6.0f));
    EXPECT_EQ(source.getPosition(), Vec3(4.0f, 5.0f, 6.0f));

    source.setOrientation(Quat::kIdentity);
    EXPECT_EQ(source.getOrientation(), Quat::kIdentity);

    source.setPlayOnStart(true);
    EXPECT_TRUE(source.getPlayOnStart());

    source.setIsLooping(true);
    EXPECT_TRUE(source.getIsLooping());

    source.setStartTime(-3.0f);
    EXPECT_FLOAT_EQ(source.getStartTime(), 0.0f);

    source.setMinDistance(-5.0f);
    EXPECT_FLOAT_EQ(source.getMinDistance(), 0.0f);
    source.setAttenuation(-1.0f);
    EXPECT_FLOAT_EQ(source.getAttenuation(), 0.0f);
    source.setDopplerFactor(-4.0f);
    EXPECT_FLOAT_EQ(source.getDopplerFactor(), 0.0f);
}

TEST_F(AudioSourceTest, ClipAssignmentAndFlagsWork)
{
    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 4, 1, 16), true, false));
    EXPECT_FALSE(clip.setChunkData(0, asSpan(makeKnownWav16MonoSamples())));

    AudioSource source;
    source.setAllowSpatialization(true);
    EXPECT_TRUE(source.getAllowSpatialization());
    source.setClip(&clip);

    EXPECT_EQ(source.getClip(), &clip);
    EXPECT_TRUE(source.is3D());
    EXPECT_FALSE(source.useStreaming());
    EXPECT_EQ(source.getSourceId(), 0u);

    source.setAllowSpatialization(false);
    EXPECT_FALSE(source.getAllowSpatialization());
    EXPECT_FALSE(source.is3D());

    source.setClip(nullptr);
    EXPECT_EQ(source.getClip(), nullptr);
}

TEST_F(AudioSourceTest, PlayPauseAndStopWithoutClipStayStopped)
{
    AudioSource source;
    source.play();
    EXPECT_EQ(source.getState(), AudioSource::State::kStopped);
    EXPECT_FALSE(source.isActuallyPlaying());
    source.update();
    source.pause();
    EXPECT_EQ(source.getState(), AudioSource::State::kStopped);
    source.stop();
    EXPECT_EQ(source.getState(), AudioSource::State::kStopped);
}

TEST_F(AudioSourceTest, MotionTrackingUpdatesVelocity)
{
    AudioSource source;
    source.setPosition(Vec3(10.0f, 0.0f, 0.0f));
    source.updateVelocityFromPositionDelta(2.0f);
    EXPECT_EQ(source.getVelocity(), Vec3(5.0f, 0.0f, 0.0f));

    source.resetMotionTracking();
    EXPECT_EQ(source.getVelocity(), Vec3::kZero);
}

TEST_F(AudioSourceTest, StreamingControlsAreCallable)
{
    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 4, 1, 16), false, true));
    EXPECT_FALSE(clip.setChunkData(0, asSpan(packPcm<i16>(0, 0))));

    AudioSource source;
    source.setClip(&clip);
    source.requestStreamingBuffersUpdate();
    source.update();

    EXPECT_FALSE(source.isActuallyPlaying());
    EXPECT_EQ(source.getState(), AudioSource::State::kStopped);
}

TEST_F(AudioSourceTest, SetTimeWorksWhenRuntimeIsAvailable)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    if (Audio::Devices.count() == 0)
        GTEST_SKIP() << "No audio devices available";

    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 8000, 1, 16), false, false));
    EXPECT_FALSE(clip.setChunkData(0, asSpan(packPcm<i16>(0, 16384, -16384, 32767))));

    AudioSource source;
    source.setClip(&clip);
    source.setTime(1.5f);

    EXPECT_FLOAT_EQ(source.getStartTime(), clip.getLength());
    EXPECT_GE(source.getTime(), 0.0f);
    EXPECT_EQ(source.getState(), AudioSource::State::kStopped);
}

TEST_F(AudioSourceTest, PlayPauseAndStopWorkWithRuntime)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    if (Audio::Devices.count() == 0)
        GTEST_SKIP() << "No audio devices available";

    AudioClip clip;
    clip.setDescriptor(makeClipDescriptor(makeInfo(4, 8000, 1, 16), false, false));
    EXPECT_FALSE(clip.setChunkData(0, asSpan(packPcm<i16>(0, 16384, -16384, 32767))));

    AudioSource source;
    source.setClip(&clip);

    source.play();
    EXPECT_EQ(source.getState(), AudioSource::State::kPlaying);
    EXPECT_TRUE(source.isActuallyPlaying());
    EXPECT_NE(source.getSourceId(), 0u);

    source.pause();
    EXPECT_EQ(source.getState(), AudioSource::State::kPaused);
    EXPECT_FALSE(source.isActuallyPlaying());

    source.play();
    EXPECT_EQ(source.getState(), AudioSource::State::kPlaying);
    EXPECT_TRUE(source.isActuallyPlaying());

    source.stop();
    EXPECT_EQ(source.getState(), AudioSource::State::kStopped);
    EXPECT_FALSE(source.isActuallyPlaying());
}
