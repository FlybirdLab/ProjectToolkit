/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxResource.hpp>
#include <toolkit/graphics/GfxShaderProgram.hpp>
#include <toolkit/collection/HashMap.hpp>

namespace toolkit
{

// The runtime version of the shaders cache supported by the all graphics back-ends. The same for all the shader cache formats (easier to sync and validate).
#define GPU_SHADER_CACHE_VERSION 1

    class GfxConstantBuffer;

    // The GPU resource with shader programs that can run on the GPU and are able to perform rendering calculation using textures, vertices and other resources.
    class TK_API GfxShader : public GfxResource
    {
    public:
        // Creates the shader resource and loads its data from the bytes.
        virtual bool create(class MemoryIStream &stream);

    public:
        // Gets the vertex shader.
        TK_FORCE_INLINE class GfxShaderProgramVS *getVS(const TkStringView &name, i32 permutationIndex = 0) const
        {
            return static_cast<GfxShaderProgramVS *>(getShader(ShaderStage::kVertex, name, permutationIndex));
        }

        // Gets the hull shader.
        TK_FORCE_INLINE GfxShaderProgramHS *getHS(const TkStringView &name, i32 permutationIndex = 0) const
        {
#if TK_DEBUG
            return static_cast<GfxShaderProgramHS *>(getShader(ShaderStage::kHull, name, permutationIndex));
#else
            return nullptr;
#endif
        }

        // Gets domain shader.
        TK_FORCE_INLINE GfxShaderProgramDS *getDS(const TkStringView &name, i32 permutationIndex = 0) const
        {
#if TK_DEBUG
            return static_cast<GfxShaderProgramDS *>(getShader(ShaderStage::kDomain, name, permutationIndex));
#else
            return nullptr;
#endif
        }

        // Gets the geometry shader.
        TK_FORCE_INLINE GfxShaderProgramGS *getGS(const TkStringView &name, i32 permutationIndex = 0) const
        {
#if TK_DEBUG
            return static_cast<GfxShaderProgramGS *>(getShader(ShaderStage::kGeometry, name, permutationIndex));
#else
            return nullptr;
#endif
        }

        // Gets the pixel shader.
        TK_FORCE_INLINE GfxShaderProgramPS *getPS(const TkStringView &name, i32 permutationIndex = 0) const
        {
            return static_cast<GfxShaderProgramPS *>(getShader(ShaderStage::kPixel, name, permutationIndex));
        }

        // Gets the compute shader.
        TK_FORCE_INLINE GfxShaderProgramCS *getCS(const TkStringView &name, i32 permutationIndex = 0) const
        {
            return static_cast<GfxShaderProgramCS *>(getShader(ShaderStage::kCompute, name, permutationIndex));
        }

        // Gets the constant buffer.
        TK_FORCE_INLINE GfxConstantBuffer *getCB(i32 slot) const
        {
            TK_ASSERT(slot >= 0 && slot < lengthOf(m_constantBuffers));
            return m_constantBuffers[slot];
        }

        // Determines whether the specified shader program is in the shader.
        bool hasShader(const TkStringView &name, i32 permutationIndex = 0) const;
        GfxResType getType() const final override;

    protected:
        GfxShaderProgram *getShader(ShaderStage stage, const TkStringView &name, i32 permutationIndex) const;
        virtual GfxShaderProgram *createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream) = 0;
        static void readVertexLayout(MemoryIStream &stream, GfxVertexLayout *&inputLayout, GfxVertexLayout *&vertexLayout);

        void onReleaseData() override;

    protected:
        TkHashMap<u32, GfxShaderProgram *> m_shaders;
        GfxConstantBuffer *m_constantBuffers[GFX_MAX_CB_BINDED];

        GfxShader();
    };

}