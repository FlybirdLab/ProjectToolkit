/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLShader.hpp"

#if GFX_API_OPENGL

#include "GLNative.hpp"
#include "GLShaderProgram.hpp"
#include "GLUtils.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/GLShaderSourceFormat.hpp"
#include "toolkit/memory/Memory.hpp"

namespace toolkit
{

    // ---- GLShaderProgramBase template body -------------------------------

    template <typename BaseType>
    void GLShaderProgramBase<BaseType>::destroyShader()
    {
        if (m_shader)
        {
            glDeleteShader(static_cast<GLuint>(m_shader));
            m_shader = 0;
        }
    }

    // Explicit instantiations so the template body lives in this .cpp.
    template void GLShaderProgramBase<GfxShaderProgramVS>::destroyShader();
    template void GLShaderProgramBase<GfxShaderProgramPS>::destroyShader();

    // ---- GLSL source bytecode header -------------------------------------

    namespace
    {
        // Pull a (const char*, size) out of bytecode. Supports both the typed
        // header format (Phase 6+: written by ShaderCompilerGLSL) and the
        // raw-GLSL fallback used in Phase 3/4 manual testing.
        bool extractGLSLSource(TkSpan<byte> bytecode, const char *&outSrc, GLint &outSize)
        {
            const byte *data = bytecode.get();
            const u32 size = static_cast<u32>(bytecode.length());

            if (size >= sizeof(gfx::GLShaderSourceHeader))
            {
                gfx::GLShaderSourceHeader header;
                TkMemory::memoryCopy(&header, data, sizeof(header));
                if (header.magic == gfx::GL_SHADER_SOURCE_MAGIC)
                {
                    if (header.version != gfx::GL_SHADER_SOURCE_VERSION)
                    {
                        LogError("GLSL shader source header version mismatch: got %u, expected %u", header.version, gfx::GL_SHADER_SOURCE_VERSION);
                        return false;
                    }
                    const u32 payloadAvail = size - sizeof(gfx::GLShaderSourceHeader);
                    if (header.sourceSize > payloadAvail)
                    {
                        LogError("GLSL shader source header reports %u bytes but only %u available", header.sourceSize, payloadAvail);
                        return false;
                    }
                    outSrc = reinterpret_cast<const char *>(data + sizeof(gfx::GLShaderSourceHeader));
                    outSize = static_cast<GLint>(header.sourceSize);
                    return true;
                }
            }

            // Fallback: bytecode is raw GLSL text.
            outSrc = reinterpret_cast<const char *>(data);
            outSize = static_cast<GLint>(size);
            return true;
        }

        GLuint compileGLSL(GLenum stage, const char *src, GLint srcSize, const char *shaderName)
        {
            const GLuint shader = glCreateShader(stage);
            if (shader == 0)
            {
                LogError("glCreateShader failed for stage 0x%04x", stage);
                return 0;
            }
            glShaderSource(shader, 1, &src, &srcSize);
            glCompileShader(shader);

            GLint status = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE)
            {
                GLint logLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
                if (logLen > 0)
                {
                    char *log = TkMemory::newArray<char>(static_cast<u32>(logLen));
                    if (log)
                    {
                        glGetShaderInfoLog(shader, logLen, nullptr, log);
                        LogError("GLSL compile failed (%s):\n%s", shaderName ? shaderName : "<unnamed>", log);
                        TkMemory::deleteArray(log, static_cast<u32>(logLen));
                    }
                }
                else
                {
                    LogError("GLSL compile failed (%s) with no info log", shaderName ? shaderName : "<unnamed>");
                }
                glDeleteShader(shader);
                return 0;
            }
            return shader;
        }
    }

    // ---- GLShader::createShaderProgram -----------------------------------

    GfxShaderProgram *GLShader::createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream)
    {
        const char *shaderName = TkString(initializer.name).getText();

        switch (type)
        {
        case ShaderStage::kVertex:
        {
            // The shader-cache writes the VS input/vertex layout right after
            // the VS bytecode; we must consume that stream slice now.
            GfxVertexLayout *inputLayout = nullptr;
            GfxVertexLayout *vertexLayout = nullptr;
            readVertexLayout(stream, inputLayout, vertexLayout);

            const char *src = nullptr;
            GLint srcSize = 0;
            if (!extractGLSLSource(bytecode, src, srcSize))
                return nullptr;
            const GLuint shader = compileGLSL(GL_VERTEX_SHADER, src, srcSize, shaderName);
            if (shader == 0)
                return nullptr;
            return TkMemory::newObject<GLShaderProgramVS>(TkMemory::getDefault(), initializer, static_cast<u32>(shader), static_cast<u32>(srcSize), inputLayout, vertexLayout);
        }
        case ShaderStage::kPixel:
        {
            const char *src = nullptr;
            GLint srcSize = 0;
            if (!extractGLSLSource(bytecode, src, srcSize))
                return nullptr;
            const GLuint shader = compileGLSL(GL_FRAGMENT_SHADER, src, srcSize, shaderName);
            if (shader == 0)
                return nullptr;
            return TkMemory::newObject<GLShaderProgramPS>(TkMemory::getDefault(), initializer, static_cast<u32>(shader), static_cast<u32>(srcSize));
        }
        case ShaderStage::kHull:
            GLReportUnsupported("GLShader: Hull shader stage");
            return nullptr;
        case ShaderStage::kDomain:
            GLReportUnsupported("GLShader: Domain shader stage");
            return nullptr;
        case ShaderStage::kGeometry:
            GLReportUnsupported("GLShader: Geometry shader stage");
            return nullptr;
        case ShaderStage::kCompute:
            GLReportUnsupported("GLShader: Compute shader stage");
            return nullptr;
        }
        return nullptr;
    }

}

#elif GFX_API_OPENGLES

namespace toolkit
{
    GfxShaderProgram *GLShader::createShaderProgram(ShaderStage, const GfxShaderProgramInitializer &, TkSpan<byte>, MemoryIStream &)
    {
        return nullptr;
    }
}

#endif
