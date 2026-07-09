/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxResource.hpp>

namespace toolkit
{
    // Constant Buffer object used to pass parameters to the shaders on GPU
    class TK_API GfxConstantBuffer : public GfxResource
    {
    public:
        // Gets the buffer size (in bytes).
        TK_FORCE_INLINE u32 getSize() const
        {
            return m_size;
        }

        GfxResType getType() const override
        {
            return GfxResType::kBuffer;
        }

    protected:
        u32 m_size = 0;
    };

}
