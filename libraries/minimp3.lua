--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- minimp3 - header-only MP3 decoder.
require("minimp3", {
    source = "https://github.com/lieff/minimp3.git",
    ref = "7b590fdcfa5a79c033e76eacc05d0c3e4c79f536",
    build = "header_only",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    artifacts = {
        include_dirs = { "." }
    }
})
