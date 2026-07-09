/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "VkDevice.hpp"
#include "toolkit/platforms/vulkan/VkResourceOwner.hpp"
#include "toolkit/graphics/GfxConstantBuffer.hpp"
#include "toolkit/graphics/GfxShader.hpp"

namespace toolkit
{
    // The shared ring buffer for uniform buffers uploading for Vulkan backend.
    class VulkanUniformBufferUploader : public VulkanResource<GfxResource>, public VulkanResourceOwner
    {
    public:
        struct Allocation
        {
            // The allocation offset from the GPU buffer begin (in bytes).
            u64 offset;
            // The allocation size (in bytes).
            u64 size;
            // The GPU buffer.
            VkBuffer buffer;
            // The CPU memory address to the mapped buffer data. Can be used to write the uniform buffer contents to upload them to GPU.
            byte *address;
        };

    private:
        VkBuffer m_buffer;
        VmaAllocation m_allocation;
        u64 m_size;
        u64 m_offset;
        u32 m_minAlignment;
        byte *m_mapped;
        VulkanCmdBuffer *m_fenceCmdBuffer;
        u64 m_fenceCounter;

    public:
        VulkanUniformBufferUploader(VulkanDevice *device);

    public:
        Allocation allocate(u64 size, u32 alignment, VulkanContext *context);

    public:
        GfxResType getType() const final override
        {
            return GfxResType::kBuffer;
        }

        GfxResource *asResource() const override
        {
            return (GfxResource *)this;
        }

    protected:
        void onReleaseData() override;
    };

    // Constant Buffer for Vulkan backend.
    class VulkanConstantBuffer : public VulkanResource<GfxConstantBuffer>, public VulkanDescriptorOwnerResource
    {
    public:
        VulkanConstantBuffer(VulkanDevice *device, u32 size) noexcept
            : VulkanResource(device, TkString::Empty)
        {
            m_size = size;
        }

    public:
        VulkanUniformBufferUploader::Allocation allocation;

    public:
        void descriptorAsDynamicUniformBuffer(VulkanContext *context, VkBuffer &buffer, VkDeviceSize &offset, VkDeviceSize &range, u32 &dynamicOffset) override
        {
            buffer = allocation.buffer;
            offset = 0;
            range = allocation.size;
            dynamicOffset = (u32)allocation.offset;
        }
    };

    // Shader for Vulkan backend.
    class VulkanShader : public VulkanResource<GfxShader>
    {
    public:
        VulkanShader(VulkanDevice *device, const TkStringView &name)
            : VulkanResource<GfxShader>(device, name)
        {
        }

    protected:
        GfxShaderProgram *createShaderProgram(ShaderStage type, const GfxShaderProgramInitializer &initializer, TkSpan<byte> bytecode, MemoryIStream &stream) override;
    };

}

#endif