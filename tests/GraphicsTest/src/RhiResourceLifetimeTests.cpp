/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, DeviceDisposeWithLiveResources)
{
    auto *buffer = device->createBuffer("RhiTest.LiveBuffer");
    ASSERT_NE(buffer, nullptr);
    ASSERT_TRUE(buffer->init(GfxBufferDescription::buffer(128, GfxBufferFlags::kNone)));

    auto *texture = device->createTexture("RhiTest.LiveTexture");
    ASSERT_NE(texture, nullptr);
    ASSERT_TRUE(texture->init(GfxTextureDescription::new2D(8, 8, PixelFormat::kR8G8B8A8_UNorm)));

    auto *sampler = device->createSampler();
    ASSERT_NE(sampler, nullptr);
    ASSERT_TRUE(sampler->init(GfxSamplerDescription::newSampler()));

    GfxManager::disposeDevice();
    EXPECT_EQ(GfxManager::getDevice(), nullptr);

    deleteResource(buffer);
    deleteResource(texture);
    deleteResource(sampler);

    device = nullptr;
}

TEST_P(GraphicsDeviceTest, OnReleasingCalledByDeviceDispose)
{
    auto *buffer = device->createBuffer("RhiTest.OnReleasing");
    ASSERT_NE(buffer, nullptr);
    ASSERT_TRUE(buffer->init(GfxBufferDescription::buffer(128, GfxBufferFlags::kNone)));

    bool called = false;
    buffer->onReleasing.bind([&called]() { called = true; });
    GfxManager::disposeDevice();
    EXPECT_TRUE(called);

    deleteResource(buffer);
    device = nullptr;
}

TEST_P(GraphicsDeviceTest, MemoryUsageClearedOnRelease)
{
    auto *buffer = device->createBuffer("RhiTest.MemoryUsage");
    ASSERT_NE(buffer, nullptr);
    ASSERT_TRUE(buffer->init(GfxBufferDescription::buffer(128, GfxBufferFlags::kNone)));
    EXPECT_GT(buffer->getMemoryUse(), 0u);
    buffer->releaseData();
    EXPECT_EQ(buffer->getMemoryUse(), 0u);
    deleteResource(buffer);
}
