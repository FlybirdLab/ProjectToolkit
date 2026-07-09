/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxVertexLayout.hpp>

#if GFX_API_NULL

namespace toolkit
{

    // Vertex layout for Null backend.
    class NullVertexLayout : public GfxVertexLayout
    {
    public:
        NullVertexLayout(const Elements &elements)
            : GfxVertexLayout()
        {
            setElements(elements, false);
        }
    };

}

#endif