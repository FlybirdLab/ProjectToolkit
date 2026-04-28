/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "GfxConstantBuffer.hpp"
#include "GfxShader.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/io/MemoryStream.hpp"
#include "GfxDevice.hpp"
#include "GfxVertexLayout.hpp"
#include "toolkit/types/Span.hpp"

namespace toolkit
{

    static TK_FORCE_INLINE u32 hashPermutation(const TkStringView &name, i32 permutationIndex)
    {
        return HashFunc<TkStringView>::get(name) * 37 + permutationIndex;
    }

    void GfxShaderProgram::init(const GfxShaderProgramInitializer &initializer)
    {
        m_name = initializer.name;
        m_bindings = initializer.bindings;
        m_flags = initializer.flags;
#if TK_DEBUG
        m_owner = initializer.owner;
#endif
    }

    GfxShader::GfxShader()
        : GfxResource{}
    {
        TkMemory::memoryClear(m_constantBuffers, sizeof(m_constantBuffers));
    }

    bool GfxShader::create(MemoryIStream &stream)
    {
        releaseData();

        // Version
        i32 version;
        stream.readI32(&version);
        if (version != GPU_SHADER_CACHE_VERSION)
        {
            LogWarn("Unsupported shader version %i. The current version is %i.", version, GPU_SHADER_CACHE_VERSION);
            return false;
        }

        // Additional data start
        i32 additionalDataStart;
        stream.readI32(&additionalDataStart);

        // Shaders count
        i32 shadersCount;
        stream.readI32(&shadersCount);
        GfxShaderProgramInitializer initializer;
#if TK_DEBUG
        initializer.owner = this;
        const TkStringView name = getName();
#else
        const TkStringView name;
#endif
        const bool hasCompute = GfxDevice::sInstance->limits.hasCompute;
        for (i32 i = 0; i < shadersCount; i++)
        {
            const ShaderStage type = static_cast<ShaderStage>(stream.readByte());
            const i32 permutationsCount = stream.readByte();
            TK_ASSERT(TkMath::isInRange(permutationsCount, 1, SHADER_PERMUTATIONS_MAX_COUNT));

            // Load shader name
            stream.read(initializer.name, 11);
            TK_ASSERT(initializer.name.hasChars());

            // Load shader flags
            stream.readU32((u32 *)&initializer.flags);

            for (i32 permutationIndex = 0; permutationIndex < permutationsCount; permutationIndex++)
            {
                // Load bytecode
                u32 bytecodeSize;
                stream.readU32(&bytecodeSize);
                if (bytecodeSize > stream.length() - stream.pos())
                {
                    LogWarn("Invalid shader bytecode size.");
                    return false;
                }
                byte *bytecode = stream.move<byte>(bytecodeSize);

                // Read bindings
                stream.readBytes(&initializer.bindings, sizeof(ShaderBindings));

                // Create shader program
                if (type == ShaderStage::kCompute && !hasCompute)
                {
                    LogWarn("Failed to create %i Shader program '%s' (%s).", static_cast<i32>(type), TkString(initializer.name).getText(), name.getText());
                    continue;
                }
                GfxShaderProgram *shader = createGfxShaderProgram(type, initializer, TkSpan<byte>(bytecode, bytecodeSize), stream);
                if (shader == nullptr)
                {
#if !GFX_ALLOW_TESSELLATION_SHADERS
                    if (type == ShaderStage::kHull || type == ShaderStage::kDomain)
                        continue;
#endif
#if !GFX_ALLOW_GEOMETRY_SHADERS
                    if (type == ShaderStage::kGeometry)
                        continue;
#endif
                    LogError("Failed to create %i Shader program '%s' (%s).", static_cast<i32>(type), TkString(initializer.name).getText(), name.getText());
                    return true;
                }

                // Add to the collection
                const u32 hash = hashPermutation(shader->getName(), permutationIndex);
                TK_ASSERT(!m_shaders.containsKey(hash));
                m_shaders.add(hash, shader);
            }
        }

        // Constant Buffers
        const byte constantBuffersCount = stream.readByte();
        for (i32 i = 0; i < constantBuffersCount; i++)
        {
            // Load info
            const byte slotIndex = stream.readByte();
            if (slotIndex >= GFX_MAX_CB_BINDED)
            {
                LogWarn("Failed to create shader constant buffer.");
                return false;
            }
            u32 size;
            stream.readU32(&size);

            // Create CB
#if GFX_ENABLE_RESOURCE_NAMING
            TkString cbName = TkString::format(TEXT("%s.CB%i"), toString(), i);
#else
            TkString cbName;
#endif
            TK_ASSERT(m_constantBuffers[slotIndex] == nullptr);
            const auto cb = GfxDevice::sInstance->createConstantBuffer(size, cbName);
            if (cb == nullptr)
            {
                LogWarn("Failed to create shader constant buffer.");
                return false;
            }
            m_constantBuffers[slotIndex] = cb;
        }

        // Don't read additional data

        m_memoryUsage = 1;
        return true;
    }

