/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxTimerQuery.hpp>

#if GFX_API_NULL

namespace toolkit
{

    // GPU timer query object for Null backend.
    class NullTimeQuery : public GfxTimerQuery
    {
    public:
        void begin() override
        {
        }

        void end() override
        {
        }

        bool hasResult() override
        {
            return false;
        }

        float getResult() override
        {
            return 0.0f;
        }
    };

}

#endif