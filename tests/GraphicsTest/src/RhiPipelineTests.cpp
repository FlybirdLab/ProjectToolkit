/*
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "GraphicsTestCommon.hpp"

using namespace toolkit;
using namespace toolkit::tests;

TEST_P(GraphicsDeviceTest, PipelineObjectCreateRelease)
{
    auto *pipeline = device->createPipelineState();
    ASSERT_NE(pipeline, nullptr);
    EXPECT_FALSE(pipeline->isValid());
    pipeline->releaseData();
    pipeline->releaseData();
    deleteResource(pipeline);
}

TEST_P(GraphicsDeviceTest, GraphicsAndComputePipelineCreationRequiresCompiledShaders)
{
    GTEST_SKIP() << "Pipeline creation needs compiled shader bytecode fixtures; covered by named placeholder until ShaderCompiler test API is exposed.";
}
