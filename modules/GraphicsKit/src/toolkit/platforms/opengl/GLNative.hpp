/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/graphics/environment.hpp>

#if GFX_API_OPENGL

// Pull in glad before any platform GL headers. glad ships its own gl.h shim
// so users must not include <GL/gl.h> independently in this backend.
#include <glad/glad.h>

#if TK_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
#elif TK_APPLE
    #include <OpenGL/OpenGL.h>
#endif

#endif // GFX_API_OPENGL
