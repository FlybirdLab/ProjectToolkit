/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxShader.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

#include <toolkit/types/Span.hpp>

namespace toolkit
{

    class GLShader : public GfxShader
    {
    protected:
        GfxShaderProgram *createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream) override;
    };

}

#endif
