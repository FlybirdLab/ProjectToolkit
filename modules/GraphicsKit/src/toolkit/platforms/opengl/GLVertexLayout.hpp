/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxVertexLayout.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    class GLVertexLayout : public GfxVertexLayout
    {
    public:
        GLVertexLayout(const Elements &elements)
            : GfxVertexLayout()
        {
            setElements(elements, false);
        }
    };

}

#endif
