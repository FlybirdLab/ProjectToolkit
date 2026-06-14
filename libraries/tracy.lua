--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- Tracy profiler.
require("tracy", {
    source = "https://github.com/wolfpld/tracy.git",
    ref = "07147111b26ddaf43fb46fabbab42de4451fa567",
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
            TRACY_STATIC = true,
            TRACY_ENABLE = true,
            TRACY_ON_DEMAND = false,
            TRACY_CALLSTACK = false,
            TRACY_CLIENT_PYTHON = false,
            TRACY_Fortran = false,
            TRACY_LTO = false
        }
    },
    artifacts = {
        include_dirs = { "include/tracy" }
    },
    platforms = {
        windows = {
            artifacts = {
                include_dirs = { "include/tracy" },
                lib_files = {
                    debug = { "lib/Debug/TracyClient.lib" },
                    release = { "lib/TracyClient.lib" }
                }
            }
        },
        unix = {
            artifacts = {
                include_dirs = { "include/tracy" },
                lib_files = { "lib/libTracyClient.a" },
                system_libs = { "pthread", "dl" }
            }
        }
    }
})
