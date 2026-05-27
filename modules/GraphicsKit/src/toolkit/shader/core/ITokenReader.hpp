/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <sdhc/IShaderParser.hpp>

namespace toolkit {
    
    // Interface for objects that can read shader source code tokens
    class ITokenReader
    {
    public:
        virtual ~ITokenReader() = default;

    public:
        // Checks if given token can be processed by this reader
        virtual bool checkStartToken(const Token& token) = 0;
        // Start processing source after reading start token
        virtual void process(IShaderParser* parser, Reader& text) = 0;
    };
}