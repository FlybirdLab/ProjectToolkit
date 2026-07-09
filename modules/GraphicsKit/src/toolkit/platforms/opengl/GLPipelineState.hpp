/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxPipelineState.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Pipeline state for the OpenGL fallback backend.
    //
    // Wraps a linked GL program object. Phase 3 only supports VS + PS (no HS/
    // DS/GS/CS); other stages are rejected by init().
    //
    // m_desc is a value-copy of the description so GLContext can re-read the
    // primitive topology / blend / depth / cull settings at setState() time.
    class GLPipelineState : public GfxPipelineState
    {
    public:
        bool isValid() const override;
        bool init(const Description &desc) override;

        // Program object (== GLuint). 0 when invalid / released.
        u32 program() const { return m_program; }

        // Cached description -- callers (GLContext) need this to apply blend /
        // depth / cull / primitive-topology state on setState().
        const Description &desc() const { return m_desc; }

    protected:
        void onReleaseData() override;

    private:
        Description m_desc = {};
        u32 m_program = 0;
        bool m_valid = false;
    };

}

#endif
