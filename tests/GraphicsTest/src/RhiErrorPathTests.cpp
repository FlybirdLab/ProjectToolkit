/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, InvalidResourceDescriptionsReturnFalse)
{
    auto *buffer = device->createBuffer("RhiTest.InvalidBuffer");
    ASSERT_NE(buffer, nullptr);
    EXPECT_FALSE(buffer->init(GfxBufferDescription::buffer(0, GfxBufferFlags::kNone)));
    deleteResource(buffer);

    auto *texture = device->createTexture("RhiTest.InvalidTexture");
    ASSERT_NE(texture, nullptr);
    EXPECT_FALSE(texture->init(GfxTextureDescription::new2D(0, 0, PixelFormat::kUnknown)));
    deleteResource(texture);
}
