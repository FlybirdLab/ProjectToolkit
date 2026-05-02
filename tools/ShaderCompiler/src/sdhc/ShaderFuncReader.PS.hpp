/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "ShaderFuncReader.hpp"

namespace toolkit
{
    // Pixel Shaders reader
    class PixelShaderFunctionReader : public ShaderFunctionReader<PixelShaderMeta>
    {
        DECLARE_SHADER_META_READER_HEADER("META_PS", ps);

        PixelShaderFunctionReader() = default;
        ~PixelShaderFunctionReader() = default;
    };
}
