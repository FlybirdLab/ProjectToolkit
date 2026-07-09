/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkUtils.hpp"

#if GFX_API_VULKAN

#include "toolkit/types/String.hpp"
#include "toolkit/types/StringBuilder.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{
    VkFormat VkUtils::pixelFormatToVkFormat[110] =
        {
            VK_FORMAT_UNDEFINED,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_FORMAT_R32G32B32A32_UINT,
            VK_FORMAT_R32G32B32A32_SINT,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32_UINT,
            VK_FORMAT_R32G32B32_SINT,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_FORMAT_R16G16B16A16_UNORM,
            VK_FORMAT_R16G16B16A16_UINT,
            VK_FORMAT_R16G16B16A16_SNORM,
            VK_FORMAT_R16G16B16A16_SINT,
            VK_FORMAT_R32G32_SFLOAT,
            VK_FORMAT_R32G32_SFLOAT,
            VK_FORMAT_R32G32_UINT,
            VK_FORMAT_R32G32_SINT,
            VK_FORMAT_UNDEFINED, // TODO: R32G8X24_Typeless
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_UNDEFINED, // TODO: R32_Float_X8X24_Typeless
            VK_FORMAT_UNDEFINED, // TODO: X32_Typeless_G8X24_UInt
            VK_FORMAT_A2B10G10R10_UNORM_PACK32,
            VK_FORMAT_A2B10G10R10_UNORM_PACK32,
            VK_FORMAT_A2B10G10R10_UINT_PACK32,
            VK_FORMAT_B10G11R11_UFLOAT_PACK32,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_FORMAT_R8G8B8A8_UINT,
            VK_FORMAT_R8G8B8A8_SNORM,
            VK_FORMAT_R8G8B8A8_SINT,
            VK_FORMAT_R16G16_SFLOAT,
            VK_FORMAT_R16G16_SFLOAT,
            VK_FORMAT_R16G16_UNORM,
            VK_FORMAT_R16G16_UINT,
            VK_FORMAT_R16G16_SNORM,
            VK_FORMAT_R16G16_SINT,
            VK_FORMAT_R32_SFLOAT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_R32_SFLOAT,
            VK_FORMAT_R32_UINT,
            VK_FORMAT_R32_SINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_X8_D24_UNORM_PACK32,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_R8G8_UNORM,
            VK_FORMAT_R8G8_UNORM,
            VK_FORMAT_R8G8_UINT,
            VK_FORMAT_R8G8_SNORM,
            VK_FORMAT_R8G8_SINT,
            VK_FORMAT_R16_SFLOAT,
            VK_FORMAT_R16_SFLOAT,
            VK_FORMAT_D16_UNORM,
            VK_FORMAT_R16_UNORM,
            VK_FORMAT_R16_UINT,
            VK_FORMAT_R16_SNORM,
            VK_FORMAT_R16_SINT,
            VK_FORMAT_R8_UNORM,
            VK_FORMAT_R8_UNORM,
            VK_FORMAT_R8_UINT,
            VK_FORMAT_R8_SNORM,
            VK_FORMAT_R8_SINT,
            VK_FORMAT_UNDEFINED, // TODO: A8_UNorm
            VK_FORMAT_UNDEFINED, // TODO: R1_UNorm
            VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
            VK_FORMAT_UNDEFINED, // TODO: R8G8_B8G8_UNorm
            VK_FORMAT_UNDEFINED, // TODO: G8R8_G8B8_UNorm
            VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
            VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
            VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
            VK_FORMAT_BC2_UNORM_BLOCK,
            VK_FORMAT_BC2_UNORM_BLOCK,
            VK_FORMAT_BC2_SRGB_BLOCK,
            VK_FORMAT_BC3_UNORM_BLOCK,
            VK_FORMAT_BC3_UNORM_BLOCK,
            VK_FORMAT_BC3_SRGB_BLOCK,
            VK_FORMAT_BC4_UNORM_BLOCK,
            VK_FORMAT_BC4_UNORM_BLOCK,
            VK_FORMAT_BC4_SNORM_BLOCK,
            VK_FORMAT_BC5_UNORM_BLOCK,
            VK_FORMAT_BC5_UNORM_BLOCK,
            VK_FORMAT_BC5_SNORM_BLOCK,
            VK_FORMAT_B5G6R5_UNORM_PACK16,
            VK_FORMAT_B5G5R5A1_UNORM_PACK16,
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_UNDEFINED, // TODO: R10G10B10_Xr_Bias_A2_UNorm
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_B8G8R8A8_SRGB,
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_B8G8R8A8_SRGB,
            VK_FORMAT_BC6H_UFLOAT_BLOCK,
            VK_FORMAT_BC6H_UFLOAT_BLOCK,
            VK_FORMAT_BC6H_SFLOAT_BLOCK,
            VK_FORMAT_BC7_UNORM_BLOCK,
            VK_FORMAT_BC7_UNORM_BLOCK,
            VK_FORMAT_BC7_SRGB_BLOCK,
            VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
            VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
            VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
            VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
            VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
            VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
            VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
            VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
            VK_FORMAT_G8B8G8R8_422_UNORM,       // YUY2
            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, // NV12
    };

    VkBlendOp VkUtils::operationToVkBlendOp[6] =
        {
            VK_BLEND_OP_MAX_ENUM,
            VK_BLEND_OP_ADD,              // Add
            VK_BLEND_OP_SUBTRACT,         // Subtract
            VK_BLEND_OP_REVERSE_SUBTRACT, // RevSubtract
            VK_BLEND_OP_MIN,              // Min
            VK_BLEND_OP_MAX,              // Max
    };

    VkCompareOp VkUtils::comparisonFuncToVkCompareOp[9] =
        {
            VK_COMPARE_OP_MAX_ENUM,
            VK_COMPARE_OP_NEVER,            // Never
            VK_COMPARE_OP_LESS,             // Less
            VK_COMPARE_OP_EQUAL,            // Equal
            VK_COMPARE_OP_LESS_OR_EQUAL,    // LessEqual
            VK_COMPARE_OP_GREATER,          // Grather
            VK_COMPARE_OP_NOT_EQUAL,        // NotEqual
            VK_COMPARE_OP_GREATER_OR_EQUAL, // GratherEqual
            VK_COMPARE_OP_ALWAYS,           // Always
    };

    // @formatter:on

