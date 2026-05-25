/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLUtils.hpp"

#if GFX_API_OPENGL || GFX_API_OPENGLES

#include "toolkit/debug/Logger.hpp"

#if GFX_API_OPENGL
#include <toolkit/platforms/opengl/GLNative.hpp>
#include <cstdio>
#endif

namespace toolkit
{

    const char *GLBackendName()
    {
#if GFX_API_OPENGL
        return "OpenGL 4.1 Core";
#else
        return "OpenGL ES 3.0";
#endif
    }

    void GLReportUnsupported(const char *apiName)
    {
        LogError("OpenGL fallback backend does not support: %s", apiName ? apiName : "<unknown>");
    }

#if GFX_API_OPENGL

    namespace
    {
        const char *safeGLString(GLenum name)
        {
            const GLubyte *s = glGetString(name);
            return s ? reinterpret_cast<const char *>(s) : "<null>";
        }
    }

    void GLLogVersionInfo()
    {
        LogInfo("GL_VENDOR: %s", safeGLString(GL_VENDOR));
        LogInfo("GL_RENDERER: %s", safeGLString(GL_RENDERER));
        LogInfo("GL_VERSION: %s", safeGLString(GL_VERSION));
        LogInfo("GL_SHADING_LANGUAGE_VERSION: %s", safeGLString(GL_SHADING_LANGUAGE_VERSION));
    }

    void GLQueryVersion(int &outMajor, int &outMinor)
    {
        outMajor = 0;
        outMinor = 0;

        // Prefer glGetIntegerv: defined since GL 3.0 and what the spec recommends.
        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        if (major > 0)
        {
            outMajor = major;
            outMinor = minor;
            return;
        }

        // Fallback: parse "X.Y" prefix of GL_VERSION.
        const char *ver = safeGLString(GL_VERSION);
        if (!ver || ver[0] == '\0')
        {
            LogWarn("GLQueryVersion: GL_VERSION returned null/empty");
            return;
        }
        int a = 0, b = 0;
        if (std::sscanf(ver, "%d.%d", &a, &b) == 2)
        {
            outMajor = a;
            outMinor = b;
        }
        else
        {
            LogWarn("GLQueryVersion: could not parse GL_VERSION '%s'", ver);
        }
    }

    bool GLCheckMinimumVersion(int major, int minor)
    {
        int gotMajor = 0, gotMinor = 0;
        GLQueryVersion(gotMajor, gotMinor);
        const bool ok = (gotMajor > major) || (gotMajor == major && gotMinor >= minor);
        if (!ok)
        {
            LogWarn("OpenGL %d.%d required but got %d.%d", major, minor, gotMajor, gotMinor);
        }
        return ok;
    }

    bool GLGetVertexAttribFormat(PixelFormat format, GLVertexAttribFormat &out)
    {
        switch (format)
        {
        case PixelFormat::kR32_Float:
            out = {1, GL_FLOAT, false, false, 4};
            return true;
        case PixelFormat::kR32G32_Float:
            out = {2, GL_FLOAT, false, false, 8};
            return true;
        case PixelFormat::kR32G32B32_Float:
            out = {3, GL_FLOAT, false, false, 12};
            return true;
        case PixelFormat::kR32G32B32A32_Float:
            out = {4, GL_FLOAT, false, false, 16};
            return true;

        case PixelFormat::kR8G8B8A8_UNorm:
            out = {4, GL_UNSIGNED_BYTE, true, false, 4};
            return true;
        case PixelFormat::kR8G8B8A8_UInt:
            out = {4, GL_UNSIGNED_BYTE, false, true, 4};
            return true;

        case PixelFormat::kR16G16_Float:
            out = {2, GL_HALF_FLOAT, false, false, 4};
            return true;
        case PixelFormat::kR16G16B16A16_Float:
            out = {4, GL_HALF_FLOAT, false, false, 8};
            return true;

        default:
            GLReportUnsupported("vertex attribute format");
            return false;
        }
    }

    bool GLGetTextureFormatInfo(PixelFormat format, GLTextureFormatInfo &out)
    {
        out = GLTextureFormatInfo{};
        switch (format)
        {
        case PixelFormat::kR8_UNorm:
            out.internalFormat = GL_R8;
            out.format = GL_RED;
            out.type = GL_UNSIGNED_BYTE;
            out.renderable = true;
            return true;
        case PixelFormat::kR8G8_UNorm:
            out.internalFormat = GL_RG8;
            out.format = GL_RG;
            out.type = GL_UNSIGNED_BYTE;
            out.renderable = true;
            return true;
        case PixelFormat::kR8G8B8A8_UNorm:
            out.internalFormat = GL_RGBA8;
            out.format = GL_RGBA;
            out.type = GL_UNSIGNED_BYTE;
            out.renderable = true;
            return true;
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
            out.internalFormat = GL_SRGB8_ALPHA8;
            out.format = GL_RGBA;
            out.type = GL_UNSIGNED_BYTE;
            out.renderable = true;
            return true;
        case PixelFormat::kB8G8R8A8_UNorm:
            // Sized internal format is still RGBA8; we feed it BGRA so the
            // driver swaps channels at upload.
            out.internalFormat = GL_RGBA8;
            out.format = GL_BGRA;
            out.type = GL_UNSIGNED_BYTE;
            out.renderable = true;
            return true;
        case PixelFormat::kR16G16B16A16_Float:
            out.internalFormat = GL_RGBA16F;
            out.format = GL_RGBA;
            out.type = GL_HALF_FLOAT;
            out.renderable = true;
            return true;
        case PixelFormat::kR32_Float:
            out.internalFormat = GL_R32F;
            out.format = GL_RED;
            out.type = GL_FLOAT;
            out.renderable = true;
            return true;
        case PixelFormat::kR32G32_Float:
            out.internalFormat = GL_RG32F;
            out.format = GL_RG;
            out.type = GL_FLOAT;
            out.renderable = true;
            return true;
        case PixelFormat::kR32G32B32A32_Float:
            out.internalFormat = GL_RGBA32F;
            out.format = GL_RGBA;
            out.type = GL_FLOAT;
            out.renderable = true;
            return true;

        // ---- Depth / depth-stencil formats (Phase 5) --------------------
        case PixelFormat::kD16_UNorm:
            out.internalFormat = GL_DEPTH_COMPONENT16;
            out.format = GL_DEPTH_COMPONENT;
            out.type = GL_UNSIGNED_SHORT;
            out.renderable = true;
            out.filterable = false;
            out.depth = true;
            return true;
        case PixelFormat::kD24_UNorm_S8_UInt:
            out.internalFormat = GL_DEPTH24_STENCIL8;
            out.format = GL_DEPTH_STENCIL;
            out.type = GL_UNSIGNED_INT_24_8;
            out.renderable = true;
            out.filterable = false;
            out.depth = true;
            out.stencil = true;
            return true;
        case PixelFormat::kD32_Float:
            out.internalFormat = GL_DEPTH_COMPONENT32F;
            out.format = GL_DEPTH_COMPONENT;
            out.type = GL_FLOAT;
            out.renderable = true;
            out.filterable = false;
            out.depth = true;
            return true;

        default:
            GLReportUnsupported("texture format");
            return false;
        }
    }

    bool GLCheckError(const char *where)
    {
        bool ok = true;
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            LogError("OpenGL error at %s: 0x%X", where ? where : "<unknown>", err);
            ok = false;
        }
        return ok;
    }

#endif // GFX_API_OPENGL

}

#endif
