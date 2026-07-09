/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, StagingBufferSizesAndAlignmentSmoke)
{
    for (u32 size : {1u, 4u, 15u, 16u, 255u, 256u, 4096u})
    {
        auto *buffer = device->createBuffer("RhiTest.StagingSize");
        ASSERT_NE(buffer, nullptr);
        ASSERT_TRUE(buffer->init(GfxBufferDescription::buffer(size, GfxBufferFlags::kNone, PixelFormat::kUnknown, nullptr, 0, GfxResourceUsage::kStagingUpload)));
        EXPECT_EQ(buffer->getSize(), size);
        EXPECT_TRUE(buffer->isStaging());
        deleteResource(buffer);
    }
}

TEST_P(GraphicsDeviceTest, StagingInitFailurePath)
{
    auto *buffer = device->createBuffer("RhiTest.StagingFailure");
    ASSERT_NE(buffer, nullptr);
    EXPECT_FALSE(buffer->init(GfxBufferDescription::buffer(0, GfxBufferFlags::kNone, PixelFormat::kUnknown, nullptr, 0, GfxResourceUsage::kStagingUpload)));
    deleteResource(buffer);
}
