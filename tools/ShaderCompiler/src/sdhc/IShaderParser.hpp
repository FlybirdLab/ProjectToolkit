/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <sdhc/common.hpp>

namespace toolkit
{
    struct ParserMacros
    {
        const TkArray<ShaderMacro> *data;

        ParserMacros(const TkArray<ShaderMacro> &d)
        {
            data = &d;
        }

        Token getValue(Token &token) const
        {
            for (i32 i = 0; i < data->count(); i++)
            {
                if (token == data->at(i).name)
                {
                    // Use macro value
                    return Token(data->at(i).definition);
                }
            }

            // Fallback to token
            return token;
        }
    };

    // Interface describing shader source code parser
    class IShaderParser
    {
    public:
        virtual ~IShaderParser() = default;

    public:
        // Get the parser feature level of the target platform graphics backend.
        virtual GfxFeatureLevel getFeatureLevel() const = 0;
        // Get the parser macros.
        virtual ParserMacros getMacros() const = 0;
        // Get value indicating that shader processing operation failed
        virtual bool failed() const = 0;
        // Get source code reader
        virtual Reader &getReader() = 0;

        // Event send to the parser on reading shader source code error
        virtual void onError(const TkString &message) = 0;
        // Event send to the parser on reading shader source code warning
        virtual void onWarning(const TkString &message) = 0;
    };
}