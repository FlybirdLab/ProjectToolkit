/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLSampler.hpp"

#if GFX_API_OPENGL

#include "GLNative.hpp"
#include "GLUtils.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    namespace
    {
        void applyAddressMode(GLuint sampler, GLenum coord, GfxSamplerAddressMode mode)
        {
            GLint glMode = GL_REPEAT;
            switch (mode)
            {
            case GfxSamplerAddressMode::kWrap:
                glMode = GL_REPEAT;
                break;
            case GfxSamplerAddressMode::kClamp:
                glMode = GL_CLAMP_TO_EDGE;
                break;
            case GfxSamplerAddressMode::kMirror:
                glMode = GL_MIRRORED_REPEAT;
                break;
            case GfxSamplerAddressMode::kBorder:
                glMode = GL_CLAMP_TO_BORDER;
                break;
            default:
                break;
            }
            glSamplerParameteri(sampler, coord, glMode);
        }

        void applyBorderColor(GLuint sampler, GfxSamplerBorderColor color)
        {
            GLfloat rgba[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            switch (color)
            {
            case GfxSamplerBorderColor::kTransparentBlack:
                rgba[0] = rgba[1] = rgba[2] = 0.0f;
                rgba[3] = 0.0f;
                break;
            case GfxSamplerBorderColor::kOpaqueBlack:
                rgba[0] = rgba[1] = rgba[2] = 0.0f;
                rgba[3] = 1.0f;
                break;
            case GfxSamplerBorderColor::kOpaqueWhite:
                rgba[0] = rgba[1] = rgba[2] = 1.0f;
                rgba[3] = 1.0f;
                break;
            default:
                break;
            }
            glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, rgba);
        }
    }

    GLSampler::~GLSampler()
    {
        GLSampler::onReleaseData();
    }

    bool GLSampler::onInit()
    {
        const auto &desc = m_desc;

        GLuint s = 0;
        glGenSamplers(1, &s);
        if (s == 0)
        {
            LogError("glGenSamplers failed");
            return false;
        }

        // Filter
        switch (desc.filter)
        {
        case GfxSamplerFilter::kPoint:
            glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case GfxSamplerFilter::kBilinear:
            glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case GfxSamplerFilter::kTrilinear:
        case GfxSamplerFilter::kAnisotropic:
        default:
            glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }

        // Address modes
        applyAddressMode(s, GL_TEXTURE_WRAP_S, desc.addressU);
        applyAddressMode(s, GL_TEXTURE_WRAP_T, desc.addressV);
        applyAddressMode(s, GL_TEXTURE_WRAP_R, desc.addressW);

        // LOD bias / clamps
        glSamplerParameterf(s, GL_TEXTURE_LOD_BIAS, desc.mipBias);
        glSamplerParameterf(s, GL_TEXTURE_MIN_LOD, desc.minMipLevel);
        glSamplerParameterf(s, GL_TEXTURE_MAX_LOD, desc.maxMipLevel);

        // Comparison function
        if (desc.comparisonFunction == GfxSamplerCompareFunction::kLess)
        {
            glSamplerParameteri(s, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glSamplerParameteri(s, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        }
        else
        {
            glSamplerParameteri(s, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        }

        // Border color (only meaningful if any address is kBorder, but always
        // safe to set on Desktop GL).
        applyBorderColor(s, desc.borderColor);

        // Anisotropy: query the GL_EXT_texture_filter_anisotropic extension at
        // runtime via glad. If not supported, fall through silently.
        if (desc.filter == GfxSamplerFilter::kAnisotropic && desc.maxAnisotropy > 1)
        {
#ifdef GL_TEXTURE_MAX_ANISOTROPY
            GLfloat maxSupported = 1.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxSupported);
            const GLfloat clamped = (static_cast<GLfloat>(desc.maxAnisotropy) > maxSupported) ? maxSupported : static_cast<GLfloat>(desc.maxAnisotropy);
            glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY, clamped);
#elif defined(GL_TEXTURE_MAX_ANISOTROPY_EXT)
            GLfloat maxSupported = 1.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSupported);
            const GLfloat clamped = (static_cast<GLfloat>(desc.maxAnisotropy) > maxSupported) ? maxSupported : static_cast<GLfloat>(desc.maxAnisotropy);
            glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT, clamped);
#endif
        }

        if (!GLCheckError("GLSampler::onInit"))
        {
            glDeleteSamplers(1, &s);
            return false;
        }

        m_handle = static_cast<u32>(s);
        return true;
    }

    void GLSampler::onReleaseData()
    {
        if (m_handle)
        {
            const GLuint s = static_cast<GLuint>(m_handle);
            glDeleteSamplers(1, &s);
            m_handle = 0;
        }
        GfxSampler::onReleaseData();
    }

}

#elif GFX_API_OPENGLES

namespace toolkit
{
    GLSampler::~GLSampler() {}
    bool GLSampler::onInit() { return false; }
    void GLSampler::onReleaseData() { GfxSampler::onReleaseData(); }
}

#endif
