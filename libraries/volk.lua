--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- volk - Vulkan meta-loader.
require("volk", {
    source = "https://github.com/zeux/volk.git",
    ref = "3ca312a4f38baa63d8006b6905abbeeb89c8087d",
    build = "header_only",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    artifacts = {
        include_dirs = { "." }
    }
})
