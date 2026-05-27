/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/shader/core/Compiler.hpp"

namespace toolkit
{
    // Implementation of shaders compiler for the desktop OpenGL fallback
    // backend (GLSL 410, Core profile).
    //
    // Phase 6 scope -- Plan B (passthrough + header wrap):
    //   - Input is treated as GLSL 410 source bytes (no HLSL-to-GLSL cross
    //     compilation in this phase).
    //   - Output bytecode = GLShaderSourceHeader + GLSL source bytes.
    //   - Only VS / PS stages are accepted; HS/DS/GS/CS report a clear error.
    //   - No SPIR-V Cross dependency.
    //
    // The GLSL runtime (GLShader::createShaderProgram) already understands the
    // GLShaderSourceHeader format via the shared toolkit/graphics/
    // GLShaderSourceFormat.hpp header.
    class ShaderCompilerGLSL : public ShaderCompiler
    {
    public:
        ShaderCompilerGLSL(GfxShaderProfile profile);

    protected:
        bool compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite = nullptr) override;
        bool onCompileBegin() override;
    };
}
