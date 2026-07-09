--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- Vulkan Memory Allocator - stb-style single-header Vulkan allocator.
require("vma", {
    source = "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git",
    ref = "master",
    build = "header_only",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    artifacts = {
        include_dirs = { "include" }
    }
})
