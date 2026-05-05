/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, ResourceListAddRemove)
{
    const i32 before = device->getResources().count();
    auto *buffer = device->createBuffer("RhiTest.ResourceList");
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(device->getResources().count(), before + 1);
    deleteResource(buffer);
    EXPECT_EQ(device->getResources().count(), before);
}
