/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "ShaderFuncReader.hpp"
#include "toolkit/memory/Memory.hpp"

namespace toolkit
{
    // Domain Shaders reader
    class DomainShaderFunctionReader : public ShaderFunctionReader<DomainShaderMeta>
    {
        DECLARE_SHADER_META_READER_HEADER("META_DS", ds);

        DomainShaderFunctionReader()
        {
            m_childReaders.add(TkMemory::newObject<StripLineReader>(TkMemory::getDefault(), "domain"));
        }
        ~DomainShaderFunctionReader() = default;
    };
}
