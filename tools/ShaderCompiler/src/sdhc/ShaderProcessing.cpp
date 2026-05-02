/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ShaderFuncReader.CB.hpp"
#include "ShaderFuncReader.VS.hpp"
#include "ShaderFuncReader.HS.hpp"
#include "ShaderFuncReader.DS.hpp"
#include "ShaderFuncReader.GS.hpp"
#include "ShaderFuncReader.PS.hpp"
#include "ShaderFuncReader.CS.hpp"

#include "toolkit/debug/Profiler.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    Parser::Parser(const TkString &targetName, const char *source, i32 sourceLength, ParserMacros macros, GfxFeatureLevel featureLevel)
        : m_failed(false), m_targetName(targetName), m_text(source, sourceLength), m_macros(macros), m_featureLevel(featureLevel)
    {
    }
    Parser::~Parser() = default;

    bool Parser::process(const TkString &targetName, const char *source, i32 sourceLength, ParserMacros macros, GfxFeatureLevel featureLevel, ShaderMeta *result)
    {
        PROFILE_NAMED("Shader.Parse");
        Parser parser(targetName, source, sourceLength, macros, featureLevel);
        parser.process(result);
        return !parser.failed();
    }

    void Parser::process(ShaderMeta *result)
    {
        init();

        m_failed = process();

        if (!m_failed)
            m_failed = collectResults(result);
    }

    void Parser::init()
    {
        // Init text processing tokens for hlsl language
        m_text.setupHLSL();

        // Init shader functions readers
        m_childReaders.add(TkMemory::newObject<ConstantBufferReader>());
        m_childReaders.add(TkMemory::newObject<VertexShaderFunctionReader>());
        m_childReaders.add(TkMemory::newObject<HullShaderFunctionReader>());
        m_childReaders.add(TkMemory::newObject<DomainShaderFunctionReader>());
        m_childReaders.add(TkMemory::newObject<GeometryShaderFunctionReader>());
        m_childReaders.add(TkMemory::newObject<PixelShaderFunctionReader>());
        m_childReaders.add(TkMemory::newObject<ComputeShaderFunctionReader>());
    }

    bool Parser::process()
    {
        const Token defineToken("#define");

        // Read whole source code
        Token token;
        while (m_text.canRead())
        {
            m_text.readToken(&token);

            // Preprocessor definition
            if (token == defineToken)
            {
                // Skip
                m_text.readLine();
            }
            else
            {
                // Call children
                processChildren(token, this);
            }
        }

        return false;
    }

    bool Parser::collectResults(ShaderMeta *result)
    {
        // Collect results from all the readers
        for (i32 i = 0; i < m_childReaders.count(); i++)
            m_childReaders[i]->collectResults(this, result);

        return false;
    }

    void Parser::onError(const TkString &message)
    {
        // Set flag
        m_failed = true;

        // Send event
        LogError("Processing shader '%s' error at line %i. %s", m_targetName, m_text.getLine(), message);
    }

    void Parser::onWarning(const TkString &message)
    {
        // Send event
        LogWarn("Processing shader '%s' warning at line %i. %s", m_targetName, m_text.getLine(), message);
    }

    VertexShaderMeta::InputType parseInputType(const Token &token)
    {
        struct InputDesc
        {
            VertexShaderMeta::InputType e;
            const char *s;
        };

#define _PARSE_ENTRY(x)                    \
    {                                      \
        VertexShaderMeta::InputType::k##x, #x \
    }
        const InputDesc formats[] =
            {
                _PARSE_ENTRY(Invalid),
                _PARSE_ENTRY(POSITION),
                _PARSE_ENTRY(COLOR),
                _PARSE_ENTRY(TEXCOORD),
                _PARSE_ENTRY(NORMAL),
                _PARSE_ENTRY(TANGENT),
                _PARSE_ENTRY(BITANGENT),
                _PARSE_ENTRY(ATTRIBUTE),
                _PARSE_ENTRY(BLENDINDICES),
                _PARSE_ENTRY(BLENDWEIGHTS),
                _PARSE_ENTRY(BLENDWEIGHT), // [Deprecated in v1.10]
            };
#undef _PARSE_ENTRY

        for (i32 i = 0; i < lengthOf(formats); i++)
        {
            if (token == formats[i].s)
                return formats[i].e;
        }

        return VertexShaderMeta::InputType::kInvalid;
    }

    PixelFormat parsePixelFormat(const Token &token)
    {
        struct DataDesc
        {
            PixelFormat e;
            const char *s;
        };

#define _PARSE_ENTRY(x)    \
    {                      \
        PixelFormat::k##x, #x \
    }
        const DataDesc formats[] =
            {
                _PARSE_ENTRY(Unknown),
                _PARSE_ENTRY(R32G32B32A32_Float),
                _PARSE_ENTRY(R32G32B32A32_UInt),
                _PARSE_ENTRY(R32G32B32A32_SInt),
                _PARSE_ENTRY(R32G32B32_Float),
                _PARSE_ENTRY(R32G32B32_UInt),
                _PARSE_ENTRY(R32G32B32_SInt),
                _PARSE_ENTRY(R16G16B16A16_Float),
                _PARSE_ENTRY(R16G16B16A16_UNorm),
                _PARSE_ENTRY(R16G16B16A16_UInt),
                _PARSE_ENTRY(R16G16B16A16_SNorm),
                _PARSE_ENTRY(R16G16B16A16_SInt),
                _PARSE_ENTRY(R32G32_Float),
                _PARSE_ENTRY(R32G32_UInt),
                _PARSE_ENTRY(R32G32_SInt),
                _PARSE_ENTRY(R10G10B10A2_UNorm),
                _PARSE_ENTRY(R10G10B10A2_UInt),
                _PARSE_ENTRY(R11G11B10_Float),
                _PARSE_ENTRY(R8G8B8A8_UNorm),
                _PARSE_ENTRY(R8G8B8A8_UNorm_sRGB),
                _PARSE_ENTRY(R8G8B8A8_UInt),
                _PARSE_ENTRY(R8G8B8A8_SNorm),
                _PARSE_ENTRY(R8G8B8A8_SInt),
                _PARSE_ENTRY(R16G16_Float),
                _PARSE_ENTRY(R16G16_UNorm),
                _PARSE_ENTRY(R16G16_UInt),
                _PARSE_ENTRY(R16G16_SNorm),
                _PARSE_ENTRY(R16G16_SInt),
                _PARSE_ENTRY(R32_Float),
                _PARSE_ENTRY(R32_UInt),
                _PARSE_ENTRY(R32_SInt),
                _PARSE_ENTRY(R8G8_UNorm),
                _PARSE_ENTRY(R8G8_UInt),
                _PARSE_ENTRY(R8G8_SNorm),
                _PARSE_ENTRY(R8G8_SInt),
                _PARSE_ENTRY(R16_Float),
                _PARSE_ENTRY(R16_UNorm),
                _PARSE_ENTRY(R16_UInt),
                _PARSE_ENTRY(R16_SNorm),
                _PARSE_ENTRY(R16_SInt),
                _PARSE_ENTRY(R8_UNorm),
                _PARSE_ENTRY(R8_UInt),
                _PARSE_ENTRY(R8_SNorm),
                _PARSE_ENTRY(R8_SInt),
                _PARSE_ENTRY(A8_UNorm),
                _PARSE_ENTRY(R1_UNorm),
                _PARSE_ENTRY(R8G8_B8G8_UNorm),
                _PARSE_ENTRY(G8R8_G8B8_UNorm),
                _PARSE_ENTRY(BC1_UNorm),
                _PARSE_ENTRY(BC1_UNorm_sRGB),
                _PARSE_ENTRY(BC2_UNorm),
                _PARSE_ENTRY(BC2_UNorm_sRGB),
                _PARSE_ENTRY(BC3_UNorm),
                _PARSE_ENTRY(BC3_UNorm_sRGB),
                _PARSE_ENTRY(BC4_UNorm),
                _PARSE_ENTRY(BC4_SNorm),
                _PARSE_ENTRY(BC5_UNorm),
                _PARSE_ENTRY(BC5_SNorm),
                _PARSE_ENTRY(B5G6R5_UNorm),
                _PARSE_ENTRY(B5G5R5A1_UNorm),
                _PARSE_ENTRY(B8G8R8A8_UNorm),
                _PARSE_ENTRY(B8G8R8X8_UNorm),
                _PARSE_ENTRY(B8G8R8A8_UNorm_sRGB),
                _PARSE_ENTRY(B8G8R8X8_UNorm_sRGB),
                _PARSE_ENTRY(BC6H_Uf16),
                _PARSE_ENTRY(BC6H_Sf16),
                _PARSE_ENTRY(BC7_UNorm),
                _PARSE_ENTRY(BC7_UNorm_sRGB),
            };
#undef _PARSE_ENTRY

        for (i32 i = 0; i < lengthOf(formats); i++)
        {
            if (token.equalsIgnoreCase(formats[i].s))
                return formats[i].e;
        }

        return PixelFormat::kUnknown;
    }

    GfxShaderFlags parseShaderFlags(const Token &token)
    {
        struct DataDesc
        {
            GfxShaderFlags e;
            const char *s;
        };

#define _PARSE_ENTRY(x)    \
    {                      \
        GfxShaderFlags::k##x, #x \
    }
        const DataDesc data[] =
            {
                _PARSE_ENTRY(Default),
                _PARSE_ENTRY(Hidden),
                _PARSE_ENTRY(NoFastMath),
                _PARSE_ENTRY(VertexToGeometryShader),
            };
        static_assert(lengthOf(data) == 4, "Invalid amount of Shader Flag data entries.");
#undef _PARSE_ENTRY

        for (i32 i = 0; i < lengthOf(data); i++)
        {
            if (token.equalsIgnoreCase(data[i].s))
                return data[i].e;
        }

        return GfxShaderFlags::kDefault;
    }

}
