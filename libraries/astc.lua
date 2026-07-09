--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- astc-encoder - ASTC texture compression library
require("astc", {
    source = "https://github.com/ARM-software/astc-encoder.git",
    ref = "2b86c032a9a5e23e58eb9e32ac484556c448d0b1",
    build = "cmake",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    cmake = {
        generator = "Ninja",
        options = {
            BUILD_SHARED_LIBS = false,
            CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
            CMAKE_POSITION_INDEPENDENT_CODE = true,
            ASTCENC_ASAN = false,
            ASTCENC_BIG_ENDIAN = false,
            ASTCENC_CLI = false,
            ASTCENC_DECOMPRESSOR = false,
            ASTCENC_DIAGNOSTICS = false,
            ASTCENC_INVARIANCE = true,
            ASTCENC_ISA_AVX2 = false,
            ASTCENC_ISA_NATIVE = false,
            ASTCENC_ISA_NEON = false,
            ASTCENC_ISA_NONE = false,
            ASTCENC_ISA_SSE2 = false,
            ASTCENC_ISA_SSE41 = false,
            ASTCENC_ISA_SVE_128 = false,
            ASTCENC_ISA_SVE_256 = false,
            ASTCENC_SHAREDLIB = false,
            ASTCENC_UBSAN = false,
            ASTCENC_UNITTEST = false,
            ASTCENC_WERROR = true,
            ASTCENC_X86_GATHERS = true
        }
    },
    artifacts = {
        mode = "copy",
        include_dirs = { "Source" }
    },
    platforms = {
        windows = {
            artifacts = {
                mode = "copy",
                include_dirs = { "Source" },
                lib_files = { "Source/astcenc-native-static.lib" }
            }
        },
        unix = {
            artifacts = {
                mode = "copy",
                include_dirs = { "Source" },
                lib_files = { "Source/libastcenc-native-static.a" }
            }
        }
    }
})
