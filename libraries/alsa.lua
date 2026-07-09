--[[
Author: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

-- ALSA (Advanced Linux Sound Architecture) system library.
-- Official repository: https://github.com/alsa-project/alsa-lib
when("linux", function()
    require_local(".", {
        name = "alsa",
        build = "prebuilt",
        export_defines = false,
        artifacts = {
            system_libs = { "asound" }
        }
    })
end)
