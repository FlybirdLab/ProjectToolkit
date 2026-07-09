/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

#if defined(GFX_API_VULKAN) && GFX_API_VULKAN
#include <toolkit/platforms/vulkan/VkBuffer.hpp>
#include <toolkit/platforms/vulkan/VkDevice.hpp>
#include <toolkit/platforms/vulkan/VkSampler.hpp>
#include <toolkit/platforms/vulkan/VkTexture.hpp>
#endif

using namespace toolkit;
using namespace toolkit::tests;

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GraphicsDeviceTest);

TEST_P(GraphicsDeviceTest, BackendNativeHandles)
{
    if (GetParam() != GfxBackendType::kVulkan)
        GTEST_SKIP() << "Native Vulkan handle checks only apply to Vulkan.";

#if defined(GFX_API_VULKAN) && GFX_API_VULKAN
    auto *vkDevice = static_cast<VulkanDevice *>(device);
    ASSERT_NE(VulkanDevice::instance, VK_NULL_HANDLE);
    ASSERT_NE(vkDevice->device, VK_NULL_HANDLE);
    ASSERT_NE(vkDevice->adapter, nullptr);
    ASSERT_NE(vkDevice->allocator, VK_NULL_HANDLE);

    auto *buffer = static_cast<VulkanBuffer *>(device->createBuffer("RhiTest.VulkanBuffer"));
    ASSERT_NE(buffer, nullptr);
    ASSERT_TRUE(buffer->init(GfxBufferDescription::buffer(64, GfxBufferFlags::kNone)));
    EXPECT_NE(buffer->getHandle(), VK_NULL_HANDLE);
    EXPECT_NE(buffer->getAllocation(), VK_NULL_HANDLE);
    deleteResource(buffer);

    auto *texture = static_cast<VulkanTexture *>(device->createTexture("RhiTest.VulkanTexture"));
    ASSERT_NE(texture, nullptr);
    ASSERT_TRUE(texture->init(GfxTextureDescription::new2D(8, 8, PixelFormat::kR8G8B8A8_UNorm)));
    EXPECT_NE(texture->getHandle(), VK_NULL_HANDLE);
    EXPECT_NE(texture->view(0), nullptr);
    deleteResource(texture);

    auto *sampler = static_cast<VulkanSampler *>(device->createSampler());
    ASSERT_NE(sampler, nullptr);
    ASSERT_TRUE(sampler->init(GfxSamplerDescription::newSampler()));
    EXPECT_NE(sampler->sampler, VK_NULL_HANDLE);
    deleteResource(sampler);
#endif
}

TEST_P(GraphicsDeviceTest, HeadlessSmoke)
{
    if (GetParam() != GfxBackendType::kNull)
        GTEST_SKIP() << "Headless checks only apply to the headless backend.";

    EXPECT_EQ(device->getBackendType(), GfxBackendType::kNull);
    device->waitForGPU();
}
