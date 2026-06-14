--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- CoreAudio system frameworks from the Apple SDK.
when("macos", function()
    require_local(".", {
        name = "coreaudio",
        build = "prebuilt",
        export_defines = false,
        artifacts = {
            frameworks = {
                "CoreAudio",
                "AudioToolbox",
                "AVFoundation",
                "Foundation"
            }
        }
    })
end)
