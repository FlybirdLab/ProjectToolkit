--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

require("openal", {
    source = "https://github.com/kcat/openal-soft.git",
    ref = "master",  -- Latest master branch as detected
    build = "cmake",
    export_defines = false,
    cmake = {
        generator = "Ninja",
        options = {
            ALSOFT_UTILS = false,
            ALSOFT_EXAMPLES = false,
            ALSOFT_TESTS = false,
            LIBTYPE = "STATIC",
            -- Force static runtime library
            CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
            CMAKE_POSITION_INDEPENDENT_CODE = true
        }
    },
    artifacts = {
        include_dirs = { "include", "include/AL" }
    },
    platforms = {
        windows = {
            artifacts = {
                lib_files = {
                    debug = { "lib/OpenAL32.lib" },
                    release = { "lib/OpenAL32.lib" }
                }
            }
        },
        unix = {
            artifacts = {
                lib_files = { "lib/libopenal.a" }
            }
        }
    }
})
