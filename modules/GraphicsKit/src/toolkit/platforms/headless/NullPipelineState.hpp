/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxPipelineState.hpp>

#if GFX_API_NULL

namespace toolkit
{
    // Graphics pipeline state object for Null backend.
    class NullPipelineState : public GfxPipelineState
    {
    public:
        bool isValid() const override
        {
            return false;
        }
    };
}

#endif