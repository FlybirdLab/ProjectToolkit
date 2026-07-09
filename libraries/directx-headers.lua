--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- DirectX-Headers - Direct3D / DXCore headers and GUID library.
-- On Windows it's part of the SDK; on Unix it must be built from source.
when("unix", function()
    require("directx-headers", {
        source = "https://github.com/microsoft/DirectX-Headers.git",
        ref = "v1.721.1-preview",
        build = "cmake",
        linkage = "static",
        runtime = "static",
        export_defines = false,
        cmake = {
            generator = "Ninja",
            build_type = "Release",
            install = true,
            options = {
                CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
                CMAKE_POSITION_INDEPENDENT_CODE = true,
                BUILD_TESTING = false,
                DXHEADERS_BUILD_TEST = false,
                DXHEADERS_BUILD_GOOGLE_TEST = false
            }
        },
        artifacts = {
            include_dirs = { "include" }
        }
    })
end)

when("windows", function()
    require_local(".", {
        name = "directx-headers",
        build = "prebuilt",
        export_defines = false,
        artifacts = {}
    })
end)
