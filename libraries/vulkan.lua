--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- Vulkan SDK - local system SDK headers and loader import library.
when("windows", function()
    require_local(".", {
        name = "vulkan",
        build = "prebuilt",
        export_defines = false,
        artifacts = {
            include_dirs = { "D:/Sdks/vulkan/include" },
            lib_files = { "D:/Sdks/vulkan/Lib/vulkan-1.lib" }
        }
    })
end)
