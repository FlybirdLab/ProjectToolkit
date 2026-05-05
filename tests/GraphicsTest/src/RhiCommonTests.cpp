/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

INSTANTIATE_TEST_SUITE_P(EnabledBackends, GraphicsDeviceTest, ::testing::ValuesIn(getEnabledGraphicsBackends()), getBackendTestName);

TEST(RhiDescriptions, PixelFormatBasics)
{
    GfxPixelFormat::init();

    EXPECT_EQ(GfxPixelFormat::sizeInBytes(PixelFormat::kUnknown), 0);
    EXPECT_EQ(GfxPixelFormat::sizeInBytes(PixelFormat::kR8G8B8A8_UNorm), 4);
    EXPECT_EQ(GfxPixelFormat::sizeInBytes(PixelFormat::kR16G16B16A16_Float), 8);
    EXPECT_EQ(GfxPixelFormat::sizeInBytes(PixelFormat::kR32G32B32A32_Float), 16);

    EXPECT_TRUE(GfxPixelFormat::isDepthStencil(PixelFormat::kD24_UNorm_S8_UInt));
    EXPECT_TRUE(GfxPixelFormat::hasStencil(PixelFormat::kD24_UNorm_S8_UInt));
    EXPECT_TRUE(GfxPixelFormat::isSRGB(PixelFormat::kR8G8B8A8_UNorm_sRGB));
    EXPECT_TRUE(GfxPixelFormat::isCompressed(PixelFormat::kBC1_UNorm));
    EXPECT_EQ(GfxPixelFormat::computeBlockSize(PixelFormat::kR8G8B8A8_UNorm), 1);
    EXPECT_EQ(GfxPixelFormat::computeBlockSize(PixelFormat::kBC1_UNorm), 4);
}

TEST(RhiDescriptions, ShaderStageConstants)
{
    EXPECT_EQ(ShaderStage_Count, 6);
    EXPECT_EQ(kShaderStageCount, 6);
    EXPECT_EQ(static_cast<i32>(ShaderStage::kVertex), 0);
    EXPECT_EQ(static_cast<i32>(ShaderStage::kHull), 1);
    EXPECT_EQ(static_cast<i32>(ShaderStage::kDomain), 2);
    EXPECT_EQ(static_cast<i32>(ShaderStage::kGeometry), 3);
    EXPECT_EQ(static_cast<i32>(ShaderStage::kPixel), 4);
    EXPECT_EQ(static_cast<i32>(ShaderStage::kCompute), 5);
}

TEST(RhiDescriptions, ShaderBindingsDefaultInitialized)
{
    ShaderBindings bindings;
    EXPECT_EQ(bindings.instructionsCount, 0u);
    EXPECT_EQ(bindings.usedCBsMask, 0u);
    EXPECT_EQ(bindings.usedSRsMask, 0u);
    EXPECT_EQ(bindings.usedUAsMask, 0u);
    EXPECT_FALSE(bindings.isUsingCB(0));
    EXPECT_FALSE(bindings.isUsingSR(0));
    EXPECT_FALSE(bindings.isUsingUA(0));
}

TEST(RhiDescriptions, BufferDescriptionFactories)
{
    auto raw = GfxBufferDescription::raw(64);
    EXPECT_EQ(raw.size, 64u);
    EXPECT_EQ(raw.stride, sizeof(float));
    EXPECT_EQ(raw.format, PixelFormat::kR32_Typeless);
    EXPECT_TRUE(enumHasAnyFlags(raw.flags, GfxBufferFlags::kRawBuffer));

    auto structured = GfxBufferDescription::structured(8, 16, true);
    EXPECT_EQ(structured.size, 128u);
    EXPECT_EQ(structured.stride, 16u);
    EXPECT_TRUE(structured.isShaderResource());
    EXPECT_TRUE(structured.isUnorderedAccess());

    u16 indices[] = {0, 1, 2};
    auto index = GfxBufferDescription::index(sizeof(u16), 3, indices);
    EXPECT_EQ(index.size, 6u);
    EXPECT_EQ(index.stride, 2u);
    EXPECT_EQ(index.format, PixelFormat::kR16_UInt);
    EXPECT_TRUE(enumHasAnyFlags(index.flags, GfxBufferFlags::kIndexBuffer));
}

TEST(RhiDescriptions, TextureDescriptionFactories)
{
    auto tex2D = GfxTextureDescription::new2D(16, 8, PixelFormat::kR8G8B8A8_UNorm);
    EXPECT_EQ(tex2D.width, 16);
    EXPECT_EQ(tex2D.height, 8);
    EXPECT_EQ(tex2D.depth, 1);
    EXPECT_EQ(tex2D.arraySize, 1);
    EXPECT_EQ(tex2D.mipLevels, 1);
    EXPECT_TRUE(tex2D.isShaderResource());

    auto tex3D = GfxTextureDescription::new3D(4, 5, 6, PixelFormat::kR8G8B8A8_UNorm);
    EXPECT_TRUE(tex3D.isVolume());
    EXPECT_EQ(tex3D.depth, 6);

    auto cube = GfxTextureDescription::newCube(8, PixelFormat::kR8G8B8A8_UNorm);
    EXPECT_TRUE(cube.isCubeMap());
    EXPECT_EQ(cube.arraySize, 6);
}

TEST(RhiDescriptions, SamplerDescriptionDefaults)
{
    GfxSamplerDescription desc = GfxSamplerDescription::newSampler();
    EXPECT_EQ(desc.filter, GfxSamplerFilter::kPoint);
    EXPECT_EQ(desc.addressU, GfxSamplerAddressMode::kWrap);
    EXPECT_EQ(desc.addressV, GfxSamplerAddressMode::kWrap);
    EXPECT_EQ(desc.addressW, GfxSamplerAddressMode::kWrap);
    EXPECT_EQ(desc.comparisonFunction, GfxSamplerCompareFunction::kNever);
}

TEST(GraphicsDeviceLifecycle, DisposeWithoutDeviceIsStable)
{
    GfxManager::disposeDevice();
    EXPECT_EQ(GfxManager::getDevice(), nullptr);
    GfxManager::disposeDevice();
    EXPECT_EQ(GfxManager::getDevice(), nullptr);
}

TEST_P(GraphicsDeviceTest, CreateDisposeDevice)
{
    ASSERT_NE(device, nullptr);
    EXPECT_EQ(device->getBackendType(), GetParam());
    EXPECT_TRUE(device->canDraw());
}

TEST_P(GraphicsDeviceTest, FrameLifecycle)
{
    device->beginFrame();
    device->endFrame();
    device->beginFrame();
    device->endFrame();
    EXPECT_GE(device->getFrameIndex(), 2u);
}
