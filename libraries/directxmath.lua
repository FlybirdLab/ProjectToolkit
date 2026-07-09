--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- DirectXMath - SIMD C++ math library.
-- On Windows it's part of the SDK; on Unix it must be built from source.
when("unix", function()
    require("directxmath", {
        source = "https://github.com/microsoft/DirectXMath.git",
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
                BUILD_TESTING = false
            }
        },
        artifacts = {
            include_dirs = { "include" }
        }
    })
end)

when("windows", function()
    require_local(".", {
        name = "directxmath",
        build = "prebuilt",
        export_defines = false,
        artifacts = {}
    })
end)
