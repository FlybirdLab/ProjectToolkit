--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

-- stb - single-file public domain libraries
require("stb", {
    source = "https://github.com/nothings/stb.git",
    ref = "master",
    build = "header_only",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    artifacts = {
        include_dirs = { "." }
    }
})
