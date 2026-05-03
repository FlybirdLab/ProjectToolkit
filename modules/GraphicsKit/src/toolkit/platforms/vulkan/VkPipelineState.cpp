/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkPipelineState.hpp"
#include "VkVertexLayout.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"

#if GFX_API_VULKAN

#include "VkShaderProgram.hpp"
#include "VkUtils.hpp"
#include "toolkit/types/Pair.hpp"

namespace toolkit
{

    static VkStencilOp toVulkanStencilOp(const StencilOperation value)
    {
        switch (value)
        {
        case StencilOperation::kKeep:
            return VK_STENCIL_OP_KEEP;
        case StencilOperation::kZero:
            return VK_STENCIL_OP_ZERO;
        case StencilOperation::kReplace:
            return VK_STENCIL_OP_REPLACE;
        case StencilOperation::kIncrementSaturated:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOperation::kDecrementSaturated:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOperation::kInvert:
            return VK_STENCIL_OP_INVERT;
        case StencilOperation::kIncrement:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOperation::kDecrement:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default:
            return VK_STENCIL_OP_KEEP;
        }
    }

    static VkBlendFactor toVulkanBlendFactor(const GfxBlendingMode::Blend value)
    {
        switch (value)
        {
        case GfxBlendingMode::Blend::kZero:
            return VK_BLEND_FACTOR_ZERO;
        case GfxBlendingMode::Blend::kOne:
            return VK_BLEND_FACTOR_ONE;
        case GfxBlendingMode::Blend::kSrcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case GfxBlendingMode::Blend::kInvSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case GfxBlendingMode::Blend::kSrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case GfxBlendingMode::Blend::kInvSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case GfxBlendingMode::Blend::kDestAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case GfxBlendingMode::Blend::kInvDestAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case GfxBlendingMode::Blend::kDestColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case GfxBlendingMode::Blend::kInvDestColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case GfxBlendingMode::Blend::kSrcAlphaSat:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case GfxBlendingMode::Blend::kBlendFactor:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case GfxBlendingMode::Blend::kBlendInvFactor:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case GfxBlendingMode::Blend::kSrc1Color:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case GfxBlendingMode::Blend::kInvSrc1Color:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case GfxBlendingMode::Blend::kSrc1Alpha:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case GfxBlendingMode::Blend::kInvSrc1Alpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default:
            return VK_BLEND_FACTOR_ZERO;
        }
    }

    VulkanShaderProgramCS::~VulkanShaderProgramCS()
    {
        if (m_pipelineState)
            TkMemory::deletePtr(m_pipelineState);
    }

