--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- DirectXTex - DirectX texture processing library.
require("dxt", {
    source = "https://github.com/microsoft/DirectXTex.git",
    ref = "may2026",
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
        windows = {
            artifacts = {
                include_dirs = { "include" },
                lib_files = { "lib/DirectXTex.lib" }
            }
        }
    }
})
