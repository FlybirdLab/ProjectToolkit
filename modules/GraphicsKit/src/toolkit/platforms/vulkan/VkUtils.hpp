/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "toolkit/memory/Memory.hpp"
#include "toolkit/graphics/GfxSampler.hpp"

namespace toolkit
{

#define VALIDATE_VK_RESULT(x)                                     \
    {                                                             \
        VkResult result = x;                                      \
        if (result != VK_SUCCESS)                                 \
            VkUtils::logResult(result, __FILE__, __LINE__, true); \
    }
#define LOG_VK_RESULT(result) \
    if (result != VK_SUCCESS) \
    VkUtils::logResult(result, __FILE__, __LINE__)
#define LOG_VK_RESULT_WITH_RETURN(result)               \
    if (result != VK_SUCCESS)                           \
    {                                                   \
        VkUtils::logResult(result, __FILE__, __LINE__); \
        return false;                                   \
    }
#if GFX_ENABLE_ASSERTION
#define VK_SET_DEBUG_NAME(d, handle, type, name) VkUtils::setName(d->device, (u64)handle, type, name)
#else
#define VK_SET_DEBUG_NAME(device, handle, type, name)
#endif

    // Set of utilities for rendering on Vulkan platform.
    class VkUtils
    {
    private:
        static VkFormat pixelFormatToVkFormat[static_cast<i32>(PixelFormat::MAX)];
        static VkBlendFactor blendToVkBlendFactor[static_cast<i32>(GfxBlendingMode::Blend::kMax)];
        static VkBlendOp operationToVkBlendOp[6]; // HACK:static_cast<i32>(GfxBlendingMode::Operation::kMax)
        static VkCompareOp comparisonFuncToVkCompareOp[static_cast<i32>(GfxComparisonFunc::kMax)];

    public:
#if GFX_ENABLE_RESOURCE_NAMING
        static void setName(VkDevice device, u64 objectHandle, VkObjectType objectType, const TkString &name);
        static void setName(VkDevice device, u64 objectHandle, VkObjectType objectType, const char *name);
#endif

        static TkString getErrorString(VkResult result);
        static void logResult(VkResult result, const char *file = nullptr, u32 line = 0, bool fatal = false);

        static inline VkPipelineStageFlags getBufferBarrierFlags(VkAccessFlags accessFlags)
        {
            VkPipelineStageFlags stageFlags = (VkPipelineStageFlags)0;
            switch (accessFlags)
            {
            case 0:
                stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                break;
            case VK_ACCESS_INDIRECT_COMMAND_READ_BIT:
                stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
                break;
            case VK_ACCESS_TRANSFER_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                break;
            case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_ACCESS_TRANSFER_READ_BIT:
                stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VK_ACCESS_SHADER_READ_BIT:
            case VK_ACCESS_SHADER_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
            case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            default:
                TK_ASSERT(false && "VkUtils getBufferBarrierFlags");
                break;
            }
            return stageFlags;
        }

        static inline VkPipelineStageFlags getImageBarrierFlags(VkImageLayout layout, VkAccessFlags &accessFlags)
        {
            VkPipelineStageFlags stageFlags;
            switch (layout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                accessFlags = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                accessFlags = VK_ACCESS_TRANSFER_READ_BIT;
                stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                accessFlags = VK_ACCESS_NONE;
                stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                accessFlags = VK_ACCESS_SHADER_READ_BIT;
                stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            default:
                TK_ASSERT(false && "VkUtils getImageBarrierFlags");
                break;
            }
            return stageFlags;
        }

        template <class T>
        static TK_FORCE_INLINE void zeroStruct(T &data, VkStructureType type)
        {
            static_assert(!TIsPointer<T>::Value, "Don't use a pointer.");
            static_assert(TK_OFFSET_OF(T, sType) == 0, "Assumes type is the first member in the Vulkan type.");
            data.sType = type;
            TkMemory::memoryClear((u8 *)&data + sizeof(VkStructureType), sizeof(T) - sizeof(VkStructureType));
        }

