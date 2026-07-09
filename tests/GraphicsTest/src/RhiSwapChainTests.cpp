/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST(RhiSwapChainTests, WindowBackedSwapChainTestsLiveInWindowKit)
{
    GTEST_SKIP() << "SwapChain requires WindowKit/GfxWindow surface setup and is intentionally kept out of no-window GraphicsTest runs.";
}
