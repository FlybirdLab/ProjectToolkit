/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/utils/PixelFormat.hpp"
#include <toolkit/graphics/common.hpp>

#if GFX_API_VULKAN

#include <toolkit/platforms/vulkan/environment.hpp>
#include <toolkit/base/Math.hpp>

#if TK_WINDOWS
#include <toolkit/platforms/win32/IncludeWindows.hpp>
#endif

// Use volk as is a meta-loader for Vulkan
#include <volk.h>

// Use Vulkan Memory Allocator for buffer and image memory allocations
#define VMA_MIN(v1, v2) (TkMath::min((v1), (v2)))
#define VMA_MAX(v1, v2) (TkMath::max((v1), (v2)))
#define VMA_SWAP(v1, v2) (toolkit::swap((v1), (v2)))
#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) TkMemory::allocate(TkMemory::getDefault(), u64(size), u64(alignment))
#define VMA_SYSTEM_ALIGNED_FREE(ptr) TkMemory::free(TkMemory::getDefault(), ptr)
#define VMA_NULLABLE
#define VMA_NOT_NULL
#include <vk_mem_alloc.h>

#if TK_MACOS || TK_IOS
// Declare potentially missing extensions from newer SDKs
#ifndef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME "VK_KHR_portability_enumeration"
#define VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR 0x00000001
#endif
#ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#endif
#endif

namespace toolkit
{

    // // Hack to use VkDescriptorType if Vulkan runtime is not used
    // typedef enum VkDescriptorType
    // {
    //     VK_DESCRIPTOR_TYPE_SAMPLER = 0,
    //     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
    //     VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
    //     VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
    //     VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
    //     VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
    //     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
    //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
    //     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
    //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
    //     VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
    //     VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT = 1000138000,
    //     VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
    //     VK_DESCRIPTOR_TYPE_BEGIN_RANGE = VK_DESCRIPTOR_TYPE_SAMPLER,
    //     VK_DESCRIPTOR_TYPE_END_RANGE = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    //     VK_DESCRIPTOR_TYPE_RANGE_SIZE = (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT - VK_DESCRIPTOR_TYPE_SAMPLER + 1),
    //     VK_DESCRIPTOR_TYPE_MAX_ENUM = 0x7FFFFFFF
    // } VkDescriptorType;

    enum class SpirvShaderResourceType
    {
        kUnknown = 0,
        kConstantBuffer = 1,
        kBuffer = 2,
        kSampler = 3,
        kTexture1D = 4,
        kTexture2D = 5,
        kTexture3D = 6,
        kTextureCube = 7,
        kTexture1DArray = 8,
        kTexture2DArray = 9,
    };

    enum class SpirvShaderResourceBindingType : byte
    {
        kINVALID = 0,
        kCB = 1,
        kSAMPLER = 2,
        kSRV = 3,
        kUAV = 4,
        MAX
    };

    struct SpirvShaderDescriptorInfo
    {
        enum
        {
            kMaxDescriptors = 64,
        };

        // A single descriptor data.
        struct Descriptor
        {
            // The binding slot (the descriptors slot).
            byte binding;
            // The layout slot (the descriptors set slot).
            byte set;
            // The input slot (the pipeline slot).
            byte slot;
            // The resource binding type (the graphics pipeline abstraction binding layer type).
            SpirvShaderResourceBindingType bindingType;
            // The Vulkan descriptor type.
            VkDescriptorType descriptorType;
            // The resource type.
            SpirvShaderResourceType resourceType;
            // The resource format.
            PixelFormat resourceFormat;
            // The amount of slots used by the descriptor (eg. array of textures size).
            u32 count;
        };
        u16 imageInfosCount;
        u16 bufferInfosCount;
        u32 texelBufferViewsCount;
        u32 descriptorTypesCount;
        Descriptor descriptorTypes[kMaxDescriptors];
    };

    struct SpirvShaderHeader
    {
        enum class Types
        {
            // The raw SPIR-V byte code.
            kRaw = 0,
        };
        // The data type
        Types type;
        // The shader descriptors usage information
        SpirvShaderDescriptorInfo info;
    };
}

#endif