#define VKERR(x)       \
    case x:            \
        sb.append(#x); \
        break

#if GFX_ENABLE_RESOURCE_NAMING

    void VkUtils::setName(VkDevice device, u64 objectHandle, VkObjectType objectType, const TkString &name)
    {
#if VK_EXT_debug_utils
        setName(device, objectHandle, objectType, name.get());
#endif
    }

    void VkUtils::setName(VkDevice device, u64 objectHandle, VkObjectType objectType, const char *name)
    {
#if VK_EXT_debug_utils
        // Check for valid function pointer (may not be present if not running in a debugging application)
        if (vkSetDebugUtilsObjectNameEXT != nullptr && name != nullptr && *name != 0)
        {
            VkDebugUtilsObjectNameInfoEXT objectNameInfo;
            zeroStruct(objectNameInfo, VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT);
            objectNameInfo.objectType = objectType;
            objectNameInfo.objectHandle = objectHandle;
            objectNameInfo.pObjectName = name;
            const VkResult result = vkSetDebugUtilsObjectNameEXT(device, &objectNameInfo);
            LOG_VK_RESULT(result);
        }
#endif
    }

#endif

    TkString VkUtils::getErrorString(VkResult result)
    {
        TkStringBuilder sb(64);

        // Switch error code
        switch (result)
        {
            VKERR(VK_SUCCESS);
            VKERR(VK_NOT_READY);
            VKERR(VK_TIMEOUT);
            VKERR(VK_EVENT_SET);
            VKERR(VK_EVENT_RESET);
            VKERR(VK_INCOMPLETE);
            VKERR(VK_ERROR_OUT_OF_HOST_MEMORY);
            VKERR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
            VKERR(VK_ERROR_INITIALIZATION_FAILED);
            VKERR(VK_ERROR_DEVICE_LOST);
            VKERR(VK_ERROR_MEMORY_MAP_FAILED);
            VKERR(VK_ERROR_LAYER_NOT_PRESENT);
            VKERR(VK_ERROR_EXTENSION_NOT_PRESENT);
            VKERR(VK_ERROR_FEATURE_NOT_PRESENT);
            VKERR(VK_ERROR_INCOMPATIBLE_DRIVER);
            VKERR(VK_ERROR_TOO_MANY_OBJECTS);
            VKERR(VK_ERROR_FORMAT_NOT_SUPPORTED);
            VKERR(VK_ERROR_FRAGMENTED_POOL);
            VKERR(VK_ERROR_OUT_OF_POOL_MEMORY);
            VKERR(VK_ERROR_INVALID_EXTERNAL_HANDLE);
            VKERR(VK_ERROR_SURFACE_LOST_KHR);
            VKERR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
            VKERR(VK_SUBOPTIMAL_KHR);
            VKERR(VK_ERROR_OUT_OF_DATE_KHR);
            VKERR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
            VKERR(VK_ERROR_VALIDATION_FAILED_EXT);
            VKERR(VK_ERROR_INVALID_SHADER_NV);
#if VK_HEADER_VERSION >= 89
            VKERR(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
#endif
            VKERR(VK_ERROR_FRAGMENTATION_EXT);
            VKERR(VK_ERROR_NOT_PERMITTED_EXT);
#if VK_HEADER_VERSION < 140
            VKERR(VK_RESULT_RANGE_SIZE);
#endif
        default:
            sb.appendFormat("0x%x", static_cast<u32>(result));
            break;
        }

        return sb.toString();
    }

    void VkUtils::logResult(VkResult result, const char *file, u32 line, bool fatal)
    {
        TK_ASSERT(result != VK_SUCCESS);

        // Process error and format message
        TkStringBuilder sb;
        sb.append("Vulkan error: ");
        sb.append(getErrorString(result));
        if (file)
            sb.append(" at ").append(file).append(':').append(line);
        const TkStringView msg(sb.toStringView());

        // Handle error
        FatalErrorType errorType = FatalErrorType::kNone;
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_ERROR_OUT_OF_DEVICE_MEMORY || result == VK_ERROR_OUT_OF_POOL_MEMORY)
            errorType = FatalErrorType::kGPUOutOfMemory;
        else if (result == VK_TIMEOUT)
            errorType = FatalErrorType::kGPUHang;
        else if (result == VK_ERROR_DEVICE_LOST || result == VK_ERROR_SURFACE_LOST_KHR || result == VK_ERROR_MEMORY_MAP_FAILED)
            errorType = FatalErrorType::kGPUCrash;
        if (errorType != FatalErrorType::kNone) {
            LogError("[VkUtils] Error:%i Msg:%s", errorType, msg);
            TK_ASSERT(false);
        }
        else {
            if(fatal) {
                LogError("[VkUtils] Fatal %s", msg);
            } else {
                LogError("[VkUtils] Error %s", msg);
            }
        }
    }

    bool VkUtils::hasExtension(const TkArray<const char *> &extensions, const char *name)
    {
        for (i32 i = 0; i < extensions.count(); i++)
        {
            const char *extension = extensions[i];
            if (extension && TkStringUtils::compareStrings(extension, name) == 0)
                return true;
        }
        return false;
    }

}

#endif
