/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

using namespace audio_test;

namespace
{
    class AudioRuntimeTest : public AudioTestBase
    {
    };
}

TEST_F(AudioRuntimeTest, InitUpdateAndDisposeAreStable)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    EXPECT_TRUE(Audio::isInitialized());
    EXPECT_GE(Audio::Devices.count(), 1);
    EXPECT_FLOAT_EQ(Audio::getMasterVolume(), 1.0f);
    EXPECT_FLOAT_EQ(Audio::getVolume(), 1.0f);

    Audio::update(0.016f);
    Audio::update();

    Audio::dispose();
    EXPECT_FALSE(Audio::isInitialized());
}

TEST_F(AudioRuntimeTest, AudioSettingsMutatorsUpdateStaticState)
{
    EXPECT_TRUE(Audio::getMuteOnFocusLoss());
    EXPECT_FALSE(Audio::getEnableHRTF());

    Audio::setMuteOnFocusLoss(false);
    EXPECT_FALSE(Audio::getMuteOnFocusLoss());

    Audio::setEnableHRTF(true);
    EXPECT_TRUE(Audio::getEnableHRTF());
    Audio::setEnableHRTF(false);
    EXPECT_FALSE(Audio::getEnableHRTF());

    Audio::setDopplerFactor(-5.0f);
    EXPECT_FLOAT_EQ(AudioSystem::s_dopplerFactor, 0.0f);
    Audio::setDopplerFactor(1.5f);
    EXPECT_FLOAT_EQ(AudioSystem::s_dopplerFactor, 1.5f);

    Audio::setMasterVolume(1.5f);
    EXPECT_FLOAT_EQ(Audio::getMasterVolume(), 1.0f);
    Audio::setMasterVolume(0.25f);
    EXPECT_FLOAT_EQ(Audio::getMasterVolume(), 0.25f);
}

TEST_F(AudioRuntimeTest, UpdateDrivesRegisteredSources)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";

    AudioSource source;
    source.setPosition(Vec3(10.0f, 0.0f, 0.0f));
    Audio::update(2.0f);

    EXPECT_EQ(source.getVelocity(), Vec3(5.0f, 0.0f, 0.0f));
}

TEST_F(AudioRuntimeTest, FocusPauseAndMasterVolumePolicyUpdatesVolume)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";

    Audio::setMasterVolume(0.75f);
    Audio::setMuteOnFocusLoss(true);
    Audio::setAppHasFocus(true);
    Audio::setPaused(false);
    EXPECT_FLOAT_EQ(Audio::getVolume(), 0.75f);

    Audio::setPaused(true);
    Audio::update(0.016f);
    EXPECT_FLOAT_EQ(Audio::getVolume(), 0.0f);

    Audio::setPaused(false);
    Audio::setAppHasFocus(false);
    Audio::update(0.016f);
    EXPECT_FLOAT_EQ(Audio::getVolume(), 0.0f);

    Audio::setMuteOnFocusLoss(false);
    Audio::update(0.016f);
    EXPECT_FLOAT_EQ(Audio::getVolume(), 0.75f);
}

TEST_F(AudioRuntimeTest, ActiveDeviceSelectionAndSignalsAreCallable)
{
    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    if (Audio::Devices.count() == 0)
        GTEST_SKIP() << "No audio devices available";

    bool activeDeviceChanged = false;
    struct Handler
    {
        bool *flag = nullptr;
        void onChanged()
        {
            *flag = true;
        }
    } handler{&activeDeviceChanged};

    Audio::onActiveDeviceChanged.bind<Handler, &Handler::onChanged>(&handler);

    if (Audio::Devices.count() > 0)
    {
        Audio::setActiveDeviceIdx(-1);
        EXPECT_EQ(Audio::getActiveDevice(), nullptr);
        activeDeviceChanged = false;
        Audio::setActiveDeviceIdx(0);
        EXPECT_EQ(Audio::getActiveDeviceIdx(), 0);
        EXPECT_NE(Audio::getActiveDevice(), nullptr);

        Audio::setActiveDeviceIdx(9999);
        EXPECT_EQ(Audio::getActiveDeviceIdx(), Audio::Devices.count() - 1);
    }

    EXPECT_TRUE(activeDeviceChanged);
    Audio::onActiveDeviceChanged.unbind<Handler, &Handler::onChanged>(&handler);
}

TEST_F(AudioRuntimeTest, DevicesChangedSignalFiresDuringInitAndDispose)
{
    bool devicesChanged = false;
    struct Handler
    {
        bool *flag = nullptr;
        void onChanged()
        {
            *flag = true;
        }
    } handler{&devicesChanged};

    Audio::onDevicesChanged.bind<Handler, &Handler::onChanged>(&handler);

    struct BindingGuard
    {
        Handler *handler = nullptr;
        ~BindingGuard()
        {
            if (handler != nullptr)
                Audio::onDevicesChanged.unbind<Handler, &Handler::onChanged>(handler);
        }
    } guard{&handler};

    if (!Audio::init())
        GTEST_SKIP() << "Audio runtime unavailable";
    EXPECT_TRUE(Audio::isInitialized());
    EXPECT_TRUE(devicesChanged);

    devicesChanged = false;
    Audio::dispose();
    EXPECT_TRUE(devicesChanged);
}
