--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- SPIRV-Cross - SPIR-V reflection and cross-compilation.
require("spirv", {
    source = "https://github.com/KhronosGroup/SPIRV-Cross.git",
    ref = "38681a30e09679191cc3957719eeee76024f6daf",
    build = "cmake",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    cmake = {
        generator = "Ninja",
        options = {
            BUILD_SHARED_LIBS = false,
            CMAKE_POLICY_DEFAULT_CMP0091 = "NEW",
            CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
            CMAKE_POSITION_INDEPENDENT_CODE = true,
            SPIRV_CROSS_CLI = false,
            SPIRV_CROSS_ENABLE_TESTS = false,
            SPIRV_CROSS_ENABLE_GLSL = true,
            SPIRV_CROSS_ENABLE_HLSL = true,
            SPIRV_CROSS_ENABLE_MSL = false,
            SPIRV_CROSS_ENABLE_CPP = false,
            SPIRV_CROSS_ENABLE_REFLECT = false,
            SPIRV_CROSS_ENABLE_C_API = true,
            SPIRV_CROSS_ENABLE_UTIL = false,
            SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS = false,
            SPIRV_CROSS_FORCE_PIC = true,
            SPIRV_CROSS_SKIP_INSTALL = false,
            SPIRV_CROSS_WERROR = false
        }
    },
    artifacts = {
        include_dirs = { "include" }
    },
    platforms = {
        windows = {
            artifacts = {
                include_dirs = { "include" },
                lib_files = {
                    debug = {
                        "lib/spirv-cross-cd.lib",
                        "lib/spirv-cross-cored.lib",
                        "lib/spirv-cross-glsld.lib",
                        "lib/spirv-cross-hlsld.lib"
                    },
                    release = {
                        "lib/spirv-cross-c.lib",
                        "lib/spirv-cross-core.lib",
                        "lib/spirv-cross-glsl.lib",
                        "lib/spirv-cross-hlsl.lib"
                    }
                }
            }
        },
        unix = {
            artifacts = {
                include_dirs = { "include" },
                lib_files = {
                    "lib/libspirv-cross-c.a",
                    "lib/libspirv-cross-core.a",
                    "lib/libspirv-cross-glsl.a",
                    "lib/libspirv-cross-hlsl.a"
                }
            }
        }
    }
})
