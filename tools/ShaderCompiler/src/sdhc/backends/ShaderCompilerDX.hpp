/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "sdhc/Compiler.hpp"

namespace toolkit
{
    // Implementation of shaders compiler for DirectX platform using https://github.com/microsoft/DirectXShaderCompiler.
    class ShaderCompilerDX : public ShaderCompiler
    {
    private:
        TkArray<char> m_funcNameDefineBuffer;
        void *m_compiler;
        void *m_library;
        void *m_containerReflection;

    public:
        ShaderCompilerDX(GfxShaderProfile profile);
        ~ShaderCompilerDX();

    protected:
        bool compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite = nullptr) override;
        bool onCompileBegin() override;
    };
}