/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/vulkan/linux/LinuxPlatform.hpp>

#if GFX_API_VULKAN && TK_LINUX

#include "../VkUtils.hpp"

// Contents of vulkan\vulkan_xlib.h inlined here to prevent typename collisions with engine types due to X11 types
#include "toolkit/platforms/linux/IncludeX11.hpp"
#ifdef __cplusplus
extern "C"
{
#endif
#define VK_KHR_xlib_surface 1
#define VK_KHR_XLIB_SURFACE_SPEC_VERSION 6
#define VK_KHR_XLIB_SURFACE_EXTENSION_NAME "VK_KHR_xlib_surface"
	typedef VkFlags VkXlibSurfaceCreateFlagsKHR;

	typedef struct VkXlibSurfaceCreateInfoKHR
	{
		VkStructureType sType;
		const void *pNext;
		VkXlibSurfaceCreateFlagsKHR flags;
		X11::Display *dpy;
		X11::Window window;
	} VkXlibSurfaceCreateInfoKHR;

	typedef VkResult(VKAPI_PTR *PFN_vkCreateXlibSurfaceKHR)(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
	typedef VkBool32(VKAPI_PTR *PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, X11::Display *dpy, X11::VisualID visualID);
#ifndef VK_NO_PROTOTYPES
	VKAPI_ATTR VkResult VKAPI_CALL vkCreateXlibSurfaceKHR(
		VkInstance instance,
		const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkSurfaceKHR *pSurface);
	VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceXlibPresentationSupportKHR(
		VkPhysicalDevice physicalDevice,
		uint32_t queueFamilyIndex,
		Display *dpy,
		VisualID visualID);
#endif
#ifdef __cplusplus
}
#endif
//

// Export X11 surface extension from volk
extern PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
extern PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR;

namespace toolkit
{

	void LinuxVulkanPlatform::getInstanceExtensions(TkArray<const char *> &extensions, TkArray<const char *> &layers)
	{
		extensions.add(VK_KHR_SURFACE_EXTENSION_NAME);
		extensions.add(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
	}

	void LinuxVulkanPlatform::createSurface(void *windowHandle, VkInstance instance, VkSurfaceKHR *surface)
	{
		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
		VkUtils::zeroStruct(surfaceCreateInfo, VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.dpy = (X11::Display *)LinuxPlatform::getXDisplay();
		surfaceCreateInfo.window = (X11::Window)windowHandle;
		VALIDATE_VK_RESULT(vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, surface));
	}

}

#endif
