--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

require("enet", {
    source = "https://github.com/lsalzman/enet.git",
    ref = "v1.3.18",
    build = "cmake",
    export_defines = false,
    cmake = {
        generator = "Ninja",
        options = {
            BUILD_SHARED_LIBS = false,
            -- Force static runtime library (MultiThreadedDebug for debug, MultiThreaded for release)
            CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
            CMAKE_POSITION_INDEPENDENT_CODE = true
        }
    },
    artifacts = {
        include_dirs = { "include" }
    },
    platforms = {
        windows = {
            artifacts = {
                lib_files = {
                    debug = { "lib/static/enet.lib" },
                    release = { "lib/static/enet.lib" }
                },
                system_libs = { "winmm" }
            }
        },
        unix = {
            artifacts = {
                lib_files = { "lib/static/libenet.a" }
            }
        }
    }
})