/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, WaitIdleIsStableWithNoSubmittedWork)
{
    device->waitForGPU();
}

TEST_P(GraphicsDeviceTest, FenceSemaphoreScenariosRequirePublicSyncApi)
{
    GTEST_SKIP() << "Fence/semaphore handles are backend internals; expose public sync API or a test hook before asserting timeout/reset behavior.";
}
