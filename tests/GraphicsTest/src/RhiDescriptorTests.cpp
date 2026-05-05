/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, DescriptorBindingScenariosRequireShaderFixtures)
{
    GTEST_SKIP() << "Descriptor binding correctness needs compiled shader fixtures and public draw/dispatch hooks.";
}
