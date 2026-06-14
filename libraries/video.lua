--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

-- Native platform video system libraries.
when("windows", function()
    require_local(".", {
        name = "video",
        build = "prebuilt",
        export_defines = false,
        artifacts = {
            system_libs = {
                "mf",
                "mfcore",
                "mfplat",
                "mfplay",
                "mfreadwrite",
                "mfuuid"
            }
        }
    })
end)

when("macos", function()
    require_local(".", {
        name = "video",
        build = "prebuilt",
        export_defines = false,
        artifacts = {
            frameworks = {
                "Foundation",
                "AVFoundation",
                "CoreMedia",
                "CoreVideo",
                "CoreFoundation",
                "QuartzCore"
            }
        }
    })
end)

when("android", function()
    require_local(".", {
        name = "video",
        build = "prebuilt",
        export_defines = false,
        artifacts = {
            system_libs = {
                "android",
                "log",
                "mediandk"
            }
        }
    })
end)
