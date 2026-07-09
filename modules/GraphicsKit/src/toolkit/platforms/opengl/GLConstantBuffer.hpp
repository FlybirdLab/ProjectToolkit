/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxConstantBuffer.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Constant buffer for the OpenGL fallback backend.
    //
    // Backed by GL_UNIFORM_BUFFER with GL_DYNAMIC_DRAW usage. update() uses
    // glBufferSubData so the buffer can be re-uploaded each frame without
    // re-allocating.
    class GLConstantBuffer : public GfxConstantBuffer
    {
    public:
        explicit GLConstantBuffer(u32 size);

        bool init();
        void update(const void *data);

        // GL buffer object (== GLuint). 0 when not allocated.
        u32 handle() const { return m_handle; }

    protected:
        void onReleaseData() override;

    private:
        u32 m_handle = 0;
    };

}

#endif
