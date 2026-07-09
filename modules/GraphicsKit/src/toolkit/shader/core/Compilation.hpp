/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/core/common.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/platforms/FileSystem.hpp>

namespace toolkit
{
    // Shaders compilation service allows to compile shader source code for a desire platform. Supports multi-threading.
    class ShadersCompilation
    {
    public:
        // Fired when a registered shader include file changes. Engine/editor layers can bind here and decide what assets to reload.
        static TkAction<const TkString &, FileAction> onIncludeChanged;

    public:
        static void init();
        static void dispose();
        
    public:
        // Compile the shader.
        static bool compile(ShaderCompilationOptions &options);

        //DELETE // Registers shader asset for the automated reloads on source includes changes.
        //DELETE static void RegisterForShaderReloads(Asset *asset, const String &includedPath);
        //DELETE // Unregisters shader asset from the automated reloads on source includes changes.
        //DELETE static void UnregisterForShaderReloads(Asset *asset);

        // Registers an include file for decoupled change notifications.
        static void registerIncludeForReloads(TkStringView includedPath);
        // Unregisters an include file from decoupled change notifications.
        static void unregisterIncludeForReloads(TkStringView includedPath);

        // Reads the included shader files stored in the shader cache data.
        static void extractShaderIncludes(byte *shaderCache, i32 shaderCacheLength, TkArray<TkString> &includes);
        // Resolves shader path name into absolute file path. Resolves './<ProjectName>/ShaderFile.hlsl' cases into a full path.
        static TkString resolveShaderPath(TkStringView path);
        // Compacts the full shader file path into portable format with project name prefix such as './<ProjectName>/ShaderFile.hlsl'.
        static TkString compactShaderPath(TkStringView path);

    private:
        static class ShaderCompiler *createCompiler(GfxShaderProfile profile);
        static ShaderCompiler *requestCompiler(GfxShaderProfile profile);
        static void freeCompiler(ShaderCompiler *compiler);
    };

}
