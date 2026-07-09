/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, QueueWaitIdle)
{
    device->waitForGPU();
}

TEST_P(GraphicsDeviceTest, CommandCopyAndDrawScenariosRequirePublicCommandApi)
{
    GTEST_SKIP() << "Command buffer and render pass objects are backend internals; add public RHI command API hooks before testing copies/draw/dispatch directly.";
}
