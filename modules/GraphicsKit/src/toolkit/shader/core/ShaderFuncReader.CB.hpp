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
    // Constant Buffers reader
    class ConstantBufferReader : public ShaderMetaReader<ConstantBufferMeta>
    {
    private:
        Token m_endToken;
        DECLARE_SHADER_META_READER_HEADER("META_CB_BEGIN", cb);

        ConstantBufferReader()
            : m_endToken("META_CB_END")
        {
        }
        ~ConstantBufferReader() = default;

        void onParseBefore(IShaderParser *parser, Reader &text) override
        {
            Token token;

            // Clear current meta
            m_current.name.clear();

            // Here we read '(x)\n' where 'x' is a shader function slot
            text.readToken(&token);
            if (!TkStringUtils::fromString(TkStringView{token.start, token.length}, m_current.slot))
            {
                parser->onError("Invalid constant buffer slot index.");
                return;
            }

            // Read buffer name
            text.readToken(&token);
            m_current.name = token.toString();

            // Check if name is unique
            for (i32 i = 0; i < m_cache.count(); i++)
            {
                if (m_cache[i].name == m_current.name)
                {
                    parser->onError(TkString::format("Duplicated constant buffer \'%s\'. Buffer with that name already exists.", m_current.name));
                    return;
                }
            }

            // Read rest of the line
            text.readLine();
        }

        void onParse(IShaderParser *parser, Reader &text) override
        {
            Token token;

            // Read function properties
            bool foundEnd = false;
            while (text.canRead())
            {
                text.readToken(&token);

                // Try to find the ending
                if (token == m_endToken)
                {
                    foundEnd = true;
                    break;
                }
            }

            // Check if end has not been found
            if (!foundEnd)
            {
                parser->onError(TkString::format("Missing constant buffer \'%s\' ending.", m_current.name));
                return;
            }
        }

        void onParseAfter(IShaderParser *parser, Reader &text) override
        {
            // Cache buffer
            m_cache.add(m_current);
        }

        void collectResults(IShaderParser *parser, ShaderMeta *result) override
        {
            // Validate constant buffer slots overlapping
            for (i32 i = 0; i < m_cache.count(); i++)
            {
                auto &first = m_cache[i];
                for (i32 j = i + 1; j < m_cache.count(); j++)
                {
                    auto &second = m_cache[j];
                    if (first.slot == second.slot)
                    {
                        parser->onError("Constant buffers slots are overlapping.");
                        return;
                    }
                }
            }

            // Validate amount of used constant buffers
            for (i32 i = 0; i < m_cache.count(); i++)
            {
                auto &f = m_cache[i];
                if (f.slot >= GFX_MAX_CB_BINDED)
                {
                    parser->onError(TkString::format("Constant buffer %s is using invalid slot %i. Maximum supported slot is %i.", f.name, f.slot, GFX_MAX_CB_BINDED - 1));
                    return;
                }
            }

            // Base
            ShaderMetaReader::collectResults(parser, result);
        }
    };
}
