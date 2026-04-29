/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxShader.hpp>

#if GFX_API_NULL

#include <toolkit/types/Span.hpp>

namespace toolkit
{

    // Shader for Null backend.
    class NullShader : public GfxShader
    {
    protected:
        GfxShaderProgram *createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream) override
        {
            return nullptr;
        }

    public:
        bool create(MemoryIStream &stream) override
        {
            return false;
        }
    };
}

#endif