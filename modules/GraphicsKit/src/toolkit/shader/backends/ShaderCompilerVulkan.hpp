/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/shader/core/Compiler.hpp"

namespace toolkit
{
    // Implementation of shaders compiler for Vulkan rendering backend.
    class ShaderCompilerVulkan : public ShaderCompiler
    {
    private:
        TkArray<char> m_funcNameDefineBuffer;

    public:
        ShaderCompilerVulkan(GfxShaderProfile profile);
        ~ShaderCompilerVulkan();

    protected:
        bool compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite = nullptr) override;
        bool onCompileBegin() override;
    };

}
