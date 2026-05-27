/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include "ITokenReader.hpp"

namespace toolkit {
    
    // Interface for shader functions readers like Pixel Shader readers or Constant Buffer readers
    class IShaderFunctionReader : public ITokenReader
    {
    public:
        virtual ~IShaderFunctionReader() = default;

    public:
        // Collects shader function reader results to the final Shader Meta
        virtual void collectResults(IShaderParser* parser, class ShaderMeta* result) = 0;
    };
}