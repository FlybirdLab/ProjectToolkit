/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkShader.hpp"

#if GFX_API_VULKAN

#include "VkUtils.hpp"
#include "VkCmdBuffer.hpp"
#include "VkContext.hpp"
#include "toolkit/debug/Logger.hpp"
#include "VkShaderProgram.hpp"
#include "toolkit/types/Span.hpp"
#include "toolkit/io/MemoryStream.hpp"
#include "toolkit/collection/DataCollection.hpp"

namespace toolkit
{

#if TK_WINDOWS || TK_LINUX || TK_MACOS
#define VK_UNIFORM_RING_BUFFER_SIZE (24 * 1024 * 1024)
#else
#define VK_UNIFORM_RING_BUFFER_SIZE (8 * 1024 * 1024)
#endif

    VulkanUniformBufferUploader::VulkanUniformBufferUploader(VulkanDevice *device)
        : VulkanResource(device, "Uniform Buffer Uploader"), m_size(VK_UNIFORM_RING_BUFFER_SIZE), m_offset(0), m_mapped(nullptr), m_fenceCmdBuffer(nullptr), m_fenceCounter(0)
    {
        m_minAlignment = (u32)device->physicalDeviceLimits.minUniformBufferOffsetAlignment;

        // Setup buffer description
        VkBufferCreateInfo bufferInfo;
        VkUtils::zeroStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
        bufferInfo.size = m_size;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        // Create buffer
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        VkResult result = vmaCreateBuffer(m_device->allocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
        LOG_VK_RESULT(result);
        m_memoryUsage = bufferInfo.size;

        // Map buffer
        result = vmaMapMemory(m_device->allocator, m_allocation, (void **)&m_mapped);
        LOG_VK_RESULT(result);
    }

    VulkanUniformBufferUploader::Allocation VulkanUniformBufferUploader::allocate(u64 size, u32 alignment, VulkanContext *context)
    {
        alignment = TkMath::max(m_minAlignment, alignment);
        u64 offset = TkMath::alignUp<u64>(m_offset, alignment);

        // Check if wrap around ring buffer
        if (offset + size >= m_size)
        {
            auto cmdBuffer = context->getCmdBufferManager()->getActiveCmdBuffer();
            if (m_fenceCmdBuffer && m_fenceCounter == cmdBuffer->getFenceSignaledCounter())
            {
                LogError("Wrapped around the ring buffer! Need to wait on the GPU!");
                context->flush();
                cmdBuffer = context->getCmdBufferManager()->getActiveCmdBuffer();
            }

            offset = 0;
            m_offset = size;

            m_fenceCmdBuffer = cmdBuffer;
            m_fenceCounter = cmdBuffer->getSubmittedFenceCounter();
        }
        else
        {
            // Move within the buffer
            m_offset = offset + size;
        }

        Allocation result;
        result.offset = offset;
        result.size = size;
        result.buffer = m_buffer;
        result.address = m_mapped + offset;
        return result;
    }

    void VulkanUniformBufferUploader::onReleaseData()
    {
        if (m_allocation != VK_NULL_HANDLE)
        {
            if (m_mapped)
            {
                vmaUnmapMemory(m_device->allocator, m_allocation);
                m_mapped = nullptr;
            }
            vmaDestroyBuffer(m_device->allocator, m_buffer, m_allocation);
            m_buffer = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
        }
    }

    GfxShaderProgram *VulkanShader::createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream)
    {
        // Extract the SPIR-V shader header from the cache
        SpirvShaderHeader *header = (SpirvShaderHeader *)bytecode.get();
        bytecode = bytecode.slice(sizeof(SpirvShaderHeader));

        // Extract the SPIR-V bytecode
        ByteCollection spirv;
        TK_ASSERT(header->type == SpirvShaderHeader::Types::kRaw);
        spirv.link(bytecode);

        // Create shader module from SPIR-V bytecode
        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VkShaderModuleCreateInfo createInfo;
        VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
        createInfo.codeSize = (size_t)spirv.length();
        createInfo.pCode = (const u32 *)spirv.get();
#if VK_USE_VALIDATION_CACHE
        VkShaderModuleValidationCacheCreateInfoEXT validationInfo;
        if (m_device->validationCache != VK_NULL_HANDLE)
        {
            VkUtils::zeroStruct(validationInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT);
            validationInfo.validationCache = m_device->validationCache;
            createInfo.pNext = &validationInfo;
        }
#endif
        VALIDATE_VK_RESULT(vkCreateShaderModule(m_device->device, &createInfo, nullptr, &shaderModule));
#if GFX_ENABLE_RESOURCE_NAMING
        VK_SET_DEBUG_NAME(m_device, shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, initializer.name.getText());
#endif
#if 0
    // Helper debug print of the shader module handle (Debug Layer sometimes doesn't print the debug name but the handle)
    LogInfo("VK_OBJECT_TYPE_SHADER_MODULE=0x{0:x}, {1}", (uintptr)shaderModule, initializer.name.getText());
#endif

        GfxShaderProgram *shader = nullptr;
        switch (type)
        {
        case ShaderStage::kVertex:
        {
            GfxVertexLayout *inputLayout, *vertexLayout;
            readVertexLayout(stream, inputLayout, vertexLayout);
            shader = TkMemory::newObject<VulkanShaderProgramVS>(TkMemory::getDefault(), m_device, initializer, header->info, shaderModule, inputLayout, vertexLayout);
            break;
        }
#if GFX_ALLOW_TESSELLATION_SHADERS
        case ShaderStage::kHull:
        {
            i32 controlPointsCount;
            stream.readI32(&controlPointsCount);
            shader = TkMemory::newObject<VulkanShaderProgramHS>(TkMemory::getDefault(), m_device, initializer, header->info, shaderModule, controlPointsCount);
            break;
        }
        case ShaderStage::kDomain:
        {
            shader = TkMemory::newObject<VulkanShaderProgramDS>(TkMemory::getDefault(), m_device, initializer, header->info, shaderModule);
            break;
        }
#else
        case ShaderStage::kHull:
        {
            i32 controlPointsCount;
            stream.ReadI32(&controlPointsCount);
            break;
        }
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        case ShaderStage::kGeometry:
        {
            shader = TkMemory::newObject<VulkanShaderProgramGS>(TkMemory::getDefault(), m_device, initializer, header->info, shaderModule);
            break;
        }
#endif
        case ShaderStage::kPixel:
        {
            shader = TkMemory::newObject<VulkanShaderProgramPS>(TkMemory::getDefault(), m_device, initializer, header->info, shaderModule);
            break;
        }
        case ShaderStage::kCompute:
        {
            shader = TkMemory::newObject<VulkanShaderProgramCS>(TkMemory::getDefault(), m_device, initializer, header->info, shaderModule);
            break;
        }
        }
        return shader;
    }

}

#endif