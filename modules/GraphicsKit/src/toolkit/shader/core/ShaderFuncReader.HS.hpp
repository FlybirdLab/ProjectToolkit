/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "ShaderFuncReader.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{
    // Hull Shaders reader
    class HullShaderFunctionReader : public ShaderFunctionReader<HullShaderMeta>
    {
        class PatchSizeReader : public ITokenReader
        {
        protected:
            HullShaderFunctionReader *m_parent;
            Token m_startToken;

        public:
            PatchSizeReader(HullShaderFunctionReader *parent)
                : m_parent(parent), m_startToken("META_HS_PATCH")
            {
            }

        public:
            bool checkStartToken(const Token &token) override
            {
                return token == m_startToken;
            }

            void process(IShaderParser *parser, Reader &text) override
            {
                Token token;
                auto &current = m_parent->m_current;

                // Input control points count
                text.readToken(&token);
                token = parser->getMacros().getValue(token);
                i32 controlPointsCount;
                if (!TkStringUtils::fromString(TkStringView{token.start, token.length}, controlPointsCount))
                {
                    parser->onError("Cannot parse Hull shader input control points count.");
                    return;
                }
                if (!TkMath::isInRange(controlPointsCount, 1, 32))
                {
                    parser->onError("Invalid amount of control points. Valid range is [1-32].");
                    return;
                }
                current.controlPointsCount = controlPointsCount;
            }
        };

        DECLARE_SHADER_META_READER_HEADER("META_HS", hs);

        HullShaderFunctionReader()
        {
            m_childReaders.add(TkMemory::newObject<PatchSizeReader>(TkMemory::getDefault(), this));
            m_childReaders.add(TkMemory::newObject<StripLineReader>(TkMemory::getDefault(), "domain"));
            m_childReaders.add(TkMemory::newObject<StripLineReader>(TkMemory::getDefault(), "partitioning"));
            m_childReaders.add(TkMemory::newObject<StripLineReader>(TkMemory::getDefault(), "outputtopology"));
            m_childReaders.add(TkMemory::newObject<StripLineReader>(TkMemory::getDefault(), "maxtessfactor"));
            m_childReaders.add(TkMemory::newObject<StripLineReader>(TkMemory::getDefault(), "outputcontrolpoints"));
            m_childReaders.add(TkMemory::newObject<StripLineReader>(TkMemory::getDefault(), "patchconstantfunc"));
        }

        ~HullShaderFunctionReader() = default;

        void onParseBefore(IShaderParser *parser, Reader &text) override
        {
            // Clear current meta
            m_current.controlPointsCount = 0;

            // Base
            ShaderFunctionReader::onParseBefore(parser, text);
        }

        void onParseAfter(IShaderParser *parser, Reader &text) override
        {
            // Check if errors in parsed data
            if (m_current.controlPointsCount == 0)
            {
                parser->onError(TkString::format("Hull Shader \'%s\' has missing META_HS_PATCH macro that defines the amount of the input control points from the Input Assembler.", m_current.name));
                return;
            }

            // Base
            ShaderFunctionReader::onParseAfter(parser, text);
        }
    };
}
