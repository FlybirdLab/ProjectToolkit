/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxSampler.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    class GLSampler : public GfxSampler
    {
    public:
        ~GLSampler();

        // GL sampler object (== GLuint). 0 when not allocated.
        u32 handle() const { return m_handle; }

    protected:
        bool onInit() override;
        void onReleaseData() override;

    private:
        u32 m_handle = 0;
    };

}

#endif
