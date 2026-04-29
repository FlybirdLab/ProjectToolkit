/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxSampler.hpp>

#if GFX_API_NULL

namespace toolkit
{

    // Sampler object for Null backend.
    class NullSampler : public GfxSampler
    {
    protected:
        bool onInit() override
        {
            return false;
        }
    };

}

#endif