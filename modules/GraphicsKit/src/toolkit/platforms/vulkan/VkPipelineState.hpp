/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "VkDescriptorSet.hpp"
#include "VkCmdBuffer.hpp"
#include "VkDevice.hpp"
#include "toolkit/graphics/GfxPipelineState.hpp"

namespace toolkit
{
    class VulkanComputePipelineState
    {
    private:
        class VulkanDevice *m_device;
        VkPipeline m_handle;
        class VulkanPipelineLayout *m_layout;

    public:
        VulkanComputePipelineState(VulkanDevice *device, VkPipeline pipeline, VulkanPipelineLayout *layout);
        ~VulkanComputePipelineState();

    public:
        // The cached shader descriptor infos for compute shader.
        const SpirvShaderDescriptorInfo *descriptorInfo;

        VulkanDescriptorSetWriteContainer dsWriteContainer;
        VulkanDescriptorSetWriter dsWriter;

        const VulkanDescriptorSetLayout *descriptorSetsLayout = nullptr;
        VulkanTypedDescriptorPoolSet *currentTypedDescriptorPoolSet = nullptr;
        TkArray<VkDescriptorSet> descriptorSetHandles;

        inline bool acquirePoolSet(VulkanCmdBuffer *cmdBuffer)
        {
            // Pipeline state has no current descriptor pools set or set owner is not current - acquire a new pool set
            VulkanDescriptorPoolSetContainer *cmdBufferPoolSet = cmdBuffer->getDescriptorPoolSet();
            if (currentTypedDescriptorPoolSet == nullptr || currentTypedDescriptorPoolSet->getOwner() != cmdBufferPoolSet)
            {
                if (currentTypedDescriptorPoolSet)
                    currentTypedDescriptorPoolSet->getOwner()->refs--;
                currentTypedDescriptorPoolSet = cmdBufferPoolSet->acquireTypedPoolSet(*descriptorSetsLayout);
                currentTypedDescriptorPoolSet->getOwner()->refs++;
                return true;
            }
            return false;
        }

        inline bool allocateDescriptorSets()
        {
            return currentTypedDescriptorPoolSet->allocateDescriptorSets(*descriptorSetsLayout, descriptorSetHandles.get());
        }

        TkArray<u32> dynamicOffsets;

    public:
        void bind(VulkanCmdBuffer *cmdBuffer)
        {
            vkCmdBindDescriptorSets(
                cmdBuffer->getHandle(),
                VK_PIPELINE_BIND_POINT_COMPUTE,
                getLayout()->handle,
                0,
                descriptorSetHandles.count(),
                descriptorSetHandles.get(),
                dynamicOffsets.count(),
                dynamicOffsets.get());
        }

    public:
        VkPipeline getHandle() const
        {
            return m_handle;
        }

        VulkanPipelineLayout *getLayout() const
        {
            return m_layout;
        }
    };
    
    // Graphics pipeline state object for Vulkan backend.
    class VulkanPipelineState : public VulkanResource<GfxPipelineState>
    {
    private:
        TkHashMap<TkPair<VulkanRenderPass *, class VulkanVertexLayout *>, VkPipeline> m_pipelines;
        VkGraphicsPipelineCreateInfo m_desc;
        VkPipelineShaderStageCreateInfo m_shaderStages[ShaderStage_const_size - 1];
        VkPipelineInputAssemblyStateCreateInfo m_descInputAssembly;
#if GFX_ALLOW_TESSELLATION_SHADERS
        VkPipelineTessellationStateCreateInfo m_descTessellation;
#endif
        VkPipelineViewportStateCreateInfo m_descViewport;
        VkPipelineDynamicStateCreateInfo m_descDynamic;
        VkDynamicState m_dynamicStates[4];
        VkPipelineMultisampleStateCreateInfo m_descMultisample;
        VkPipelineDepthStencilStateCreateInfo m_descDepthStencil;
        VkPipelineRasterizationStateCreateInfo m_descRasterization;
        VkPipelineColorBlendStateCreateInfo m_descColorBlend;
        VkPipelineColorBlendAttachmentState m_descColorBlendAttachments[GFX_MAX_RT_BINDED];
        VulkanPipelineLayout *m_layout;

    public:
        VulkanPipelineState(VulkanDevice *device);

    public:
        // The bitmask of stages that exist in this pipeline.
        u32 usedStagesMask;

        u32 blendEnable : 1;
        u32 depthReadEnable : 1;
        u32 depthWriteEnable : 1;
        u32 stencilReadEnable : 1;
        u32 stencilWriteEnable : 1;

        // The bitmask of stages that have descriptors.
        u32 hasDescriptorsPerStageMask;

        // The cached shader bindings per stage.
        const ShaderBindings *shaderBindingsPerStage[VulkanDescriptorSet::kGraphicsStagesCount];
        // The cached shader descriptor infos per stage.
        const SpirvShaderDescriptorInfo *descriptorInfoPerStage[VulkanDescriptorSet::kGraphicsStagesCount];

        VulkanDescriptorSetWriteContainer dsWriteContainer;
        VulkanDescriptorSetWriter dsWriter[VulkanDescriptorSet::kGraphicsStagesCount];

        const VulkanDescriptorSetLayout *descriptorSetsLayout = nullptr;
        VulkanTypedDescriptorPoolSet *currentTypedDescriptorPoolSet = nullptr;
        VulkanVertexLayout *vertexInputLayout = nullptr;
        VulkanVertexLayout *vertexBufferLayout = nullptr;
        TkArray<VkDescriptorSet> descriptorSetHandles;
        TkArray<u32> dynamicOffsets;

    public:
        inline bool acquirePoolSet(VulkanCmdBuffer *cmdBuffer)
        {
            // Lazy init
            if (!descriptorSetsLayout)
                getLayout();

            // Pipeline state has no current descriptor pools set or set owner is not current - acquire a new pool set
            VulkanDescriptorPoolSetContainer *cmdBufferPoolSet = cmdBuffer->getDescriptorPoolSet();
            if (currentTypedDescriptorPoolSet == nullptr || currentTypedDescriptorPoolSet->getOwner() != cmdBufferPoolSet)
            {
                if (currentTypedDescriptorPoolSet)
                    currentTypedDescriptorPoolSet->getOwner()->refs--;
                currentTypedDescriptorPoolSet = cmdBufferPoolSet->acquireTypedPoolSet(*descriptorSetsLayout);
                currentTypedDescriptorPoolSet->getOwner()->refs++;
                return true;
            }
            return false;
        }

        // Gets the Vulkan pipeline layout for this pipeline state.
        VulkanPipelineLayout *getLayout();
        // Gets the Vulkan graphics pipeline object for the given rendering state. Uses depth buffer and render targets formats and multi-sample levels to setup a proper PSO. Uses caching.
        VkPipeline getState(VulkanRenderPass *renderPass, VulkanVertexLayout *vertexLayout);

    public:
        bool isValid() const final override;
        bool init(const Description &desc) final override;

    protected:
        void onReleaseData() override;
    };

}

#endif