--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]
-- spdlog - Fast C++ logging library
require("spdlog", {
    source = "https://github.com/gabime/spdlog.git",
    mirrors = {
        "https://gitee.com/mirrors/spdlog.git"
    },
    ref = "v1.15.3",
    build = "cmake",
    export_defines = false,
    cmake = {
        generator = "Ninja",
        options = {
            SPDLOG_BUILD_EXAMPLE = false,
            SPDLOG_BUILD_TESTS = false,
            SPDLOG_BUILD_BENCH = false,
            SPDLOG_FMT_EXTERNAL = false,  -- Use bundled fmt
            BUILD_SHARED_LIBS = false,
            -- Force static runtime library
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
                    debug = { "lib/spdlogd.lib" },
                    release = { "lib/spdlog.lib" }
                }
            }
        },
        unix = {
            artifacts = {
                lib_files = {
                    debug = { "lib/libspdlogd.a" },
                    release = { "lib/libspdlog.a" }
                }
            }
        }
    }
})
