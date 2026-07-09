/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxBuffer.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // OpenGL buffer wrapper. Phase 2 supports vertex / index / uniform buffers
    // only; structured / raw / UA flagged buffers report unsupported and fail
    // onInit.
    //
    // m_handle / m_target are stored as raw u32 so this header does not need
    // to pull in <glad/glad.h>. The .cpp casts them back to GLuint / GLenum.
    class GLBuffer : public GfxBuffer
    {
    public:
        GfxBufferView *view() const override
        {
            return nullptr;
        }

        void *map(GfxResourceMapMode mode) override;
        void unmap() override;

        // Raw OpenGL handle (== GLuint). 0 when not allocated.
        u32 getNativeHandle() const { return m_handle; }
        // Raw target (== GLenum, e.g. GL_ARRAY_BUFFER).
        u32 getNativeTarget() const { return m_target; }

        // Stable accessors used by GLContext::bindVB / bindIB.
        u32 handle() const { return m_handle; }
        u32 target() const { return m_target; }
        u32 size() const { return m_desc.size; }

    protected:
        bool onInit() override;
        void onReleaseData() override;

    private:
        u32 m_handle = 0;
        u32 m_target = 0;
        bool m_mapped = false;
    };

}

#endif
