--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

-- DataExtend - cultureinfo and concurrentqueue headers from Toolkit prebuilt data.
require("DataExtend", {
    source = "http://panel.flybugs.ink/toolkit_prebuilt.zip",
    type = "archive",
    build = "header_only",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    artifacts = {
        include_dirs = { "." }
    }
})
