--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- SPIRV-Headers required by SPIRV-Tools.
require("spirv-headers", {
    source = "https://github.com/KhronosGroup/SPIRV-Headers.git",
    ref = "1a22b167081842915a1c78a0b5b5a353a23284aa",
    build = "header_only",
    export_defines = false,
    artifacts = {
        include_dirs = { "include" }
    }
})
