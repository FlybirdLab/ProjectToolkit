/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "IShaderFunctionReader.hpp"
#include "ITokenReadersContainer.hpp"
#include "ShaderMeta.hpp"

namespace toolkit
{
    extern GfxShaderFlags parseShaderFlags(const Token &token);

    // Implementation of shader meta functions reader
    template <typename MetaType>
    class ShaderMetaReader : public IShaderFunctionReader, public ITokenReadersContainer
    {
    protected:
        TkArray<MetaType> m_cache;
        MetaType m_current;

        ShaderMetaReader() = default;
        ~ShaderMetaReader() = default;

    protected:
        // Event called before parsing shader function
        virtual void onParseBefore(IShaderParser *parser, Reader &text) = 0;

        // Event called for parsing shader function
        virtual void onParse(IShaderParser *parser, Reader &text)
        {
            Token token;

            // Read function properties
            while (text.canRead())
            {
                text.readToken(&token);

                // Call children
                if (processChildren(token, parser))
                    break;
            }

            // Token should contain function output type, now read function name
            text.readToken(&token);
            m_current.name = token.toString();

            // Check if name is unique
            for (i32 i = 0; i < m_cache.count(); i++)
            {
                if (m_cache[i].name == m_current.name)
                {
                    parser->onError(TkString::format("Duplicated function \'%s\'. Function with that name already exists.", m_current.name));
                    return;
                }
            }
        }

        // Event called after parsing shader function
        virtual void onParseAfter(IShaderParser *parser, Reader &text) = 0;
        // Process final results and send them to the Shader Meta object
        virtual void flushCache(IShaderParser *parser, ShaderMeta *result) = 0;

    public:
        void process(IShaderParser *parser, Reader &text) override
        {
            onParseBefore(parser, text);
            if (parser->failed())
                return;

            onParse(parser, text);
            if (parser->failed())
                return;

            onParseAfter(parser, text);
        }

        void collectResults(IShaderParser *parser, ShaderMeta *result) override
        {
            // Validate function names
            for (i32 i = 0; i < m_cache.count(); i++)
            {
                auto &first = m_cache[i];
                for (i32 j = i + 1; j < m_cache.count(); j++)
                {
                    auto &second = m_cache[j];
                    if (first.name == second.name)
                    {
                        parser->onError("Duplicated shader function names.");
                        return;
                    }
                }
            }

            flushCache(parser, result);
        }
    };

    // Implementation of shader functions reade
    template <typename MetaType>
    class ShaderFunctionReader : public ShaderMetaReader<MetaType>
    {
    public:
        typedef ShaderMetaReader<MetaType> ShaderMetaReaderType;

    protected:
        class StripLineReader : public ITokenReader
        {
        private:
            Token m_startToken;

        public:
            explicit StripLineReader(const char *token)
                : m_startToken(token)
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

    public:
        // Shader function permutations reader
        class PermutationReader : public ITokenReader
        {
        protected:
            ShaderFunctionReader *m_parent;
            i32 m_startTokenPermutationSize;

            const char *PermutationTokens[SHADER_PERMUTATIONS_MAX_PARAMS_COUNT] =
                {
                    "META_PERMUTATION_1",
                    "META_PERMUTATION_2",
                    "META_PERMUTATION_3",
                    "META_PERMUTATION_4",
                };
            static_assert(SHADER_PERMUTATIONS_MAX_PARAMS_COUNT == 4, "Invalid maximum amount of shader permutations.");

        public:
            PermutationReader(ShaderFunctionReader *parent)
                : m_parent(parent), m_startTokenPermutationSize(0)
            {
            }

        public:
            // Clear cache
            void clear()
            {
                m_startTokenPermutationSize = 0;
            }

        public:
            bool checkStartToken(const Token &token) override
            {
                for (i32 i = 0; i < lengthOf(PermutationTokens); i++)
                {
                    if (token == PermutationTokens[i])
                    {
                        m_startTokenPermutationSize = i + 1;
                        return true;
                    }
                }

                return false;
            }

            void process(IShaderParser *parser, Reader &text) override
            {
                Token token;
                auto &current = m_parent->m_current;

                // Add permutation
                i32 permutationIndex = current.permutations.count();
                auto &permutation = current.permutations.addOne();

                // Read all parameters for the permutation
                TK_ASSERT(m_startTokenPermutationSize > 0);
                for (i32 paramIndex = 0; paramIndex < m_startTokenPermutationSize; paramIndex++)
                {
                    // Check for missing end
                    if (!text.canRead())
                    {
                        parser->onError("Missing ending of shader function permutation.");
                        return;
                    }

                    // Read definition name
                    text.readToken(&token);
                    if (token.length == 0)
                    {
                        parser->onError("Incorrect shader permutation. Definition name is empty.");
                        return;
                    }
                    TkString name = token.toString();

                    // Read '=' character
                    if (token.separator != Separator('='))
                    {
                        if (token.separator.isWhiteSpace())
                            text.eatWhiteSpaces();
                        if (text.peekChar() != '=')
                        {
                            parser->onError("Incorrect shader permutation. Missing \'='\' character for definition value.");
                            return;
                        }
                    }

                    // Read definition value
                    text.readToken(&token);
                    if (token.length == 0)
                    {
                        parser->onError("Incorrect shader permutation. Definition value is empty.");
                        return;
                    }
                    TkString value = token.toString();

                    // Read ',' or ')' character (depends on parameter index)
                    char checkChar = (paramIndex == m_startTokenPermutationSize - 1) ? ')' : ',';
                    if (token.separator != Separator(checkChar))
                    {
                        parser->onError("Incorrect shader permutation declaration.");
                        return;
                    }

                    // Check if hasn't been already defined in that permutation
                    if (current.hasDefinition(permutationIndex, name))
                    {
                        parser->onError(TkString::format("Incorrect shader function permutation definition. Already defined \'%s\'.", name));
                        return;
                    }

                    // Add entry to the meta
                    permutation.entries.add({name, value});
                }
            }
        };

