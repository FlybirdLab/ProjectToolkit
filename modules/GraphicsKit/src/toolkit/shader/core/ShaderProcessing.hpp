/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "IShaderFunctionReader.hpp"
#include "IShaderParser.hpp"
#include "ITokenReadersContainer.hpp"
#include <sdhc/common.hpp>
#include <sdhc/ShaderMeta.hpp>

namespace toolkit
{
    extern VertexShaderMeta::InputType parseInputType(const Token &token);
    extern PixelFormat parsePixelFormat(const Token &token);
    extern GfxShaderFlags parseShaderFlags(const Token &token);

    // Shader files meta data processing tool
    class Parser : public IShaderParser, public ITokenReadersContainerBase<IShaderFunctionReader>
    {
    private:
        bool m_failed;
        TkString m_targetName;
        Reader m_text;
        ParserMacros m_macros;
        GfxFeatureLevel m_featureLevel;

    private:
        Parser(const TkString &targetName, const char *source, i32 sourceLength, ParserMacros macros, GfxFeatureLevel featureLevel);
        ~Parser();

    public:
        // Process shader source code and generate metadata
        static bool process(const TkString &targetName, const char *source, i32 sourceLength, ParserMacros macros, GfxFeatureLevel featureLevel, ShaderMeta *result);
        // Process shader source code and generate metadata
        void process(ShaderMeta *result);

    private:
        void init();
        bool process();
        bool collectResults(ShaderMeta *result);

    public:
        GfxFeatureLevel getFeatureLevel() const override
        {
            return m_featureLevel;
        }

        bool failed() const override
        {
            return m_failed;
        }

        Reader &getReader() override
        {
            return m_text;
        }

        ParserMacros getMacros() const override
        {
            return m_macros;
        }

        void onError(const TkString &message) override;
        void onWarning(const TkString &message) override;
    };
}
