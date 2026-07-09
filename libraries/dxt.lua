--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- DirectXTex - DirectX texture processing library.
require("dxt", {
    source = "https://github.com/microsoft/DirectXTex.git",
    ref = "may2026",
    dependencies = { "directxmath", "directx-headers" },
    build = "cmake",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    cmake = {
        generator = "Ninja",
        options = {
            CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
            CMAKE_POSITION_INDEPENDENT_CODE = true,
            BUILD_SHARED_LIBS = false,
            BUILD_TOOLS = false,
            BUILD_SAMPLE = false,
            BUILD_TESTING = false
        }
    },
    artifacts = {
        include_dirs = { "include" }
    },
    platforms = {
        unix = {
            cmake = {
                configure_args = {
                    "-DCMAKE_CXX_FLAGS=-DE_ILLEGAL_STATE_CHANGE=0x8000000A"
                }
            }
        },
        windows = {
            artifacts = {
                include_dirs = { "include" },
                lib_files = { "lib/DirectXTex.lib" }
            }
        }
    }
})
