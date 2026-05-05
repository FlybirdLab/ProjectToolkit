/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, CreateVertexBuffer)
{
    GfxVertexLayout *layout = createPositionLayout();
    ASSERT_NE(layout, nullptr);
    const f32 vertices[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    auto *buffer = device->createBuffer("RhiTest.VertexBuffer");
    ASSERT_NE(buffer, nullptr);
    ASSERT_TRUE(buffer->init(GfxBufferDescription::vertex(layout, sizeof(f32) * 3, 3, vertices)));
    EXPECT_EQ(buffer->getSize(), sizeof(vertices));
    EXPECT_TRUE(enumHasAnyFlags(buffer->getFlags(), GfxBufferFlags::kVertexBuffer));
    deleteResource(buffer);
}

TEST_P(GraphicsDeviceTest, CreateIndexBuffers)
{
    u16 indices16[] = {0, 1, 2};
    auto *ib16 = device->createBuffer("RhiTest.Index16");
    ASSERT_NE(ib16, nullptr);
    ASSERT_TRUE(ib16->init(GfxBufferDescription::index(sizeof(u16), 3, indices16)));
    EXPECT_EQ(ib16->getFormat(), PixelFormat::kR16_UInt);
    deleteResource(ib16);

    u32 indices32[] = {0, 1, 2};
    auto *ib32 = device->createBuffer("RhiTest.Index32");
    ASSERT_NE(ib32, nullptr);
    ASSERT_TRUE(ib32->init(GfxBufferDescription::index(sizeof(u32), 3, indices32)));
    EXPECT_EQ(ib32->getFormat(), PixelFormat::kR32_UInt);
    deleteResource(ib32);
}

TEST_P(GraphicsDeviceTest, CreateStructuredAndRawBuffers)
{
    auto *structured = device->createBuffer("RhiTest.Structured");
    ASSERT_NE(structured, nullptr);
    ASSERT_TRUE(structured->init(GfxBufferDescription::structured(16, sizeof(u32), true)));
    EXPECT_EQ(structured->getStride(), sizeof(u32));
    EXPECT_EQ(structured->getElementsCount(), 16u);
    EXPECT_TRUE(structured->isShaderResource());
    EXPECT_TRUE(structured->isUnorderedAccess());
    deleteResource(structured);

    auto *raw = device->createBuffer("RhiTest.Raw");
    ASSERT_NE(raw, nullptr);
    ASSERT_TRUE(raw->init(GfxBufferDescription::raw(64, GfxBufferFlags::kUnorderedAccess)));
    EXPECT_TRUE(enumHasAnyFlags(raw->getFlags(), GfxBufferFlags::kRawBuffer));
    EXPECT_TRUE(raw->isUnorderedAccess());
    deleteResource(raw);
}

TEST_P(GraphicsDeviceTest, StagingUploadBufferMapUnmap)
{
    auto *buffer = device->createBuffer("RhiTest.StagingUpload");
    ASSERT_NE(buffer, nullptr);
    ASSERT_TRUE(buffer->init(GfxBufferDescription::buffer(256, GfxBufferFlags::kNone, PixelFormat::kUnknown, nullptr, 0, GfxResourceUsage::kStagingUpload)));
    ASSERT_TRUE(buffer->isStaging());
    void *mapped = buffer->map(GfxResourceMapMode::kWrite);
    ASSERT_NE(mapped, nullptr);
    TkMemory::memoryClear(mapped, 256);
    buffer->unmap();
    deleteResource(buffer);
}

TEST_P(GraphicsDeviceTest, StagingReadbackBufferMapUnmap)
{
    auto *buffer = device->createBuffer("RhiTest.StagingReadback");
    ASSERT_NE(buffer, nullptr);
    ASSERT_TRUE(buffer->init(GfxBufferDescription::buffer(256, GfxBufferFlags::kNone, PixelFormat::kUnknown, nullptr, 0, GfxResourceUsage::kStagingReadback)));
    ASSERT_TRUE(buffer->isStaging());
    void *mapped = buffer->map(GfxResourceMapMode::kRead);
    ASSERT_NE(mapped, nullptr);
    buffer->unmap();
    deleteResource(buffer);
}

TEST_P(GraphicsDeviceTest, BufferInvalidSizeFailsSafely)
{
    auto *buffer = device->createBuffer("RhiTest.InvalidSize");
    ASSERT_NE(buffer, nullptr);
    EXPECT_FALSE(buffer->init(GfxBufferDescription::buffer(0, GfxBufferFlags::kNone)));
    deleteResource(buffer);
}

TEST_P(GraphicsDeviceTest, BufferReleaseTwiceAndResourceList)
{
    const i32 before = device->getResources().count();
    auto *buffer = device->createBuffer("RhiTest.ReleaseTwice");
    ASSERT_NE(buffer, nullptr);
    EXPECT_GT(device->getResources().count(), before);
    ASSERT_TRUE(buffer->init(GfxBufferDescription::buffer(64, GfxBufferFlags::kNone)));
    buffer->releaseData();
    buffer->releaseData();
    EXPECT_EQ(buffer->getMemoryUse(), 0u);
    deleteResource(buffer);
    EXPECT_EQ(device->getResources().count(), before);
}
