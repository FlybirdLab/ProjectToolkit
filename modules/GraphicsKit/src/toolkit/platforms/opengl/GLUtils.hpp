/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/graphics/utils/PixelFormat.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Human-readable name for log messages.
    const char *GLBackendName();

    // Log a single "API X is not supported by the OpenGL fallback backend" line.
    // Use this in stubs that must NOT silently succeed (compute / UAV / indirect
    // draw / SSBO / etc).
    void GLReportUnsupported(const char *apiName);

#if GFX_API_OPENGL
    // Logs GL_VENDOR / GL_RENDERER / GL_VERSION / GL_SHADING_LANGUAGE_VERSION.
    // Caller must have a current GL context, or this will be a no-op + warning.
    void GLLogVersionInfo();

    // Returns the currently-bound GL major.minor. Both outputs are 0 on failure.
    void GLQueryVersion(int &outMajor, int &outMinor);

    // Returns true if the current GL context advertises at least the given
    // major.minor. Logs a warning if not.
    bool GLCheckMinimumVersion(int major, int minor);

    // Resolved vertex attribute format. `type` and `sizeInBytes` are GL-side
    // values: type is GLenum (e.g. GL_FLOAT), sizeInBytes is the byte size of
    // one vertex element.
    struct GLVertexAttribFormat
    {
        i32 components;
        u32 type;
        bool normalized;
        bool integer;
        u32 sizeInBytes;
    };

    // Resolves a PixelFormat to its OpenGL vertex attribute description. Only
    // the formats actually used by Phase 3 vertex layouts are supported; other
    // formats log via GLReportUnsupported and return false.
    bool GLGetVertexAttribFormat(PixelFormat format, GLVertexAttribFormat &out);

    // Resolved GL texture format. `internalFormat` is the GL_R8/GL_RGBA8/... sized
    // internal format passed to glTexImage2D; `format` is the client pixel format
    // (GL_RGBA/GL_BGRA/...); `type` is the component type (GL_UNSIGNED_BYTE/...).
    struct GLTextureFormatInfo
    {
        u32 internalFormat = 0;
        u32 format = 0;
        u32 type = 0;
        bool compressed = false;
        bool renderable = false;
        bool filterable = true;
        bool depth = false;
        bool stencil = false;
    };

    // Resolves a PixelFormat to its OpenGL texture description. Only formats
    // used by Phase 4 textures are supported; compressed / depth / typeless /
    // integer texture formats fall through to GLReportUnsupported.
    bool GLGetTextureFormatInfo(PixelFormat format, GLTextureFormatInfo &out);

    // Drains glGetError. Logs each pending error code with `where` as the
    // operation tag and returns true iff no errors were pending.
    bool GLCheckError(const char *where);
#endif

}

#endif