        // Converts Flax Pixel Format to the Vulkan Format.
        static TK_FORCE_INLINE VkFormat toVulkanFormat(const PixelFormat value)
        {
            return pixelFormatToVkFormat[(i32)value];
        }

        // Converts Flax blend operation to the Vulkan blend operation.
        static TK_FORCE_INLINE VkBlendOp toVulkanBlendOp(const GfxBlendingMode::Operation value)
        {
            return operationToVkBlendOp[(i32)value];
        }

        // Converts Flax comparison function to the Vulkan comparison operation.
        static TK_FORCE_INLINE VkCompareOp toVulkanCompareOp(const GfxComparisonFunc value)
        {
            return comparisonFuncToVkCompareOp[(i32)value];
        }

        static VkSamplerMipmapMode toVulkanMipFilterMode(GfxSamplerFilter filter)
        {
            VkSamplerMipmapMode result;
            switch (filter)
            {
            case GfxSamplerFilter::kPoint:
                result = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                break;
            case GfxSamplerFilter::kBilinear:
                result = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                break;
            case GfxSamplerFilter::kTrilinear:
                result = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                break;
            case GfxSamplerFilter::kAnisotropic:
                result = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                break;
            default:
                TK_ASSERT(false && "VkUtils toVulkanMipFilterMode");
                break;
            }
            return result;
        }

        static VkFilter toVulkanMagFilterMode(GfxSamplerFilter filter)
        {
            VkFilter result;
            switch (filter)
            {
            case GfxSamplerFilter::kPoint:
                result = VK_FILTER_NEAREST;
                break;
            case GfxSamplerFilter::kBilinear:
                result = VK_FILTER_LINEAR;
                break;
            case GfxSamplerFilter::kTrilinear:
                result = VK_FILTER_LINEAR;
                break;
            case GfxSamplerFilter::kAnisotropic:
                result = VK_FILTER_LINEAR;
                break;
            default:
                TK_ASSERT(false && "VkUtils toVulkanMagFilterMode");
                break;
            }
            return result;
        }

        static VkFilter toVulkanMinFilterMode(GfxSamplerFilter filter)
        {
            VkFilter result;
            switch (filter)
            {
            case GfxSamplerFilter::kPoint:
                result = VK_FILTER_NEAREST;
                break;
            case GfxSamplerFilter::kBilinear:
                result = VK_FILTER_LINEAR;
                break;
            case GfxSamplerFilter::kTrilinear:
                result = VK_FILTER_LINEAR;
                break;
            case GfxSamplerFilter::kAnisotropic:
                result = VK_FILTER_LINEAR;
                break;
            default:
                TK_ASSERT(false && "VkUtils toVulkanMinFilterMode");
                break;
            }
            return result;
        }

        static VkSamplerAddressMode toVulkanWrapMode(GfxSamplerAddressMode addressMode, const bool supportsMirrorClampToEdge)
        {
            VkSamplerAddressMode result;
            switch (addressMode)
            {
            case GfxSamplerAddressMode::kWrap:
                result = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                break;
            case GfxSamplerAddressMode::kClamp:
                result = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;
            case GfxSamplerAddressMode::kMirror:
                result = supportsMirrorClampToEdge ? VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;
            case GfxSamplerAddressMode::kBorder:
                result = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                break;
            default:
                TK_ASSERT(false && "VkUtils toVulkanWrapMode");
                break;
            }
            return result;
        }

        static VkCompareOp toVulkanSamplerCompareFunction(GfxSamplerCompareFunction samplerComparisonFunction)
        {
            VkCompareOp result;
            switch (samplerComparisonFunction)
            {
            case GfxSamplerCompareFunction::kLess:
                result = VK_COMPARE_OP_LESS;
                break;
            case GfxSamplerCompareFunction::kNever:
                result = VK_COMPARE_OP_NEVER;
                break;
            default:
                TK_ASSERT(false && "VkUtils toVulkanSamplerCompareFunction");
                break;
            }
            return result;
        }

        static bool hasExtension(const TkArray<const char *> &extensions, const char *name);
    };

}

#endif
