/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * https://gitlab.freedesktop.org/apinheiro/mesa/-/issues/9
 */

#pragma once

#include <toolkit/platforms/vulkan/VkPlatform.hpp>

#if GFX_API_VULKAN && TK_LINUX

namespace toolkit
{

// Support more backbuffers in case driver decides to use more
#define VK_BACK_BUFFERS_COUNT_MAX 8

// Prevent wierd error 'Invalid VkValidationCacheEXT Object'
#define VK_USE_VALIDATION_CACHE 0

	class LinuxVulkanPlatform : public VulkanPlatformBase
	{
	public:
		static void getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers);
		static void createSurface(void *windowHandle, VkInstance instance, VkSurfaceKHR *outSurface);
	};

	typedef LinuxVulkanPlatform VulkanPlatform;

}

#endif
