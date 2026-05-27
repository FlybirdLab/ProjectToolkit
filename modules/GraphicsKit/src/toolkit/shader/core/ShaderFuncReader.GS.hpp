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
    // Geometry Shaders reader
    class GeometryShaderFunctionReader : public ShaderFunctionReader<GeometryShaderMeta>
    {
        class MaxVertexCountReader : public ITokenReader
        {
        private:
            Token m_startToken;

        public:
            MaxVertexCountReader()
                : m_startToken("maxvertexcount")
            {
            }

        public:
            bool checkStartToken(const Token &token) override
            {
                return token == m_startToken;
            }

            void process(IShaderParser *parser, Reader &text) override
            {
                // Read line to end
                text.readLine();
            }
        };

        DECLARE_SHADER_META_READER_HEADER("META_GS", gs);

        GeometryShaderFunctionReader()
        {
            m_childReaders.add(TkMemory::newObject<MaxVertexCountReader>());
        }

        ~GeometryShaderFunctionReader() = default;
    };
}
