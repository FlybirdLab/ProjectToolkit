/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/VkPlatform.hpp>

#if GFX_API_VULKAN && TK_ANDROID

namespace toolkit
{

// Support more backbuffers in case driver decides to use more
#define VK_BACK_BUFFERS_COUNT_MAX 8

	class AndroidVulkanPlatform : public VulkanPlatformBase
	{
	public:
		static void getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers);
		static void getDeviceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers);
		static void createSurface(void *windowHandle, VkInstance instance, VkSurfaceKHR *surface);
	};

	typedef AndroidVulkanPlatform VulkanPlatform;

}

#endif
