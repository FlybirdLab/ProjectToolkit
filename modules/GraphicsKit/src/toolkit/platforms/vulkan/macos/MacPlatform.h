/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/VkPlatform.hpp>

#if GFX_API_VULKAN && TK_MACOS

namespace toolkit
{

#define VK_BACK_BUFFERS_COUNT 3

// General/Validation Error:0 VK_ERROR_INITIALIZATION_FAILED: Could not create MTLCounterSampleBuffer for query pool of type VK_QUERY_TYPE_TIMESTAMP. Reverting to emulated behavior. (Error code 0): Cannot allocate sample buffer
#define VK_USE_QUERIES 0

	class MacVulkanPlatform : public VulkanPlatformBase
	{
	public:
		static void getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers);
	};

	typedef MacVulkanPlatform VulkanPlatform;

}

#endif
