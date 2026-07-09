/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit::gfx
{

    // ASCII 'GLSL' little-endian. Identifies a GLSL source bytecode blob
    // produced by ShaderCompilerGLSL and consumed by the OpenGL runtime
    // (GLShader::createShaderProgram). Both sides MUST include this header to
    // stay byte-compatible.
    constexpr u32 GL_SHADER_SOURCE_MAGIC = 0x4C534C47u;
    constexpr u32 GL_SHADER_SOURCE_VERSION = 1;

    // Bytecode layout written by ShaderCompilerGLSL into the shadercache:
    //
    //   [GLShaderSourceHeader][...GLSL source bytes...]
    //
    // No null-terminator is appended; sourceSize is exact.
    struct GLShaderSourceHeader
    {
        u32 magic;       // GL_SHADER_SOURCE_MAGIC
        u32 version;     // GL_SHADER_SOURCE_VERSION
        u32 profile;     // GfxShaderProfile value cast to u32
        u32 sourceSize;  // GLSL source byte count (no trailing null)
    };

    static_assert(sizeof(GLShaderSourceHeader) == 16, "GLShaderSourceHeader must stay 16 bytes for binary compatibility");

}