        // Shader function flag reader
        class FlagReader : public ITokenReader
        {
        protected:
            ShaderFunctionReader *m_parent;
            Token m_startToken;

        public:
            FlagReader(ShaderFunctionReader *parent)
                : m_parent(parent), m_startToken("META_FLAG")
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

                // Shader Flag type
                text.readToken(&token);
                current.flags |= parseShaderFlags(token);
            }
        };

    protected:
        PermutationReader *m_permutationReader;

        ShaderFunctionReader()
        {
            ShaderMetaReaderType::m_childReaders.add(m_permutationReader = TkMemory::newObject<PermutationReader>(TkMemory::getDefault(), this));
            ShaderMetaReaderType::m_childReaders.add(TkMemory::newObject<FlagReader>(TkMemory::getDefault(), this));
        }

        ~ShaderFunctionReader() = default;

    protected:
        void onParseBefore(IShaderParser *parser, Reader &text) override
        {
            Token token;

            // Clear current meta
            auto &current = ShaderMetaReaderType::m_current;
            current.name.clear();
            current.permutations.clear();
            current.flags = GfxShaderFlags::kDefault;
            current.minFeatureLevel = GfxFeatureLevel::kES2;
            m_permutationReader->clear();

            // Here we read '(x, y)\n' where 'x' is a shader function 'visible' flag, and 'y' is mini feature level
            text.readToken(&token);
            token = parser->getMacros().getValue(token);
            if (token == "true" || token == "1")
            {
                // Visible shader
            }
            else if (token == "false" || token == "0" || token.length == 0)
            {
                // Hidden shader
                current.flags = GfxShaderFlags::kHidden;
            }
            else
            {
                // Undefined macro (fallback into hidden shader)
                current.flags = GfxShaderFlags::kHidden;
                // parser->OnError(String::Format(TEXT("Invalid shader function \'isVisible\' option value \'{0}\'."), String(token.toString())));
                // return;
            }
            text.readToken(&token);
            token = parser->getMacros().getValue(token);
            struct MinFeatureLevel
            {
                GfxFeatureLevel level;
                const char *token;
            };
            MinFeatureLevel levels[] =
                {
                    {GfxFeatureLevel::kES2, "FEATURE_LEVEL_ES2"},
                    {GfxFeatureLevel::kES3, "FEATURE_LEVEL_ES3"},
                    {GfxFeatureLevel::kES3_1, "FEATURE_LEVEL_ES3_1"},
                    {GfxFeatureLevel::kSM4, "FEATURE_LEVEL_SM4"},
                    {GfxFeatureLevel::kSM5, "FEATURE_LEVEL_SM5"},
                };
            bool missing = true;
            for (i32 i = 0; i < lengthOf(levels); i++)
            {
                if (token == levels[i].token)
                {
                    current.minFeatureLevel = levels[i].level;
                    missing = false;
                    break;
                }
            }
            if (missing)
            {
                parser->onError("Invalid shader function \'minFeatureLevel\' option value.");
                return;
            }

            // Read rest of the line
            text.readLine();
        }

        void onParseAfter(IShaderParser *parser, Reader &text) override
        {
            auto &current = ShaderMetaReaderType::m_current;

            // Validate amount of permutations
            if (current.permutations.count() > SHADER_PERMUTATIONS_MAX_COUNT)
            {
                parser->onError(TkString::format("Function \'%s\' uses too many permutations. Maximum allowed amount is {1}.", current.name, SHADER_PERMUTATIONS_MAX_COUNT));
                return;
            }

            // Check if shader has no permutations
            if (current.permutations.isEmpty())
            {
                // Just add blank permutation (rest of the code will work without any hacks for empty permutations list)
                current.permutations.add(ShaderPermutation());
            }

            // Check if use this shader program
            if ((current.flags & GfxShaderFlags::kHidden) == (GfxShaderFlags)0 && current.minFeatureLevel <= parser->getFeatureLevel())
            {
                // Cache read function
                ShaderMetaReaderType::m_cache.add(current);
            }
        }
    };
}

#define DECLARE_SHADER_META_READER_HEADER(tokenName, shaderMetaMemberCollection) \
public:                                                                          \
    bool checkStartToken(const Token &token) override                            \
    {                                                                            \
        return token == tokenName;                                               \
    }                                                                            \
    void flushCache(IShaderParser *parser, ShaderMeta *result) override          \
    {                                                                            \
        result->shaderMetaMemberCollection.add(m_cache);                          \
    }
