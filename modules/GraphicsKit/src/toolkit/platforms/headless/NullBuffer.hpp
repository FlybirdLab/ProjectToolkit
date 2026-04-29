/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxBuffer.hpp>

#if GFX_API_NULL

namespace toolkit
{

    // GPU buffer for Null backend.
    class NullBuffer : public GfxBuffer
    {
    public:
        GfxBufferView *view() const override
        {
            return nullptr;
        }

        void *map(GfxResourceMapMode mode) override
        {
            return nullptr;
        }

        void unmap() override
        {
        }

    protected:
        bool onInit() override
        {
            return false;
        }
    };

}

#endif