--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

-- DirectXShaderCompiler - DXC HLSL compiler libraries.
require("dxc", {
    source = "https://github.com/microsoft/DirectXShaderCompiler.git",
    ref = "fe2615732",
    build = "cmake",
    linkage = "shared",
    runtime = "static",
    export_defines = false,
    cmake = {
        -- generator = "Ninja",
        install = false,
        build_targets = {
            "dxcompiler",
            "dxildll",
            "dxc"
        },
        options = {
            CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
            CMAKE_POSITION_INDEPENDENT_CODE = true,
            BUILD_SHARED_LIBS = false,
            LLVM_TARGETS_TO_BUILD = "None",
            LLVM_BUILD_TESTS = false,
            LLVM_INCLUDE_TESTS = false,
            LLVM_INCLUDE_EXAMPLES = false,
            LLVM_INCLUDE_DOCS = false,
            LLVM_BUILD_DOCS = false,
            LLVM_INCLUDE_TOOLS = true,
            LLVM_ENABLE_EH = true,
            LLVM_ENABLE_RTTI = true,
            CLANG_INCLUDE_TESTS = false,
            CLANG_ENABLE_ARCMT = false,
            CLANG_ENABLE_STATIC_ANALYZER = false,
            CLANG_ENABLE_OBJC_REWRITER = false,
            HLSL_INCLUDE_TESTS = false,
            HLSL_BUILD_DXILCONV = false,
            SPIRV_BUILD_TESTS = false
        }
    },
    artifacts = {
        mode = "copy",
        include_dirs = { "include" }
    },
    platforms = {
        windows = {
            artifacts = {
                mode = "copy",
                include_dirs = { "include" },
                bin_dirs = { "bin" },
                lib_files = {
                    "lib/dxcompiler.lib",
                    "lib/dxil.lib"
                },
                bin_files = {
                    "bin/dxcompiler.dll",
                    "bin/dxil.dll",
                    "bin/dxc.exe"
                }
            }
        }
    }
})
