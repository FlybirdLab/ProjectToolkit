/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "ShaderFuncReader.hpp"
#include "ShaderProcessing.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{
    // Vertex Shaders reader
    class VertexShaderFunctionReader : public ShaderFunctionReader<VertexShaderMeta>
    {
        class InputLayoutReader : public ITokenReader
        {
        protected:
            VertexShaderFunctionReader *m_parent;
            Token m_startToken;

        public:
            InputLayoutReader(VertexShaderFunctionReader *parent)
                : m_parent(parent), m_startToken("META_VS_IN_ELEMENT")
            {
            }

        public:
            bool checkStartToken(const Token &token) override
            {
                return token == m_startToken;
            }

            void process(IShaderParser *parser, Reader &text) override
            {
                VertexShaderMeta::InputElement element;
                Token token;
                auto &current = m_parent->m_current;

                // Semantic type
                text.readToken(&token);
                element.type = parseInputType(token);

                // Semantic index
                text.readToken(&token);
                if (!TkStringUtils::fromString(TkStringView{token.start, token.length}, element.index))
                {
                    parser->onError("Cannot parse token.");
                    return;
                }

                // Element format
                text.readToken(&token);
                element.format = parsePixelFormat(token);
                if (element.format == PixelFormat::kUnknown)
                {
                    parser->onError(TkString::format("Unknown input data format \'%s\' for the Vertex Shader.", token.toString()));
                    return;
                }

                // Input slot
                text.readToken(&token);
                if (!TkStringUtils::fromString(TkStringView{token.start, token.length}, element.inputSlot))
                {
                    parser->onError("Cannot parse token.");
                    return;
                }

                // Aligned byte offset
                text.readToken(&token);
                if (token == "ALIGN")
                {
                    element.alignedByteOffset = INPUT_LAYOUT_ELEMENT_ALIGN;
                }
                else if (!TkStringUtils::fromString(TkStringView{token.start, token.length}, element.alignedByteOffset))
                {
                    parser->onError("Cannot parse token.");
                    return;
                }
                else if (element.alignedByteOffset > kMaxU8)
                {
                    parser->onError("Too big vertex element byte offset.");
                    return;
                }

                // Input slot class
                text.readToken(&token);
                if (token == "PER_VERTEX")
                {
                    element.inputSlotClass = INPUT_LAYOUT_ELEMENT_PER_VERTEX_DATA;
                }
                else if (token == "PER_INSTANCE")
                {
                    element.inputSlotClass = INPUT_LAYOUT_ELEMENT_PER_INSTANCE_DATA;
                }
                else
                {
                    parser->onError(TkString::format("Invalid input slot class type \'%s\'.", token.toString()));
                    return;
                }

                // Instance data step rate
                text.readToken(&token);
                if (!TkStringUtils::fromString(TkStringView{token.start, token.length}, element.instanceDataStepRate))
                {
                    parser->onError("Cannot parse token.");
                    return;
                }

                // Visible state
                text.readToken(&token);
                element.visibleFlag = token.toString();

                current.inputLayout.add(element);
            }
        };

        DECLARE_SHADER_META_READER_HEADER("META_VS", vs);

        VertexShaderFunctionReader()
        {
            m_childReaders.add(TkMemory::newObject<InputLayoutReader>(TkMemory::getDefault(), this));
        }
        ~VertexShaderFunctionReader() = default;

        void onParseBefore(IShaderParser *parser, Reader &text) override
        {
            // Clear current meta
            m_current.inputLayout.clear();

            // Base
            ShaderFunctionReader::onParseBefore(parser, text);
        }

        void onParseAfter(IShaderParser *parser, Reader &text) override
        {
            // Check if errors in specified input layout
            if (m_current.inputLayout.count() > GFX_MAX_VS_ELEMENTS)
            {
                parser->onError(TkString::format("Vertex Shader \'%s\' has too many input layout elements specified. Maximum allowed amount is %s.", m_current.name, GFX_MAX_VS_ELEMENTS));
                return;
            }

            // Base
            ShaderFunctionReader::onParseAfter(parser, text);
        }
    };
}
