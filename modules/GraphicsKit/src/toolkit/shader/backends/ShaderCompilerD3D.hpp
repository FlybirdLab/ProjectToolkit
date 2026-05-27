/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/shader/core/Compiler.hpp"

namespace toolkit
{

    /// Implementation of shaders compiler for DirectX platform using D3DCompiler.
    class ShaderCompilerD3D : public ShaderCompiler
    {
    public:
        ShaderCompilerD3D(GfxShaderProfile profile);

    protected:
        bool compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite = nullptr) override;
        bool onCompileBegin() override;

    private:
        TkArray<char> m_funcNameDefineBuffer;
        u32 m_flags;
    };
}
