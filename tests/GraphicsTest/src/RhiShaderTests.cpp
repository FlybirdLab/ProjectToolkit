/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, ShaderInvalidBytecodeDoesNotCrash)
{
    auto *shader = device->createShader("RhiTest.InvalidShader");
    ASSERT_NE(shader, nullptr);
    deleteResource(shader);
}
