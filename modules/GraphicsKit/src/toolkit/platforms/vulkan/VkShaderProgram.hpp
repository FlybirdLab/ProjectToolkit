/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "toolkit/graphics/GfxShaderProgram.hpp"
#include <toolkit/graphics/environment.hpp>
#include "VkDevice.hpp"

namespace toolkit
{

    // Shaders base class for Vulkan backend.
    template <typename BaseType>
    class VulkanShaderProgram : public BaseType
    {
    protected:
        VulkanDevice *m_device;

    public:
        VulkanShaderProgram(VulkanDevice *device, const GfxShaderProgramInitializer &initializer, const SpirvShaderDescriptorInfo &descriptorInfo, VkShaderModule shaderModule)
            : m_device(device), shaderModule(shaderModule), descriptorInfo(descriptorInfo)
        {
            BaseType::init(initializer);
        }

        ~VulkanShaderProgram()
        {
            if (shaderModule)
            {
                m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::kShaderModule, shaderModule);
            }
        }

    public:
        // The Vulkan shader module.
        VkShaderModule shaderModule;
        // The descriptor information container.
        SpirvShaderDescriptorInfo descriptorInfo;

    public:
        u32 getBufferSize() const override
        {
            return 0;
        }
        void *getBufferHandle() const override
        {
            return (void *)shaderModule;
        }
    };

    // Vertex Shader for Vulkan backend.
    class VulkanShaderProgramVS : public VulkanShaderProgram<GfxShaderProgramVS>
    {
    public:
        VulkanShaderProgramVS(VulkanDevice *device, const GfxShaderProgramInitializer &initializer, const SpirvShaderDescriptorInfo &descriptorInfo, VkShaderModule shaderModule, GfxVertexLayout *inputLayout, GfxVertexLayout *vertexLayout)
            : VulkanShaderProgram(device, initializer, descriptorInfo, shaderModule)
        {
            this->inputLayout = inputLayout;
            this->layout = vertexLayout;
        }
    };

#if GFX_ALLOW_TESSELLATION_SHADERS
    // Hull Shader for Vulkan backend
    class VulkanShaderProgramHS : public VulkanShaderProgram<GfxShaderProgramHS>
    {
    public:
        VulkanShaderProgramHS(VulkanDevice *device, const GfxShaderProgramInitializer &initializer, const SpirvShaderDescriptorInfo &descriptorInfo, VkShaderModule shaderModule, i32 controlPointsCount)
            : VulkanShaderProgram(device, initializer, descriptorInfo, shaderModule)
        {
            m_controlPointsCount = controlPointsCount;
        }
    };
    // Domain Shader for Vulkan backend
    class VulkanShaderProgramDS : public VulkanShaderProgram<GfxShaderProgramDS>
    {
    public:
        VulkanShaderProgramDS(VulkanDevice *device, const GfxShaderProgramInitializer &initializer, const SpirvShaderDescriptorInfo &descriptorInfo, VkShaderModule shaderModule)
            : VulkanShaderProgram(device, initializer, descriptorInfo, shaderModule)
        {
        }
    };
#endif

#if GFX_ALLOW_GEOMETRY_SHADERS
    // Geometry Shader for Vulkan backend
    class VulkanShaderProgramGS : public VulkanShaderProgram<GfxShaderProgramGS>
    {
    public:
        VulkanShaderProgramGS(VulkanDevice *device, const GfxShaderProgramInitializer &initializer, const SpirvShaderDescriptorInfo &descriptorInfo, VkShaderModule shaderModule)
            : VulkanShaderProgram(device, initializer, descriptorInfo, shaderModule)
        {
        }
    };
#endif
    // Pixel Shader for Vulkan backend
    class VulkanShaderProgramPS : public VulkanShaderProgram<GfxShaderProgramPS>
    {
    public:
        VulkanShaderProgramPS(VulkanDevice *device, const GfxShaderProgramInitializer &initializer, const SpirvShaderDescriptorInfo &descriptorInfo, VkShaderModule shaderModule)
            : VulkanShaderProgram(device, initializer, descriptorInfo, shaderModule)
        {
        }
    };
    // Compute Shader for Vulkan backend
    class VulkanShaderProgramCS : public VulkanShaderProgram<GfxShaderProgramCS>
    {
    private:
        class VulkanComputePipelineState *m_pipelineState;

    public:
        VulkanShaderProgramCS(VulkanDevice *device, const GfxShaderProgramInitializer &initializer, const SpirvShaderDescriptorInfo &descriptorInfo, VkShaderModule shaderModule)
            : VulkanShaderProgram(device, initializer, descriptorInfo, shaderModule), m_pipelineState(nullptr)
        {
        }

        ~VulkanShaderProgramCS();

    public:
        // Gets the state of the pipeline for the compute shader execution or creates a new one if missing.
        VulkanComputePipelineState *getOrCreateState();
    };

}

#endif