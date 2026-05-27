/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/GfxVertexLayout.hpp"
#include <sdhc/ShaderMeta.hpp>
#include <sdhc/common.hpp>

namespace toolkit
{

    // Base class for the objects that can compile shaders source code.
    class ShaderCompiler
    {
    public:
        struct ShaderResourceBuffer
        {
            byte slot;
            bool isUsed;
            u32 size;
        };

    private:
        TkArray<char> m_funcNameDefineBuffer;

    protected:
        GfxShaderProfile m_profile;
        class ShaderCompilationContext *m_context = nullptr;
        TkArray<ShaderMacro> m_globalMacros;
        TkArray<ShaderMacro> m_macros;
        TkArray<ShaderResourceBuffer> m_constantBuffers;

    public:
        ShaderCompiler(GfxShaderProfile profile)
            : m_profile(profile)
        {
        }
        virtual ~ShaderCompiler() = default;

    public:
        // Gets shader profile supported by this compiler.
        TK_FORCE_INLINE GfxShaderProfile getProfile() const
        {
            return m_profile;
        }

        // Performs the shader compilation.
        bool compile(ShaderCompilationContext *context);

        // Gets the included file source code. Handles system includes and absolute includes. Method is thread-safe.
        static bool getIncludedFileSource(ShaderCompilationContext *context, const char *sourceFile, const char *includedFile, const char *&source, i32 &sourceLength);
        // Clears the cache used by the shader includes.
        static void disposeIncludedFilesCache();

    protected:
        // Input elements read from reflection after shader compilation. Rough approx or attributes without exact format nor bind slot (only semantics and value dimensions match).
        struct AdditionalDataVS
        {
            GfxVertexLayout::Elements inputs;
        };

        typedef bool (*WritePermutationData)(ShaderCompilationContext *, ShaderFunctionMeta &, i32, const TkArray<ShaderMacro> &, void *);

        virtual bool compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite = nullptr) = 0;
        bool compileShaders();

        virtual bool onCompileBegin();
        virtual bool onCompileEnd();

        static bool writeShaderFunctionBegin(ShaderCompilationContext *context, ShaderFunctionMeta &meta);
        static bool writeShaderFunctionPermutation(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, const class ShaderBindings &bindings, const void *header, i32 headerSize, const void *cache, i32 cacheSize);
        static bool writeShaderFunctionPermutation(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, const ShaderBindings &bindings, const void *cache, i32 cacheSize);
        static bool writeShaderFunctionEnd(ShaderCompilationContext *context, ShaderFunctionMeta &meta);
        static bool writeCustomDataVS(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, const TkArray<ShaderMacro> &macros, void *additionalData);
        static bool writeCustomDataHS(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, const TkArray<ShaderMacro> &macros, void *additionalData);
        static const char *getShaderMetaDefinesPreamble();
        void getDefineForFunction(ShaderFunctionMeta &meta, TkArray<ShaderMacro> &macros);

        static VertexElementTypes parseVertexElementType(TkStringView semantic, u32 index = 0);
    };

}
