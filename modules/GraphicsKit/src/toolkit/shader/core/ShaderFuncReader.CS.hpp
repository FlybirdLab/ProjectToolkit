/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "ShaderFuncReader.hpp"

namespace toolkit
{
    // Compute Shaders reader
    class ComputeShaderFunctionReader : public ShaderFunctionReader<ComputeShaderMeta>
    {
    public:
        DECLARE_SHADER_META_READER_HEADER("META_CS", cs);

        ComputeShaderFunctionReader()
        {
            m_childReaders.add(TkMemory::newObject<StripLineReader>(TkMemory::getDefault(), "numthreads"));
        }

        ~ComputeShaderFunctionReader() = default;
    };
}
