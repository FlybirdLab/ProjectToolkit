/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "GfxBuffer.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    enum class VertexElementTypes : byte
    {
        // Undefined.
        kUnknown = 0,
        // Vertex position. Maps to 'POSITION' semantic in the shader.
        kPosition = 1,
        // Vertex color. Maps to 'COLOR' semantic in the shader.
        kColor = 2,
        // Vertex normal vector. Maps to 'NORMAL' semantic in the shader.
        kNormal = 3,
        // Vertex tangent vector. Maps to 'TANGENT' semantic in the shader.
        kTangent = 4,
        // Skinned bone blend indices. Maps to 'BLENDINDICES' semantic in the shader.
        kBlendIndices = 5,
        // Skinned bone blend weights. Maps to 'BLENDWEIGHTS' semantic in the shader.
        kBlendWeights = 6,
        // Primary texture coordinate (UV). Maps to 'TEXCOORD0' semantic in the shader.
        kTexCoord0 = 7,
        // Additional texture coordinate (UV1). Maps to 'TEXCOORD1' semantic in the shader.
        kTexCoord1 = 8,
        // Additional texture coordinate (UV2). Maps to 'TEXCOORD2' semantic in the shader.
        kTexCoord2 = 9,
        // Additional texture coordinate (UV3). Maps to 'TEXCOORD3' semantic in the shader.
        kTexCoord3 = 10,
        // Additional texture coordinate (UV4). Maps to 'TEXCOORD4' semantic in the shader.
        kTexCoord4 = 11,
        // Additional texture coordinate (UV5). Maps to 'TEXCOORD5' semantic in the shader.
        kTexCoord5 = 12,
        // Additional texture coordinate (UV6). Maps to 'TEXCOORD6' semantic in the shader.
        kTexCoord6 = 13,
        // Additional texture coordinate (UV7). Maps to 'TEXCOORD7' semantic in the shader.
        kTexCoord7 = 14,
        // General purpose attribute (at index 0). Maps to 'ATTRIBUTE0' semantic in the shader.
        kAttribute0 = 15,
        // General purpose attribute (at index 1). Maps to 'ATTRIBUTE1' semantic in the shader.
        kAttribute1 = 16,
        // General purpose attribute (at index 2). Maps to 'ATTRIBUTE2' semantic in the shader.
        kAttribute2 = 17,
        // General purpose attribute (at index 3). Maps to 'ATTRIBUTE3' semantic in the shader.
        kAttribute3 = 18,
        // Lightmap UVs that usually map one of the texture coordinate channels. Maps to 'LIGHTMAP' semantic in the shader.
        kLightmap = 30,
        // Texture coordinate. Maps to 'TEXCOORD' semantic in the shader.
        kTexCoord = kTexCoord0,
        // General purpose attribute. Maps to 'ATTRIBUTE0' semantic in the shader.
        kAttribute = kAttribute0,
        MAX
    };

    // Vertex buffer data element. Defines access to data passed to Vertex Shader.
    TK_PACK_BEGIN()
    struct TK_API VertexElement
    {
    public:
        // Type of the vertex element data.
        VertexElementTypes type;
        // Index of the input vertex buffer slot (as provided in GPUContext::BindVB).
        byte slot;
        // Byte offset of this element relative to the start of a vertex buffer. Use value 0 to use auto-calculated offset based on previous elements in the layout (except when explicitOffsets is false).
        byte offset;
        // Flag used to mark data using hardware-instancing (element will be repeated for every instance). Empty to step data per-vertex when reading input buffer stream (rather than per-instance step).
        byte perInstance;
        // Format of the vertex element data.
        PixelFormat format;

    public:
        TkString toString() const;

        bool operator==(const VertexElement &other) const;

        TK_FORCE_INLINE bool operator!=(const VertexElement &other) const
        {
            return !operator==(other);
        }
    } TK_PACK_END();

    u32 TK_API getHash(const VertexElement &key);
    template <>
    struct TK_API HashFunc<VertexElement>
    {
        TK_FORCE_INLINE static u32 get(const VertexElement &k)
        {
            return getHash(k);
        }
    };

    // Defines input layout of vertex buffer data passed to the Vertex Shader.
    class TK_API GfxVertexLayout : public GfxResource
    {
    public:
        typedef TkStaticArray<VertexElement, GFX_MAX_VS_ELEMENTS> Elements;

    protected:
        GfxVertexLayout();

        void setElements(const Elements &elements, bool explicitOffsets);

    public:
        GfxResType getType() const override
        {
            return GfxResType::kDescriptor;
        }

    public:
        // Get the list of elements used by this layout.
        TK_FORCE_INLINE const Elements &getElements() const
        {
            return m_elements;
        }

        // Get the list of elements used by this layout as a text (each element in a new line).
        TkString getElementsString() const;

        // Get the size in bytes of all elements in the layout structure (including their offsets).
        TK_FORCE_INLINE u32 getStride() const
        {
            return m_stride;
        }

        // Searches for a given element type in a layout.
        VertexElement findElement(VertexElementTypes type) const;

        // Get the vertex layout for a given list of elements. Uses internal cache to skip creating layout if it's already exists for a given list.
        static GfxVertexLayout *get(const Elements &elements, bool explicitOffsets = false);
        // Get the vertex layout for a given list of vertex buffers (sequence of binding slots based on layouts set on those buffers). Uses internal cache to skip creating layout if it's already exists for a given list.
        static GfxVertexLayout *get(const TkSpan<GfxBuffer *> &vertexBuffers);
        // Merges list of layouts in a single one. Uses internal cache to skip creating layout if it's already exists for a given list.
        static GfxVertexLayout *get(const TkSpan<GfxVertexLayout *> &layouts);

        // Merges reference vertex elements into the given set of elements to ensure the reference list is satisfied (vertex shader input requirement). Returns base layout if it's valid.
        static GfxVertexLayout *merge(GfxVertexLayout *base, GfxVertexLayout *reference, bool removeUnused = false, bool addMissing = true, i32 missingSlotOverride = -1);

    private:
        Elements m_elements;
        u32 m_stride;
    };

}