    VulkanComputePipelineState *VulkanShaderProgramCS::getOrCreateState()
    {
        if (m_pipelineState)
            return m_pipelineState;

        // Create pipeline layout
        VulkanDescriptorSetLayoutInfo descriptorSetLayoutInfo;
        descriptorSetLayoutInfo.addBindingsForStage(VK_SHADER_STAGE_COMPUTE_BIT, VulkanDescriptorSet::kCompute, &descriptorInfo);
        const auto layout = m_device->getOrCreateLayout(descriptorSetLayoutInfo);

        // Create pipeline description
        VkComputePipelineCreateInfo desc;
        VkUtils::zeroStruct(desc, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
        desc.basePipelineIndex = -1;
        desc.layout = layout->handle;
        VkUtils::zeroStruct(desc.stage, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
        auto &stage = desc.stage;
        VkUtils::zeroStruct(stage, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
        stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stage.module = (VkShaderModule)getBufferHandle();
        stage.pName = getName().get();

        // Create pipeline object
        VkPipeline pipeline;
        const VkResult result = vkCreateComputePipelines(m_device->device, m_device->pipelineCache, 1, &desc, nullptr, &pipeline);
        LOG_VK_RESULT(result);
        if (result != VK_SUCCESS)
            return nullptr;

        // Setup the state
        m_pipelineState = TkMemory::newObject<VulkanComputePipelineState>(TkMemory::getDefault(), m_device, pipeline, layout);
        m_pipelineState->descriptorInfo = &descriptorInfo;
        m_pipelineState->descriptorSetsLayout = &layout->descriptorSetLayout;
        m_pipelineState->descriptorSetHandles.addZeroed(m_pipelineState->descriptorSetsLayout->handles.count());
        u32 dynamicOffsetsCount = 0;
        if (descriptorInfo.descriptorTypesCount != 0)
        {
            // TODO: merge into a single allocation
            m_pipelineState->dsWriteContainer.descriptorWrites.addZeroed(descriptorInfo.descriptorTypesCount);
            m_pipelineState->dsWriteContainer.descriptorImageInfo.addZeroed(descriptorInfo.imageInfosCount);
            m_pipelineState->dsWriteContainer.descriptorBufferInfo.addZeroed(descriptorInfo.bufferInfosCount);
            m_pipelineState->dsWriteContainer.descriptorTexelBufferView.addZeroed(descriptorInfo.texelBufferViewsCount);

            TK_ASSERT(descriptorInfo.descriptorTypesCount < 255);
            m_pipelineState->dsWriteContainer.bindingToDynamicOffset.addDefault(descriptorInfo.descriptorTypesCount);
            m_pipelineState->dsWriteContainer.bindingToDynamicOffset.setAll(255);

            VkWriteDescriptorSet *currentDescriptorWrite = m_pipelineState->dsWriteContainer.descriptorWrites.get();
            VkDescriptorImageInfo *currentImageInfo = m_pipelineState->dsWriteContainer.descriptorImageInfo.get();
            VkDescriptorBufferInfo *currentBufferInfo = m_pipelineState->dsWriteContainer.descriptorBufferInfo.get();
            VkBufferView *currentTexelBufferView = m_pipelineState->dsWriteContainer.descriptorTexelBufferView.get();
            u8 *currentBindingToDynamicOffsetMap = m_pipelineState->dsWriteContainer.bindingToDynamicOffset.get();

            dynamicOffsetsCount = m_pipelineState->dsWriter.setupDescriptorWrites(descriptorInfo, currentDescriptorWrite, currentImageInfo, currentBufferInfo, currentTexelBufferView, currentBindingToDynamicOffsetMap);
        }

        m_pipelineState->dynamicOffsets.addZeroed(dynamicOffsetsCount);
        m_pipelineState->dsWriter.dynamicOffsets = m_pipelineState->dynamicOffsets.get();

        return m_pipelineState;
    }

    VulkanComputePipelineState::VulkanComputePipelineState(VulkanDevice *device, VkPipeline pipeline, VulkanPipelineLayout *layout)
        : m_device(device), m_handle(pipeline), m_layout(layout)
    {
    }

    VulkanComputePipelineState::~VulkanComputePipelineState()
    {
        dsWriteContainer.release();
        if (currentTypedDescriptorPoolSet)
        {
            currentTypedDescriptorPoolSet->getOwner()->refs--;
            currentTypedDescriptorPoolSet = nullptr;
        }
        descriptorSetsLayout = nullptr;
        descriptorSetHandles.resize(0);
        dynamicOffsets.resize(0);
        m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kPipeline, m_handle);
        m_layout = nullptr;
    }

    VulkanPipelineState::VulkanPipelineState(VulkanDevice *device)
        : VulkanResource<GfxPipelineState>(device, TkStringView::Empty), m_layout(nullptr)
    {
    }

    VulkanPipelineLayout *VulkanPipelineState::getLayout()
    {
        if (m_layout)
            return m_layout;

        VulkanDescriptorSetLayoutInfo descriptorSetLayoutInfo;
#define INIT_SHADER_STAGE(ss, bit)                          \
    if (descriptorInfoPerStage[VulkanDescriptorSet::k##ss]) \
    descriptorSetLayoutInfo.addBindingsForStage(bit, VulkanDescriptorSet::k##ss, descriptorInfoPerStage[VulkanDescriptorSet::k##ss])
        INIT_SHADER_STAGE(Vertex, VK_SHADER_STAGE_VERTEX_BIT);
#if GFX_ALLOW_TESSELLATION_SHADERS
        INIT_SHADER_STAGE(Hull, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        INIT_SHADER_STAGE(Domain, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        INIT_SHADER_STAGE(Geometry, VK_SHADER_STAGE_GEOMETRY_BIT);
#endif
        INIT_SHADER_STAGE(Pixel, VK_SHADER_STAGE_FRAGMENT_BIT);
#undef INIT_SHADER_STAGE
        m_layout = m_device->getOrCreateLayout(descriptorSetLayoutInfo);
        TK_ASSERT(m_layout);
        descriptorSetsLayout = &m_layout->descriptorSetLayout;
        descriptorSetHandles.addZeroed(descriptorSetsLayout->handles.count());

        return m_layout;
    }

    VkPipeline VulkanPipelineState::getState(VulkanRenderPass *renderPass, VulkanVertexLayout *vertexLayout)
    {
        TK_ASSERT(renderPass);

        // Try reuse cached version
        VkPipeline pipeline = VK_NULL_HANDLE;
        const TkPair<VulkanRenderPass *, VulkanVertexLayout *> key(renderPass, vertexLayout);
        if (m_pipelines.containsKey(key))
        {
            pipeline = m_pipelines.at(key);
#if TK_DEBUG
            // Verify
            TkPair<VulkanRenderPass *, VulkanVertexLayout *> refKey(nullptr, nullptr);
            m_pipelines.keyOf(pipeline, &refKey);
            TK_ASSERT(refKey == key);
#endif
            return pipeline;
        }
        PROFILE_NAMED("Create Pipeline");

        // Bind vertex input
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
        VkVertexInputBindingDescription vertexInputBindings[GFX_MAX_VB_BINDED + 1];
        VkVertexInputAttributeDescription vertexInputAttributes[GFX_MAX_VS_ELEMENTS];
        m_desc.pVertexInputState = nullptr;
        if (!vertexLayout)
            vertexLayout = vertexBufferLayout; // Fallback to shader-specified layout (if using old APIs)
        if (vertexLayout)
        {
            // Vertex bindings based on vertex buffers assigned
            for (i32 i = 0; i < GFX_MAX_VB_BINDED; i++)
            {
                VkVertexInputBindingDescription &binding = vertexInputBindings[i];
                binding.binding = i;
                binding.stride = 0;
                binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            }
            u32 bindingsCount = 0;
            for (i32 i = 0; i < vertexLayout->getElements().count(); i++)
            {
                const VertexElement &src = vertexLayout->getElements().get()[i];
                const i32 size = GfxPixelFormat::sizeInBytes(src.format);
                TK_ASSERT(src.slot < GFX_MAX_VB_BINDED);
                VkVertexInputBindingDescription &binding = vertexInputBindings[src.slot];
                binding.binding = src.slot;
                binding.stride = TkMath::max(binding.stride, (u32)(src.offset + size));
                binding.inputRate = src.perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
                bindingsCount = TkMath::max(bindingsCount, (u32)src.slot + 1);
            }

            // Vertex elements (including any merged elements from reference layout from shader reflection)
            u32 missingSlotBinding = bindingsCount;
            i32 missingSlotOverride = GFX_MAX_VB_BINDED; // Use additional slot with empty VB
            vertexLayout = (VulkanVertexLayout *)GfxVertexLayout::merge(vertexLayout, vertexInputLayout, true, true, missingSlotOverride);
            for (i32 i = 0; i < vertexLayout->getElements().count(); i++)
            {
                const VertexElement &src = vertexLayout->getElements().get()[i];
                VkVertexInputAttributeDescription &attribute = vertexInputAttributes[i];
                attribute.location = i;
                if (vertexInputLayout)
                {
                    // Sync locations with vertex shader inputs to ensure that shader will load correct attributes
                    const auto &vertexInputLayoutElements = vertexInputLayout->getElements();
                    for (i32 j = 0; j < vertexInputLayoutElements.count(); j++)
                    {
                        if (vertexInputLayoutElements.get()[j].type == src.type)
                        {
                            attribute.location = j;
                            break;
                        }
                    }
                }
                if (src.slot == missingSlotOverride)
                {
                    // Element is missing and uses special empty VB
                    vertexInputBindings[missingSlotBinding] = {GFX_MAX_VB_BINDED, sizeof(byte[4]), VK_VERTEX_INPUT_RATE_INSTANCE};
                    bindingsCount = missingSlotBinding + 1;
                }
                attribute.binding = src.slot;
                attribute.format = VkUtils::toVulkanFormat(src.format);
                attribute.offset = src.offset;
            }

            VkUtils::zeroStruct(vertexInputCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
            vertexInputCreateInfo.vertexBindingDescriptionCount = bindingsCount;
            vertexInputCreateInfo.pVertexBindingDescriptions = vertexInputBindings;
            vertexInputCreateInfo.vertexAttributeDescriptionCount = vertexLayout->getElements().count();
            vertexInputCreateInfo.pVertexAttributeDescriptions = vertexInputAttributes;
            m_desc.pVertexInputState = &vertexInputCreateInfo;
        }

        // Update description to match the pipeline
        m_descColorBlend.attachmentCount = renderPass->layout.rtsCount;
        m_descMultisample.rasterizationSamples = (VkSampleCountFlagBits)renderPass->layout.msaa;
        m_desc.renderPass = renderPass->handle;

        // Ensure to have valid layout set
        if (m_desc.layout == VK_NULL_HANDLE)
            m_desc.layout = getLayout()->handle;

        // Create object
        auto depthWrite = m_descDepthStencil.depthWriteEnable;
        m_descDepthStencil.depthWriteEnable &= renderPass->canDepthWrite ? 1 : 0;
        const VkResult result = vkCreateGraphicsPipelines(m_device->device, m_device->pipelineCache, 1, &m_desc, nullptr, &pipeline);
        m_descDepthStencil.depthWriteEnable = depthWrite;
        LOG_VK_RESULT(result);
        if (result != VK_SUCCESS)
        {
#if TK_DEBUG
            const TkString vsName = debugDesc.vs ? debugDesc.vs->getName() : TkString::Empty;
            const TkString psName = debugDesc.ps ? debugDesc.ps->getName() : TkString::Empty;
            LogError("vkCreateGraphicsPipelines failed for VS=%s, PS=%s", vsName.getText(), psName.getText());
#endif
            return VK_NULL_HANDLE;
        }

        // Cache it
        m_pipelines.add(key, pipeline);

        return pipeline;
    }

    void VulkanPipelineState::onReleaseData()
    {
        dsWriteContainer.release();
        if (currentTypedDescriptorPoolSet)
        {
            currentTypedDescriptorPoolSet->getOwner()->refs--;
            currentTypedDescriptorPoolSet = nullptr;
        }
        descriptorSetsLayout = nullptr;
        descriptorSetHandles.resize(0);
        dynamicOffsets.resize(0);
        for (auto i = m_pipelines.Begin(); i.isNotEnd(); ++i)
        {
            m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kPipeline, i->value);
        }
        m_layout = nullptr;
        m_pipelines.clear();
    }

    bool VulkanPipelineState::isValid() const
    {
        return m_memoryUsage != 0;
    }

    bool VulkanPipelineState::init(const Description &desc)
    {
        TK_ASSERT(!isValid());

        // Reset description
        VkUtils::zeroStruct(m_desc, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);

        // Stages
        usedStagesMask = 0;
        hasDescriptorsPerStageMask = 0;
        TkMemory::memoryClear(shaderBindingsPerStage, sizeof(shaderBindingsPerStage));
        TkMemory::memoryClear(descriptorInfoPerStage, sizeof(descriptorInfoPerStage));
#define INIT_SHADER_STAGE(type, type1, ss, bit)                                                                    \
    if (desc.type)                                                                                           \
    {                                                                                                        \
        i32 stageIndex = (i32)VulkanDescriptorSet::k##ss;                                                     \
        usedStagesMask |= (1 << stageIndex);                                                                 \
        auto bindings = &desc.type->getBindings();                                                           \
        if (bindings->usedCBsMask + bindings->usedSRsMask + bindings->usedUAsMask)                           \
            hasDescriptorsPerStageMask |= (1 << stageIndex);                                                 \
        shaderBindingsPerStage[stageIndex] = bindings;                                                       \
        descriptorInfoPerStage[stageIndex] = &((VulkanShaderProgram##type1 *)desc.type)->descriptorInfo; \
        auto &stage = m_shaderStages[m_desc.stageCount++];                                                    \
        VkUtils::zeroStruct(stage, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);                     \
        stage.stage = bit;                                                                                   \
        stage.module = (VkShaderModule)desc.type->getBufferHandle();                                         \
        stage.pName = desc.type->getName().get();                                                            \
    }
        INIT_SHADER_STAGE(vs, VS, Vertex, VK_SHADER_STAGE_VERTEX_BIT);
#if GFX_ALLOW_TESSELLATION_SHADERS
        INIT_SHADER_STAGE(hs, HS, Hull, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        INIT_SHADER_STAGE(ds, DS, Domain, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        INIT_SHADER_STAGE(gs, GS, Geometry, VK_SHADER_STAGE_GEOMETRY_BIT);
#endif
        INIT_SHADER_STAGE(ps, PS, Pixel, VK_SHADER_STAGE_FRAGMENT_BIT);
#undef INIT_SHADER_STAGE
        m_desc.pStages = m_shaderStages;

        // Input Assembly
        if (desc.vs)
        {
            vertexInputLayout = (VulkanVertexLayout *)desc.vs->inputLayout;
            vertexBufferLayout = (VulkanVertexLayout *)desc.vs->layout;
        }
        VkUtils::zeroStruct(m_descInputAssembly, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
        ;
        switch (desc.primitiveTopology)
        {
        case GfxPrimitiveTopologyType::kPoint:
            m_descInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case GfxPrimitiveTopologyType::kLine:
            m_descInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case GfxPrimitiveTopologyType::kTriangle:
            m_descInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        default: break;
        }
#if GFX_ALLOW_TESSELLATION_SHADERS
        if (desc.hs)
            m_descInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
#endif
        m_desc.pInputAssemblyState = &m_descInputAssembly;

#if GFX_ALLOW_TESSELLATION_SHADERS
        // Tessellation
        if (desc.hs)
        {
            VkUtils::zeroStruct(m_descTessellation, VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO);
            m_descTessellation.patchControlPoints = desc.hs->getControlPointsCount();
            m_desc.pTessellationState = &m_descTessellation;
        }
#endif

        // Viewport
        VkUtils::zeroStruct(m_descViewport, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
        m_descViewport.viewportCount = 1;
        m_descViewport.scissorCount = 1;
        m_desc.pViewportState = &m_descViewport;

        // Dynamic
        VkUtils::zeroStruct(m_descDynamic, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
        m_descDynamic.pDynamicStates = m_dynamicStates;
        m_dynamicStates[m_descDynamic.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
        m_dynamicStates[m_descDynamic.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
        m_dynamicStates[m_descDynamic.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
#define IsBlendUsingBlendFactor(blend) blend == GfxBlendingMode::Blend::kBlendFactor || blend == GfxBlendingMode::Blend::kBlendInvFactor
        if (desc.blendMode.blendEnable && (IsBlendUsingBlendFactor(desc.blendMode.srcBlend) || IsBlendUsingBlendFactor(desc.blendMode.srcBlendAlpha) ||
                                           IsBlendUsingBlendFactor(desc.blendMode.destBlend) || IsBlendUsingBlendFactor(desc.blendMode.destBlendAlpha)))
            m_dynamicStates[m_descDynamic.dynamicStateCount++] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
#undef IsBlendUsingBlendFactor
        //TODO static_assert(lengthOf(m_dynamicStates) <= 4, "Invalid dynamic states array.");
        m_desc.pDynamicState = &m_descDynamic;

        // Multisample
        VkUtils::zeroStruct(m_descMultisample, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
        m_descMultisample.minSampleShading = 1.0f;
        m_descMultisample.alphaToCoverageEnable = desc.blendMode.alphaToCoverageEnable;
        m_desc.pMultisampleState = &m_descMultisample;

        // Depth Stencil
        VkUtils::zeroStruct(m_descDepthStencil, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
        m_descDepthStencil.depthTestEnable = desc.depthEnable;
        m_descDepthStencil.depthWriteEnable = desc.depthWriteEnable;
        m_descDepthStencil.depthCompareOp = VkUtils::toVulkanCompareOp(desc.depthFunc);
        m_descDepthStencil.stencilTestEnable = desc.stencilEnable;
        m_descDepthStencil.front.compareMask = desc.stencilReadMask;
        m_descDepthStencil.front.writeMask = desc.stencilWriteMask;
        m_descDepthStencil.front.compareOp = VkUtils::toVulkanCompareOp(desc.stencilFunc);
        m_descDepthStencil.front.failOp = toVulkanStencilOp(desc.stencilFailOp);
        m_descDepthStencil.front.depthFailOp = toVulkanStencilOp(desc.stencilDepthFailOp);
        m_descDepthStencil.front.passOp = toVulkanStencilOp(desc.stencilPassOp);
        m_descDepthStencil.back = m_descDepthStencil.front;
        m_desc.pDepthStencilState = &m_descDepthStencil;
        depthReadEnable = desc.depthEnable && desc.depthFunc != GfxComparisonFunc::kAlways;
        depthWriteEnable = m_descDepthStencil.depthWriteEnable;
        stencilReadEnable = desc.stencilEnable && desc.stencilReadMask != 0 && desc.stencilFunc != GfxComparisonFunc::kAlways;
        stencilWriteEnable = desc.stencilEnable && desc.stencilWriteMask != 0;

        // Rasterization
        VkUtils::zeroStruct(m_descRasterization, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
        m_descRasterization.polygonMode = desc.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
        switch (desc.cullMode)
        {
        case GfxCullMode::kNormal:
            m_descRasterization.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case GfxCullMode::kInverted:
            m_descRasterization.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        case GfxCullMode::kTwoSided:
            m_descRasterization.cullMode = VK_CULL_MODE_NONE;
            break;
        }
        m_descRasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
        m_descRasterization.depthClampEnable = !desc.depthClipEnable && m_device->limits.hasDepthClip;
        m_descRasterization.lineWidth = 1.0f;
        m_desc.pRasterizationState = &m_descRasterization;

        // Color Blend State
        blendEnable = desc.blendMode.blendEnable;
        VkUtils::zeroStruct(m_descColorBlend, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
        {
            auto &blend = m_descColorBlendAttachments[0];
            blend.blendEnable = desc.blendMode.blendEnable;
            blend.srcColorBlendFactor = toVulkanBlendFactor(desc.blendMode.srcBlend);
            blend.dstColorBlendFactor = toVulkanBlendFactor(desc.blendMode.destBlend);
            blend.colorBlendOp = VkUtils::toVulkanBlendOp(desc.blendMode.blendOp);
            blend.srcAlphaBlendFactor = toVulkanBlendFactor(desc.blendMode.srcBlendAlpha);
            blend.dstAlphaBlendFactor = toVulkanBlendFactor(desc.blendMode.destBlendAlpha);
            blend.alphaBlendOp = VkUtils::toVulkanBlendOp(desc.blendMode.blendOpAlpha);
            blend.colorWriteMask = (VkColorComponentFlags)desc.blendMode.renderTargetWriteMask;
        }
        for (i32 i = 1; i < GFX_MAX_RT_BINDED; i++)
            m_descColorBlendAttachments[i] = m_descColorBlendAttachments[i - 1];
        m_descColorBlend.pAttachments = m_descColorBlendAttachments;
        m_descColorBlend.blendConstants[0] = 1.0f;
        m_descColorBlend.blendConstants[1] = 1.0f;
        m_descColorBlend.blendConstants[2] = 1.0f;
        m_descColorBlend.blendConstants[3] = 1.0f;
        m_desc.pColorBlendState = &m_descColorBlend;

        TK_ASSERT(dsWriteContainer.descriptorWrites.isEmpty());
        for (i32 stage = 0; stage < VulkanDescriptorSet::kGraphicsStagesCount; stage++)
        {
            const auto descriptor = descriptorInfoPerStage[stage];
            if (descriptor == nullptr || descriptor->descriptorTypesCount == 0)
                continue;

            // TODO: merge into a single allocation for a whole PSO
            dsWriteContainer.descriptorWrites.addZeroed(descriptor->descriptorTypesCount);
            dsWriteContainer.descriptorImageInfo.addZeroed(descriptor->imageInfosCount);
            dsWriteContainer.descriptorBufferInfo.addZeroed(descriptor->bufferInfosCount);
            dsWriteContainer.descriptorTexelBufferView.addZeroed(descriptor->texelBufferViewsCount);

            TK_ASSERT(descriptor->descriptorTypesCount < 255);
            dsWriteContainer.bindingToDynamicOffset.addDefault(descriptor->descriptorTypesCount);
            dsWriteContainer.bindingToDynamicOffset.setAll(255);
        }

        VkWriteDescriptorSet *currentDescriptorWrite = dsWriteContainer.descriptorWrites.get();
        VkDescriptorImageInfo *currentImageInfo = dsWriteContainer.descriptorImageInfo.get();
        VkDescriptorBufferInfo *currentBufferInfo = dsWriteContainer.descriptorBufferInfo.get();
        VkBufferView *currentTexelBufferView = dsWriteContainer.descriptorTexelBufferView.get();
        byte *currentBindingToDynamicOffsetMap = dsWriteContainer.bindingToDynamicOffset.get();
        u32 dynamicOffsetsStart[VulkanDescriptorSet::kGraphicsStagesCount];
        u32 dynamicOffsetsCount = 0;
        for (i32 stage = 0; stage < VulkanDescriptorSet::kGraphicsStagesCount; stage++)
        {
            dynamicOffsetsStart[stage] = dynamicOffsetsCount;

            const auto descriptor = descriptorInfoPerStage[stage];
            if (descriptor == nullptr || descriptor->descriptorTypesCount == 0)
                continue;

            const u32 numdynamicOffsets = dsWriter[stage].setupDescriptorWrites(*descriptor, currentDescriptorWrite, currentImageInfo, currentBufferInfo, currentTexelBufferView, currentBindingToDynamicOffsetMap);
            dynamicOffsetsCount += numdynamicOffsets;

            currentDescriptorWrite += descriptor->descriptorTypesCount;
            currentImageInfo += descriptor->imageInfosCount;
            currentBufferInfo += descriptor->bufferInfosCount;
            currentTexelBufferView += descriptor->texelBufferViewsCount;
            currentBindingToDynamicOffsetMap += descriptor->descriptorTypesCount;
        }

        dynamicOffsets.addZeroed(dynamicOffsetsCount);
        for (i32 stage = 0; stage < VulkanDescriptorSet::kGraphicsStagesCount; stage++)
        {
            dsWriter[stage].dynamicOffsets = dynamicOffsetsStart[stage] + dynamicOffsets.get();
        }

        // Set non-zero memory usage
        m_memoryUsage = sizeof(VkGraphicsPipelineCreateInfo);

        return GfxPipelineState::init(desc);
    }

}

#endif
