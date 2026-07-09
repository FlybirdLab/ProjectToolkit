/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, CreateTexture2DRgba8)
{
    for (i32 size : {1, 16, 256})
    {
        auto *texture = device->createTexture("RhiTest.Texture2D");
        ASSERT_NE(texture, nullptr);
        ASSERT_TRUE(texture->init(GfxTextureDescription::new2D(size, size, PixelFormat::kR8G8B8A8_UNorm, GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget)));
        EXPECT_EQ(texture->width(), size);
        EXPECT_EQ(texture->height(), size);
        EXPECT_EQ(texture->mipLevels(), 1);
        EXPECT_NE(texture->view(0), nullptr);
        deleteResource(texture);
    }
}

TEST_P(GraphicsDeviceTest, CreateTextureMipsArrayCubeAnd3D)
{
    auto *mips = device->createTexture("RhiTest.TextureMips");
    ASSERT_NE(mips, nullptr);
    ASSERT_TRUE(mips->init(GfxTextureDescription::new2D(16, 16, 4, PixelFormat::kR8G8B8A8_UNorm, GfxTextureFlags::kShaderResource | GfxTextureFlags::kPerMipViews)));
    EXPECT_EQ(mips->mipLevels(), 4);
    EXPECT_NE(mips->view(0, 0), nullptr);
    EXPECT_NE(mips->view(0, 1), nullptr);
    deleteResource(mips);

    auto *array = device->createTexture("RhiTest.TextureArray");
    ASSERT_NE(array, nullptr);
    ASSERT_TRUE(array->init(GfxTextureDescription::new2D(8, 8, PixelFormat::kR8G8B8A8_UNorm, GfxTextureFlags::kShaderResource, 4)));
    EXPECT_EQ(array->arraySize(), 4);
    EXPECT_NE(array->view(0), nullptr);
    EXPECT_NE(array->view(3), nullptr);
    deleteResource(array);

    auto *cube = device->createTexture("RhiTest.TextureCube");
    ASSERT_NE(cube, nullptr);
    ASSERT_TRUE(cube->init(GfxTextureDescription::newCube(8, PixelFormat::kR8G8B8A8_UNorm, GfxTextureFlags::kShaderResource)));
    EXPECT_TRUE(cube->isCubeMap());
    EXPECT_EQ(cube->arraySize(), 6);
    deleteResource(cube);

    auto *volume = device->createTexture("RhiTest.Texture3D");
    ASSERT_NE(volume, nullptr);
    ASSERT_TRUE(volume->init(GfxTextureDescription::new3D(4, 4, 4, PixelFormat::kR8G8B8A8_UNorm, GfxTextureFlags::kShaderResource)));
    EXPECT_TRUE(volume->isVolume());
    deleteResource(volume);
}

TEST_P(GraphicsDeviceTest, CreateDepthRenderTargetAndStorageTextures)
{
    auto *depth = device->createTexture("RhiTest.Depth");
    ASSERT_NE(depth, nullptr);
    ASSERT_TRUE(depth->init(GfxTextureDescription::new2D(16, 16, PixelFormat::kD32_Float, GfxTextureFlags::kDepthStencil)));
    EXPECT_TRUE(depth->isDepthStencil());
    deleteResource(depth);

    auto *rt = device->createTexture("RhiTest.RenderTarget");
    ASSERT_NE(rt, nullptr);
    ASSERT_TRUE(rt->init(GfxTextureDescription::new2D(16, 16, PixelFormat::kR8G8B8A8_UNorm, GfxTextureFlags::kRenderTarget | GfxTextureFlags::kShaderResource)));
    EXPECT_TRUE(rt->isRenderTarget());
    deleteResource(rt);

    auto *storage = device->createTexture("RhiTest.Storage");
    ASSERT_NE(storage, nullptr);
    ASSERT_TRUE(storage->init(GfxTextureDescription::new2D(16, 16, PixelFormat::kR8G8B8A8_UNorm, GfxTextureFlags::kUnorderedAccess | GfxTextureFlags::kShaderResource)));
    EXPECT_TRUE(storage->isUnorderedAccess());
    deleteResource(storage);
}

TEST_P(GraphicsDeviceTest, TextureInvalidDescFailsSafely)
{
    auto *texture = device->createTexture("RhiTest.InvalidTexture");
    ASSERT_NE(texture, nullptr);
    EXPECT_FALSE(texture->init(GfxTextureDescription::new2D(0, 16, PixelFormat::kR8G8B8A8_UNorm)));
    deleteResource(texture);
}

TEST_P(GraphicsDeviceTest, TextureReleaseTwice)
{
    auto *texture = device->createTexture("RhiTest.TextureReleaseTwice");
    ASSERT_NE(texture, nullptr);
    ASSERT_TRUE(texture->init(GfxTextureDescription::new2D(8, 8, PixelFormat::kR8G8B8A8_UNorm)));
    texture->releaseData();
    texture->releaseData();
    EXPECT_EQ(texture->getMemoryUse(), 0u);
    deleteResource(texture);
}
