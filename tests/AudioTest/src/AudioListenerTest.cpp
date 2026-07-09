/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

using namespace audio_test;

namespace
{
    class AudioListenerTest : public AudioTestBase
    {
    };
}

TEST_F(AudioListenerTest, ListenerStartsInactive)
{
    AudioListener listener;
    EXPECT_FALSE(listener.isActive());
    EXPECT_EQ(listener.getVelocity(), Vec3::kZero);
    EXPECT_EQ(listener.getPosition(), Vec3::kZero);
    EXPECT_EQ(listener.getOrientation(), Quat::kIdentity);
}

TEST_F(AudioListenerTest, DeactivatingInactiveListenerIsSafe)
{
    AudioListener listener;
    EXPECT_TRUE(listener.setActive(false));
    EXPECT_FALSE(listener.isActive());
    EXPECT_EQ(Audio::Listeners.count(), 0);
}

TEST_F(AudioListenerTest, ActivateAndDeactivateAffectCollection)
{
    AudioListener listener;
    EXPECT_TRUE(listener.setActive(true));
    EXPECT_TRUE(listener.isActive());
    EXPECT_EQ(Audio::Listeners.count(), 1);

    EXPECT_TRUE(listener.setActive(false));
    EXPECT_FALSE(listener.isActive());
    EXPECT_EQ(Audio::Listeners.count(), 0);
}

TEST_F(AudioListenerTest, ActiveListenerDestructorRemovesFromCollection)
{
    EXPECT_EQ(Audio::Listeners.count(), 0);
    {
        AudioListener listener;
        EXPECT_TRUE(listener.setActive(true));
        EXPECT_EQ(Audio::Listeners.count(), 1);
    }
    EXPECT_EQ(Audio::Listeners.count(), 0);
}

TEST_F(AudioListenerTest, SecondActiveListenerIsRejected)
{
    AudioListener first;
    AudioListener second;

    EXPECT_TRUE(first.setActive(true));
    EXPECT_FALSE(second.setActive(true));
    EXPECT_EQ(Audio::Listeners.count(), 1);
}

TEST_F(AudioListenerTest, PositionAndVelocityTrackingWorks)
{
    AudioListener listener;
    ASSERT_TRUE(listener.setActive(true));

    listener.setTransform(Vec3(10.0f, 0.0f, 0.0f), Quat::kIdentity);
    EXPECT_EQ(listener.getPosition(), Vec3(10.0f, 0.0f, 0.0f));
    EXPECT_EQ(listener.getOrientation(), Quat::kIdentity);
    listener.update(2.0f);
    EXPECT_EQ(listener.getVelocity(), Vec3(5.0f, 0.0f, 0.0f));

    listener.resetMotionTracking();
    EXPECT_EQ(listener.getVelocity(), Vec3::kZero);
}
