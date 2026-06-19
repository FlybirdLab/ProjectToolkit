--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

-- ToolkitProject Dependency Configuration
package("ToolkitProject")
version("1.0.0")
default_config("debug")
default_platform("host")
default_arch("x64")

-- Networking
include("libraries/enet.lua")
-- Audio
include("libraries/openal.lua")
include("libraries/minimp3.lua")
include("libraries/mp4v2.lua")
include("libraries/ogg.lua")
include("libraries/vorbis.lua")
include("libraries/coreaudio.lua")
-- Logging
include("libraries/spdlog.lua")
include("libraries/tracy.lua")
-- Data Formats
include("libraries/rapidjson.lua")
include("libraries/data_extend.lua")
include("libraries/stb.lua")
include("libraries/rpmalloc.lua")
include("libraries/xxhash.lua")
include("libraries/directx.lua")
include("libraries/dxc.lua")
include("libraries/dxt.lua")
include("libraries/vulkan.lua")
include("libraries/vma.lua")
include("libraries/spirv_headers.lua")
include("libraries/spirv_tools.lua")
include("libraries/spirv.lua")
include("libraries/volk.lua")
include("libraries/glad.lua")
include("libraries/glslang.lua")
include("libraries/astc.lua")
-- Video
include("libraries/video.lua")
-- Testing
include("libraries/gtest.lua")