    bool GfxShader::hasShader(const TkStringView &name, i32 permutationIndex) const
    {
        const u32 hash = hashPermutation(name, permutationIndex);
        return m_shaders.containsKey(hash);
    }

    GfxShaderProgram *GfxShader::getShader(ShaderStage stage, const TkStringView &name, i32 permutationIndex) const
    {
        GfxShaderProgram *shader = nullptr;
        const u32 hash = hashPermutation(name, permutationIndex);
        bool hasIt = m_shaders.containsKey(hash);
        if (hasIt) {
            shader = m_shaders[hash];
        }
#if !TK_DEBUG
        // Release build is more critical on that
        TK_ASSERT(shader != nullptr && shader->getStage() == stage);
#else
        if (shader == nullptr)
        {
            LogError("Missing %i shader '%s'[%i]. Object: %s.", static_cast<i32>(stage), TkString(name).getText(), permutationIndex, toString().getText());
        }
        else if (shader->getStage() != stage)
        {
            LogError("Invalid shader stage '%s'[%i]. Expected: %i. Actual: %i. Object: %s.", TkString(name).getText(), permutationIndex, static_cast<i32>(stage), static_cast<i32>(shader->getStage()), toString().getText());
        }
#endif
        return shader;
    }

    void GfxShader::readVertexLayout(MemoryIStream &stream, GfxVertexLayout *&inputLayout, GfxVertexLayout *&vertexLayout)
    {
        inputLayout = vertexLayout = nullptr;

        // Read input layout (based on shader reflection)
        GfxVertexLayout::Elements elements;
        i32 elementsCount;
        stream.readI32(&elementsCount);
        if (elementsCount < 0 || elementsCount > GFX_MAX_VS_ELEMENTS)
        {
            LogError("Incorrect input layout size.");
            return;
        }
        for (i32 i = 0; i < elementsCount; i++)
        {
            VertexElement element;
            stream.read(element);
            elements.add(element);
        }
        inputLayout = GfxVertexLayout::get(elements);

        // [Deprecated in v1.10]
        byte inputLayoutSize;
        stream.readByte(&inputLayoutSize);
        if (inputLayoutSize == 0)
            return;
        if (inputLayoutSize > GFX_MAX_VS_ELEMENTS)
        {
            LogError("Incorrect input layout size.");
            return;
        }
        elements.clear();
        for (i32 i = 0; i < inputLayoutSize; i++)
        {
            VertexElement element;
            stream.read(element);
            elements.add(element);
        }
        vertexLayout = GfxVertexLayout::get(elements);
    }

    GfxResType GfxShader::getType() const
    {
        return GfxResType::kShader;
    }

    void GfxShader::onReleaseData()
    {
        for (GfxConstantBuffer *&cb : m_constantBuffers)
        {
            if (cb)
            {
                GFX_DEL_RESOURCE(cb);
                cb = nullptr;
            }
        }
        m_memoryUsage = 0;
        m_shaders.clearDelete();
    }

}
