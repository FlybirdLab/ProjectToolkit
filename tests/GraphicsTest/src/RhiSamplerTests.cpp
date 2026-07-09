/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, CreateSamplerVariants)
{
    for (GfxSamplerFilter filter : {GfxSamplerFilter::kPoint, GfxSamplerFilter::kBilinear, GfxSamplerFilter::kTrilinear})
    {
        auto *sampler = device->createSampler();
        ASSERT_NE(sampler, nullptr);
        ASSERT_TRUE(sampler->init(GfxSamplerDescription::newSampler(filter, GfxSamplerAddressMode::kWrap)));
        EXPECT_EQ(sampler->getDescription().filter, filter);
        deleteResource(sampler);
    }
}

TEST_P(GraphicsDeviceTest, SamplerAddressAndCompareModes)
{
    auto desc = GfxSamplerDescription::newSampler(GfxSamplerFilter::kPoint, GfxSamplerAddressMode::kClamp);
    desc.addressU = GfxSamplerAddressMode::kWrap;
    desc.addressV = GfxSamplerAddressMode::kMirror;
    desc.addressW = GfxSamplerAddressMode::kBorder;
    desc.comparisonFunction = GfxSamplerCompareFunction::kLess;

    auto *sampler = device->createSampler();
    ASSERT_NE(sampler, nullptr);
    ASSERT_TRUE(sampler->init(desc));
    EXPECT_EQ(sampler->getDescription().addressU, GfxSamplerAddressMode::kWrap);
    EXPECT_EQ(sampler->getDescription().addressV, GfxSamplerAddressMode::kMirror);
    EXPECT_EQ(sampler->getDescription().addressW, GfxSamplerAddressMode::kBorder);
    EXPECT_EQ(sampler->getDescription().comparisonFunction, GfxSamplerCompareFunction::kLess);
    deleteResource(sampler);
}
