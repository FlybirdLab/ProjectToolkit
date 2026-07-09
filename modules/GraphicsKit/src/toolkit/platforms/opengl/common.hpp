/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/environment.hpp>

// First-stage OpenGL fallback backend. Desktop target is OpenGL 4.1 Core,
// the GLES variant targets ES 3.0. The backend is intentionally limited:
// no compute, no SSBO, no UAV, no indirect draw, no glTextureView.
//
// This header is the shared include for the OpenGL skeleton. It does NOT pull
// in any real GL headers yet, so the skeleton can be compiled before a GL
// loader (glad / GLEW) is wired up. Loader integration belongs to Phase 2.

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Forward declarations for the OpenGL backend.
    class GLAdapter;
    class GLDevice;
    class GLContext;
    class GLSwapChain;
    class GLBuffer;
    class GLTexture;
    class GLSampler;
    class GLShader;
    class GLShaderProgram;
    class GLPipelineState;
    class GLVertexLayout;
    class GLTimerQuery;
    class GLFramebufferCache;
    class GLStateCache;

}

#endif
