/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/utils/PixelFormat.hpp"
#include <sdhc/common.hpp>

#include <toolkit/base/Math.hpp>

namespace toolkit
{
    struct ShaderPermutationEntry
    {
        TkString name;
        TkString value;
    };

    struct ShaderPermutation
    {
        // TODO: maybe we could use macro SHADER_PERMUTATIONS_MAX_PARAMS_COUNT and reduce amount of dynamic allocations for permutations
        TkArray<ShaderPermutationEntry> entries;
        TkArray<char> debugData;
    };

    // Shader function metadata
    class ShaderFunctionMeta
    {
    public:
        // Virtual destructor
        virtual ~ShaderFunctionMeta() = default;

    public:
        // Function name
        TkString name;
        // Function flags.
        GfxShaderFlags flags;
        // The minimum graphics platform feature level to support this shader.
        GfxFeatureLevel minFeatureLevel;
        // All possible values for the permutations and it's values to generate different permutation of this function
        TkArray<ShaderPermutation> permutations;

    public:
        // Check if definition name has been added to the given permutation
        bool hasDefinition(i32 permutationIndex, const TkString &definename) const
        {
            TK_ASSERT(TkMath::isInRange(permutationIndex, 0, permutations.count() - 1));

            auto &permutation = permutations[permutationIndex];
            for (i32 i = 0; i < permutation.entries.count(); i++)
            {
                if (permutation.entries[i].name == definename)
                    return true;
            }

            return false;
        }

        // Check if definition name has been added to the permutations collection
        bool hasDefinition(const TkString &definename) const
        {
            for (i32 permutationIndex = 0; permutationIndex < permutations.count(); permutationIndex++)
            {
                if (hasDefinition(permutationIndex, definename))
                    return true;
            }

            return false;
        }

        // Get all macros for given shader permutation
        void getDefinitionsForPermutation(i32 permutationIndex, TkArray<ShaderMacro> &macros) const
        {
            TK_ASSERT(TkMath::isInRange(permutationIndex, 0, permutations.count() - 1));

            auto &permutation = permutations[permutationIndex];
            for (i32 i = 0; i < permutation.entries.count(); i++)
            {
                auto &e = permutation.entries[i];
                macros.add({e.name.get(),
                            e.value.get()});
            }
        }

    public:
        // Get shader function meta stage type.
        virtual ShaderStage getStage() const = 0;
    };

    // Vertex shader function met
    class VertexShaderMeta : public ShaderFunctionMeta
    {
    public:
        // Input element type
        enum class InputType : byte
        {
            kInvalid = 0,
            kPOSITION = 1,
            kCOLOR = 2,
            kTEXCOORD = 3,
            kNORMAL = 4,
            kTANGENT = 5,
            kBITANGENT = 6,
            kATTRIBUTE = 7,
            kBLENDINDICES = 8,
            kBLENDWEIGHTS = 9,
            kBLENDWEIGHT = kBLENDWEIGHTS,
        };

        // Input element
        struct InputElement
        {
            // Semantic type
            InputType type;
            // Semantic index
            byte index;
            // Element data format
            PixelFormat format;
            // An integer value that identifies the input-assembler
            byte inputSlot;
            // Optional. Offset (in bytes) between each element. Use INPUT_LAYOUT_ELEMENT_ALIGN for convenience to define the current element directly after the previous one, including any packing if necessary
            u32 alignedByteOffset;
            // Identifies the input data class for a single input slot. INPUT_LAYOUT_ELEMENT_PER_VERTEX_DATA or INPUT_LAYOUT_ELEMENT_PER_INSTANCE_DATA
            byte inputSlotClass;
            // The number of instances to draw using the same per-instance data before advancing in the buffer by one element. This value must be 0 for an element that contains per-vertex data
            u32 instanceDataStepRate;
            // The visible flag expression. Allows to show/hide element from the input layout based on input macros (also from permutation macros). use empty value to skip this feature.
            TkString visibleFlag;
        };

    public:
        // Input layout description
        TkArray<InputElement> inputLayout;

    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kVertex;
        }
    };

    // Hull (or tessellation control) shader function met
    class HullShaderMeta : public ShaderFunctionMeta
    {
    public:
        // The input control points count (valid range: 1-32).
        i32 controlPointsCount;

    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kHull;
        }
    };

    // Domain (or tessellation evaluation) shader function met
    class DomainShaderMeta : public ShaderFunctionMeta
    {
    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kDomain;
        }
    };

    // Geometry shader function met
    class GeometryShaderMeta : public ShaderFunctionMeta
    {
    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kGeometry;
        }
    };

    // Pixel shader function met
    class PixelShaderMeta : public ShaderFunctionMeta
    {
    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kPixel;
        }
    };

    // Compute shader function met
    class ComputeShaderMeta : public ShaderFunctionMeta
    {
    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kCompute;
        }
    };

    // Constant buffer met
    struct ConstantBufferMeta
    {
        // Slot index
        byte slot;

        // Buffer name
        TkString name;
    };

    // Shader source metadat
    class ShaderMeta
    {
    public:
        // Vertex Shaders
        TkArray<VertexShaderMeta> vs;
        // Hull Shaders
        TkArray<HullShaderMeta> hs;
        // Domain Shaders
        TkArray<DomainShaderMeta> ds;
        // Geometry Shaders
        TkArray<GeometryShaderMeta> gs;
        // Pixel Shaders
        TkArray<PixelShaderMeta> ps;
        // Compute Shaders
        TkArray<ComputeShaderMeta> cs;
        // Constant Buffers
        TkArray<ConstantBufferMeta> cb;

    public:
        // Get amount of shaders attached (not counting permutations).
        i32 getShadersCount() const
        {
            return vs.count() + hs.count() + ds.count() + gs.count() + ps.count() + cs.count();
        }

        // Get all shader functions (all types).
        void getShaders(TkArray<const ShaderFunctionMeta *> &functions) const
        {
#define PEEK_SHADERS(collection)                 \
    for (i32 i = 0; i < collection.count(); i++) \
        functions.add(dynamic_cast<const ShaderFunctionMeta *>(&(collection[i])));
            PEEK_SHADERS(vs);
            PEEK_SHADERS(hs);
            PEEK_SHADERS(ds);
            PEEK_SHADERS(gs);
            PEEK_SHADERS(ps);
            PEEK_SHADERS(cs);
#undef PEEK_SHADERS
        }
    };

}