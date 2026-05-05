/*
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#include <toolkit/graphics/GfxBuffer.hpp>
#include <toolkit/graphics/GfxConstantBuffer.hpp>
#include <toolkit/graphics/GfxDevice.hpp>
#include <toolkit/graphics/GfxManager.hpp>
#include <toolkit/graphics/GfxPipelineState.hpp>
#include <toolkit/graphics/GfxSampler.hpp>
#include <toolkit/graphics/GfxShader.hpp>
#include <toolkit/graphics/GfxShaderProgram.hpp>
#include <toolkit/graphics/GfxTexture.hpp>
#include <toolkit/graphics/GfxVertexLayout.hpp>
#include <toolkit/graphics/utils/PixelFormat.hpp>
#include <toolkit/graphics/utils/TextureData.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/utils/Color32.hpp>

#include <string>
#include <vector>

namespace toolkit::tests
{
    inline std::vector<GfxBackendType> getEnabledGraphicsBackends()
    {
        std::vector<GfxBackendType> backends;
#if (defined(GFX_API_HEADLESS) && GFX_API_HEADLESS) || (defined(GFX_API_NULL) && GFX_API_NULL)
        backends.push_back(GfxBackendType::kNull);
#endif
#if defined(GFX_API_VULKAN) && GFX_API_VULKAN
        backends.push_back(GfxBackendType::kVulkan);
#endif
        return backends;
    }

    inline const char *getBackendName(GfxBackendType backend)
    {
        switch (backend)
        {
        case GfxBackendType::kNull:
            return "Headless";
        case GfxBackendType::kVulkan:
            return "Vulkan";
        default:
            return "Unknown";
        }
    }

    inline std::string getBackendTestName(const ::testing::TestParamInfo<GfxBackendType> &info)
    {
        return getBackendName(info.param);
    }

    template <typename T>
    void deleteResource(T *&resource)
    {
        if (resource)
        {
            resource->releaseData();
            TkMemory::deletePtr(resource);
            resource = nullptr;
        }
    }

    inline GfxVertexLayout *createPositionLayout()
    {
        GfxVertexLayout::Elements elements;
        elements.add(VertexElement{VertexElementTypes::kPosition, 0, 0, 0, PixelFormat::kR32G32B32_Float});
        return GfxVertexLayout::get(elements, true);
    }

    class GraphicsDeviceTest : public ::testing::TestWithParam<GfxBackendType>
    {
    protected:
        GfxDevice *device = nullptr;

        void SetUp() override
        {
            GfxManager::disposeDevice();
            ASSERT_TRUE(GfxManager::createDevice(GetParam()));
            device = GfxManager::getDevice();
            ASSERT_NE(device, nullptr);
            ASSERT_EQ(device->getBackendType(), GetParam());
            ASSERT_TRUE(device->canDraw());
        }

        void TearDown() override
        {
            GfxManager::disposeDevice();
            device = nullptr;
            ASSERT_EQ(GfxManager::getDevice(), nullptr);
        }
    };
}
