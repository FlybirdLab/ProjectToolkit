--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- DirectX system libraries from the Windows SDK.
when("windows", function()
    require_local(".", {
        name = "directx11",
        build = "prebuilt",
        export_defines = false,
        artifacts = {
            system_libs = {
                "d3d11.lib",
                "dxgi.lib",
                "dxguid.lib"
            }
        }
    })

    require_local(".", {
        name = "directx12",
        build = "prebuilt",
        export_defines = false,
        artifacts = {
            system_libs = {
                "d3d12.lib",
                "dxgi.lib",
                "dxguid.lib"
            }
        }
    })
end)
