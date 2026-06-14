--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- mp4v2 - MP4/M4A container parser.
require("mp4v2", {
    source = "https://github.com/enzo1982/mp4v2.git",
    ref = "86b5aaf13d03291873608af559fe0399efe898f4",
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
            CMAKE_POLICY_VERSION_MINIMUM = "3.5"
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
                lib_files = { "mp4v2.lib" },
                system_libs = {
                    "mfplat.lib",
                    "mfreadwrite.lib",
                    "mfuuid.lib",
                    "ole32.lib"
                }
            }
        },
        unix = {
            artifacts = {
                mode = "copy",
                include_dirs = { "include" },
                lib_files = { "libmp4v2.a" }
            }
        }
    }
})
