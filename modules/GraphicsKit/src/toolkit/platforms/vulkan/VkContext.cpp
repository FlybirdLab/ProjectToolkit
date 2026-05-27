/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkContext.hpp"
#include "toolkit/graphics/GfxVertexLayout.hpp"
#include "toolkit/platforms/vulkan/VkSampler.hpp"
#include "toolkit/platforms/vulkan/VkShaderProgram.hpp"

#if GFX_API_VULKAN

#include "VkDevice.hpp"
#include "VkUtils.hpp"
#include "VkPipelineState.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/vulkan/VkBuffer.hpp"
#include "toolkit/platforms/vulkan/VkCmdBuffer.hpp"
#include "toolkit/platforms/vulkan/VkDescriptorSet.hpp"
#include "toolkit/platforms/vulkan/VkTexture.hpp"
#include "toolkit/graphics/GfxContext.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "toolkit/platforms/vulkan/VkShader.hpp"
#include "toolkit/math/Vector4.hpp"
#include "toolkit/types/Span.hpp"

namespace toolkit
{

    // Ensure to match the indirect commands arguments layout
    static_assert(sizeof(GfxDispatchIndirectArgs) == sizeof(VkDispatchIndirectCommand), "Wrong size of GfxDrawIndirectArgs.");
    static_assert(TK_OFFSET_OF(GfxDispatchIndirectArgs, threadGroupCountX) == TK_OFFSET_OF(VkDispatchIndirectCommand, x), "Wrong offset for GfxDrawIndirectArgs::ThreadGroupCountX");
    static_assert(TK_OFFSET_OF(GfxDispatchIndirectArgs, threadGroupCountY) == TK_OFFSET_OF(VkDispatchIndirectCommand, y), "Wrong offset for GfxDrawIndirectArgs::ThreadGroupCountY");
    static_assert(TK_OFFSET_OF(GfxDispatchIndirectArgs, threadGroupCountZ) == TK_OFFSET_OF(VkDispatchIndirectCommand, z), "Wrong offset for GfxDrawIndirectArgs::ThreadGroupCountZ");
    //
    static_assert(sizeof(GfxDrawIndirectArgs) == sizeof(VkDrawIndirectCommand), "Wrong size of GfxDrawIndirectArgs.");
    static_assert(TK_OFFSET_OF(GfxDrawIndirectArgs, verticesCount) == TK_OFFSET_OF(VkDrawIndirectCommand, vertexCount), "Wrong offset for GfxDrawIndirectArgs::VerticesCount");
    static_assert(TK_OFFSET_OF(GfxDrawIndirectArgs, instanceCount) == TK_OFFSET_OF(VkDrawIndirectCommand, instanceCount), "Wrong offset for GfxDrawIndirectArgs::InstanceCount");
    static_assert(TK_OFFSET_OF(GfxDrawIndirectArgs, startVertex) == TK_OFFSET_OF(VkDrawIndirectCommand, firstVertex), "Wrong offset for GfxDrawIndirectArgs::StartVertex");
    static_assert(TK_OFFSET_OF(GfxDrawIndirectArgs, startInstance) == TK_OFFSET_OF(VkDrawIndirectCommand, firstInstance), "Wrong offset for GfxDrawIndirectArgs::StartInstance");
    //
    static_assert(sizeof(GfxDrawIndexedIndirectArgs) == sizeof(VkDrawIndexedIndirectCommand), "Wrong size of GfxDrawIndexedIndirectArgs.");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, indicesCount) == TK_OFFSET_OF(VkDrawIndexedIndirectCommand, indexCount), "Wrong offset for GfxDrawIndexedIndirectArgs::IndicesCount");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, instanceCount) == TK_OFFSET_OF(VkDrawIndexedIndirectCommand, instanceCount), "Wrong offset for GfxDrawIndexedIndirectArgs::InstanceCount");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, startIndex) == TK_OFFSET_OF(VkDrawIndexedIndirectCommand, firstIndex), "Wrong offset for GfxDrawIndexedIndirectArgs::StartIndex");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, startVertex) == TK_OFFSET_OF(VkDrawIndexedIndirectCommand, vertexOffset), "Wrong offset for GfxDrawIndexedIndirectArgs::StartVertex");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, startInstance) == TK_OFFSET_OF(VkDrawIndexedIndirectCommand, firstInstance), "Wrong offset for GfxDrawIndexedIndirectArgs::StartInstance");

#if VK_ENABLE_BARRIERS_DEBUG

    const char *toString(VkImageLayout layout)
    {
        switch (layout)
        {
#define TO_STR(type) \
    case type:       \
        return #type
            TO_STR(VK_IMAGE_LAYOUT_UNDEFINED);
            TO_STR(VK_IMAGE_LAYOUT_GENERAL);
            TO_STR(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_PREINITIALIZED);
            TO_STR(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL);
            TO_STR(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            TO_STR(VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR);
            TO_STR(VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV);
            TO_STR(VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT);
            TO_STR(VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR);
            TO_STR(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR);
#undef TO_STR
        default:
            return "?";
        }
    }

#endif

    VulkanContext::VulkanContext(VulkanDevice *device, VulkanQueue *queue)
        : GfxContext(device), m_device(device), m_queue(queue), m_cmdBufferManager(TkMemory::newObject<VulkanCmdBufferManager>(TkMemory::getDefault(), device, this))
    {
        // Setup descriptor handles tables lookup cache
        m_handles[(i32)SpirvShaderResourceBindingType::kINVALID] = nullptr;
        m_handles[(i32)SpirvShaderResourceBindingType::kCB] = m_cbHandles;
        m_handles[(i32)SpirvShaderResourceBindingType::kSAMPLER] = nullptr;
        m_handles[(i32)SpirvShaderResourceBindingType::kSRV] = m_srHandles;
        m_handles[(i32)SpirvShaderResourceBindingType::kUAV] = m_uaHandles;
#if TK_DEBUG
        m_handlesSizes[(i32)SpirvShaderResourceBindingType::kINVALID] = 0;
        m_handlesSizes[(i32)SpirvShaderResourceBindingType::kCB] = GFX_MAX_CB_BINDED;
        m_handlesSizes[(i32)SpirvShaderResourceBindingType::kSAMPLER] = GFX_MAX_SAMPLER_BINDED;
        m_handlesSizes[(i32)SpirvShaderResourceBindingType::kSRV] = GFX_MAX_SR_BINDED;
        m_handlesSizes[(i32)SpirvShaderResourceBindingType::kUAV] = GFX_MAX_UA_BINDED;
#endif
    }

    void VulkanPipelineBarrier::execute(const VulkanCmdBuffer *cmdBuffer)
    {
        TK_ASSERT(cmdBuffer->isOutsideRenderPass());
        vkCmdPipelineBarrier(cmdBuffer->getHandle(), sourceStage, destStage, 0, 0, nullptr, bufferBarriers.count(), bufferBarriers.get(), imageBarriers.count(), imageBarriers.get());

        // Reset
        sourceStage = 0;
        destStage = 0;
        imageBarriers.clear();
        bufferBarriers.clear();
#if VK_ENABLE_BARRIERS_DEBUG
        ImageBarriersDebug.clear();
#endif
    }

    VulkanContext::~VulkanContext()
    {
        for (i32 i = 0; i < m_descriptorPools.count(); i++)
        {
            m_descriptorPools[i].clearDelete();
        }
        m_descriptorPools.clear();
        TkMemory::deletePtr(m_cmdBufferManager);
    }

    void VulkanContext::addImageBarrier(VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout, const VkImageSubresourceRange &subresourceRange, VulkanTextureView *handle)
    {
#if VK_ENABLE_BARRIERS_BATCHING
        // Auto-flush on overflow
        if (m_barriers.isFull())
        {
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();
            m_barriers.execute(cmdBuffer);
        }
#endif

        // Insert barrier
#if VK_ENABLE_BARRIERS_DEBUG
        m_barriers.imageBarriersDebug.add(handle);
#endif
        VkImageMemoryBarrier &imageBarrier = m_barriers.imageBarriers.addOne();
        VkUtils::zeroStruct(imageBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
        imageBarrier.image = image;
        imageBarrier.subresourceRange = subresourceRange;
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.oldLayout = srcLayout;
        imageBarrier.newLayout = dstLayout;
        m_barriers.sourceStage |= VkUtils::getImageBarrierFlags(srcLayout, imageBarrier.srcAccessMask);
        m_barriers.destStage |= VkUtils::getImageBarrierFlags(dstLayout, imageBarrier.dstAccessMask);
#if VK_ENABLE_BARRIERS_DEBUG
        LogWarn("Image Barrier: 0x%zx, %s -> %s for baseMipLevel: %u, baseArrayLayer: %u, levelCount: %u, layerCount: %u (%s)",
                (uintptr)image,
                toString(srcLayout),
                toString(dstLayout),
                subresourceRange.baseMipLevel,
                subresourceRange.baseArrayLayer,
                subresourceRange.levelCount,
                subresourceRange.layerCount,
                handle && handle->owner->asResource() ? *handle->owner->asResource()->toString() : *TkString::Empty);
#endif

#if !VK_ENABLE_BARRIERS_BATCHING
        // Auto-flush without batching
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();
        m_barriers.execute(cmdBuffer);
#endif
    }

    void VulkanContext::addImageBarrier(VulkanTextureView *handle, VkImageLayout dstLayout)
    {
        auto &state = handle->owner->state;
        const auto subresourceIndex = handle->subresourceIndex;
        if (subresourceIndex == -1)
        {
            const i32 mipLevels = state.getSubresourcesCount() / handle->owner->arraySlices;
            if (state.areAllSubresourcesSame())
            {
                const VkImageLayout srcLayout = state.getSubresourceState(-1);
                if (srcLayout != dstLayout)
                {
                    // Transition entire resource at once
                    VkImageSubresourceRange range;
                    range.aspectMask = handle->info.subresourceRange.aspectMask;
                    range.baseMipLevel = 0;
                    range.levelCount = mipLevels;
                    range.baseArrayLayer = 0;
                    range.layerCount = handle->owner->arraySlices;
                    addImageBarrier(handle->image, srcLayout, dstLayout, range, handle);
                    state.setResourceState(dstLayout);
                }
            }
            else
            {
                // Slow path to transition each subresource
                for (i32 i = 0; i < state.getSubresourcesCount(); i++)
                {
                    const VkImageLayout srcLayout = state.getSubresourceState(i);
                    if (srcLayout != dstLayout)
                    {
                        VkImageSubresourceRange range;
                        range.aspectMask = handle->info.subresourceRange.aspectMask;
                        range.baseMipLevel = i % mipLevels;
                        range.levelCount = 1;
                        range.baseArrayLayer = i / mipLevels;
                        range.layerCount = 1;
                        addImageBarrier(handle->image, srcLayout, dstLayout, range, handle);
                        state.setSubresourceState(i, dstLayout);
                    }
                }
            }
            TK_ASSERT(state.checkResourceState(dstLayout));
            state.setResourceState(dstLayout);
        }
        else
        {
            const VkImageLayout srcLayout = state.getSubresourceState(subresourceIndex);
            if (srcLayout != dstLayout)
            {
                // Transition a single subresource
                addImageBarrier(handle->image, srcLayout, dstLayout, handle->info.subresourceRange, handle);
                state.setSubresourceState(subresourceIndex, dstLayout);
            }
        }
    }

    void VulkanContext::addImageBarrier(VulkanTexture *texture, i32 mipSlice, i32 arraySlice, VkImageLayout dstLayout)
    {
        // Skip if no need to perform image layout transition
        const auto subresourceIndex = GfxUtils::calcSubresourceIndex(mipSlice, arraySlice, texture->mipLevels());
        auto &state = texture->state;
        const auto srcLayout = state.getSubresourceState(subresourceIndex);
        if (srcLayout == dstLayout)
            return;

        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = mipSlice;
        range.levelCount = 1;
        range.baseArrayLayer = arraySlice;
        range.layerCount = 1;
        addImageBarrier(texture->getHandle(), srcLayout, dstLayout, range, nullptr);
        state.setSubresourceState(subresourceIndex, dstLayout);
    }

    void VulkanContext::addImageBarrier(VulkanTexture *texture, VkImageLayout dstLayout)
    {
        // Check for fast path to transition the entire resource at once
        auto &state = texture->state;
        if (state.areAllSubresourcesSame())
        {
            // Skip if no need to perform image layout transition
            const auto srcLayout = state.getSubresourceState(0);
            if (srcLayout == dstLayout)
                return;

            VkImageSubresourceRange range;
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.levelCount = texture->mipLevels();
            range.baseArrayLayer = 0;
            range.layerCount = texture->arraySize();
            addImageBarrier(texture->getHandle(), srcLayout, dstLayout, range, nullptr);
            state.setResourceState(dstLayout);
        }
        else
        {
            // Slow path to transition each subresource
            for (i32 arraySlice = 0; arraySlice < texture->arraySize(); arraySlice++)
            {
                for (i32 mipSlice = 0; mipSlice < texture->mipLevels(); mipSlice++)
                {
                    addImageBarrier(texture, mipSlice, arraySlice, dstLayout);
                }
            }
        }
    }

    void VulkanContext::addBufferBarrier(VulkanBuffer *buffer, VkAccessFlags dstAccess)
    {
        // Skip if no need to perform buffer memory transition
        if ((buffer->access & dstAccess) == dstAccess)
            return;

#if VK_ENABLE_BARRIERS_BATCHING
        // Auto-flush on overflow
        if (m_barriers.isFull())
        {
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();
            m_barriers.execute(cmdBuffer);
        }
#endif

        // Insert barrier
        VkBufferMemoryBarrier &bufferBarrier = m_barriers.bufferBarriers.addOne();
        VkUtils::zeroStruct(bufferBarrier, VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER);
        bufferBarrier.buffer = buffer->getHandle();
        bufferBarrier.offset = 0;
        bufferBarrier.size = buffer->getSize();
        bufferBarrier.srcAccessMask = buffer->access;
        bufferBarrier.dstAccessMask = dstAccess;
        bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        m_barriers.sourceStage |= VkUtils::getBufferBarrierFlags(buffer->access);
        m_barriers.destStage |= VkUtils::getBufferBarrierFlags(dstAccess);
        buffer->access = dstAccess;

#if !VK_ENABLE_BARRIERS_BATCHING
        // Auto-flush without batching
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();
        m_barriers.execute(cmdBuffer);
#endif
    }

    void VulkanContext::flushBarriers()
    {
#if VK_ENABLE_BARRIERS_BATCHING
        if (m_barriers.hasBarrier())
        {
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();
            m_barriers.execute(cmdBuffer);
        }
#endif
    }

    VulkanDescriptorPool *VulkanContext::allocateDescriptorSets(const VkDescriptorSetAllocateInfo &descriptorSetAllocateInfo, const VulkanDescriptorSetLayout &layout, VkDescriptorSet *outSets)
    {
        VkResult result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
        VkDescriptorSetAllocateInfo allocateInfo = descriptorSetAllocateInfo;
        VulkanDescriptorPool *pool = nullptr;

        const u32 hash = VK_HASH_POOLS_WITH_LAYOUT_TYPES ? layout.setLayoutsHash : getHash(layout);
        DescriptorPoolArray *typedDescriptorPools = nullptr;
        bool hasResult = m_descriptorPools.containsKey(hash);
        if (hasResult)
        {
            typedDescriptorPools = &m_descriptorPools.at(hash);
        }

        if (typedDescriptorPools != nullptr)
        {
            pool = typedDescriptorPools->hasItems() ? typedDescriptorPools->last() : nullptr;
            if (pool && pool->canAllocate(layout))
            {
                allocateInfo.descriptorPool = pool->getHandle();
                result = vkAllocateDescriptorSets(m_device->device, &allocateInfo, outSets);
            }
        }
        else
        {
            typedDescriptorPools = &m_descriptorPools.add(hash, DescriptorPoolArray())->value;
        }

        if (result < VK_SUCCESS)
        {
            if (pool && pool->isEmpty())
            {
                LOG_VK_RESULT(result);
            }
            else
            {
                pool = TkMemory::newObject<VulkanDescriptorPool>(TkMemory::getDefault(), m_device, layout);
                typedDescriptorPools->add(pool);
                allocateInfo.descriptorPool = pool->getHandle();
                VALIDATE_VK_RESULT(vkAllocateDescriptorSets(m_device->device, &allocateInfo, outSets));
            }
        }

        return pool;
    }

    void VulkanContext::beginRenderPass()
    {
        auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        // Build render targets layout descriptor and framebuffer key.
        // CRITICAL: both structs are compared with TkMemory::memoryCompare on
        // the whole sizeof(), so we MUST zero them out before populating —
        // otherwise padding bytes / unused bitfield bits / un-touched array
        // slots all carry stack garbage and turn every begin into a cache
        // miss, leaking a fresh VulkanRenderPass + VulkanFramebuffer per
        // draw call.
        VulkanFramebuffer::Key framebufferKey;
        TkMemory::memoryClear(&framebufferKey, sizeof(framebufferKey));
        framebufferKey.attachmentCount = m_rtCount;
        VulkanRenderTargetLayout layout;
        TkMemory::memoryClear(&layout, sizeof(layout));
        layout.rtsCount = m_rtCount;
        layout.blendEnable = m_currentState && m_currentState->blendEnable;
        layout.depthFormat = m_rtDepth ? m_rtDepth->getFormat() : PixelFormat::kUnknown;
        for (i32 i = 0; i < GFX_MAX_RT_BINDED; i++)
        {
            auto handle = m_rtHandles[i];
            if (handle)
            {
                layout.rtvsFormats[i] = handle->getFormat();
                framebufferKey.attachments[i] = handle->getFramebufferView();
                addImageBarrier(handle, handle->layoutRTV);
            }
            else
            {
                layout.rtvsFormats[i] = PixelFormat::kUnknown;
                framebufferKey.attachments[i] = VK_NULL_HANDLE;
            }
        }
        VulkanTextureView *handle;
        if (m_rtDepth)
        {
            handle = m_rtDepth;
            layout.readDepth = true;
            layout.readStencil = GfxPixelFormat::hasStencil(handle->getFormat());
            layout.writeDepth = handle->layoutRTV == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || handle->layoutRTV == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL || handle->layoutRTV == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            layout.writeStencil = handle->layoutRTV == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || handle->layoutRTV == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL || handle->layoutRTV == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
            if (m_currentState && 0)
            {
                // TODO: use this but only if state doesn't change during whole render pass (eg. 1st draw call might not draw depth but 2nd might)
                layout.readDepth &= m_currentState->depthReadEnable;
                layout.readStencil &= m_currentState->stencilReadEnable;
                layout.writeDepth &= m_currentState->depthWriteEnable;
                layout.writeStencil &= m_currentState->stencilWriteEnable;
            }
            framebufferKey.attachmentCount++;
            framebufferKey.attachments[m_rtCount] = handle->getFramebufferView();
            addImageBarrier(handle, handle->layoutRTV);
        }
        else
        {
            handle = m_rtHandles[0];
            layout.readDepth = false;
            layout.writeDepth = false;
        }
        layout.msaa = handle->getMSAA();
        // extent is NOT part of the render pass cache key — keep it local
        VkExtent2D extent;
        extent.width = handle->extent.width;
        extent.height = handle->extent.height;
        layout.layers = handle->layers;

        // Get or create objects
        auto renderPass = m_device->getOrCreateRenderPass(layout);
        framebufferKey.renderPass = renderPass;
        auto framebuffer = m_device->getOrCreateFramebuffer(framebufferKey, extent, layout.layers);
        m_renderPass = renderPass;

        flushBarriers();

        // TODO: use clear values for render pass begin to improve performance
        cmdBuffer->beginRenderPass(renderPass, framebuffer, 0, nullptr);
    }

    void VulkanContext::endRenderPass()
    {
        auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        cmdBuffer->endRenderPass();
        m_renderPass = nullptr;

        // Place a barrier between RenderPasses, so that color / depth outputs can be read in subsequent passes
        // TODO: remove it in future and use proper barriers without whole pipeline stalls
        vkCmdPipelineBarrier(cmdBuffer->getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
    }

    void VulkanContext::updateDescriptorSets(const SpirvShaderDescriptorInfo &descriptorInfo, VulkanDescriptorSetWriter &dsWriter, bool &needsWrite)
    {
        for (u32 i = 0; i < descriptorInfo.descriptorTypesCount; i++)
        {
            const auto &descriptor = descriptorInfo.descriptorTypes[i];
            const i32 descriptorIndex = descriptor.binding;
            VulkanDescriptorOwnerResource **handles = m_handles[(i32)descriptor.bindingType];
            for (u32 index = 0; index < descriptor.count; index++)
            {
                const u32 slot = descriptor.slot + index;
                u32 slotsCount = 0;
                switch (descriptor.bindingType)
                {
                case SpirvShaderResourceBindingType::kCB:
                    slotsCount = GFX_MAX_CB_BINDED;
                    break;
                case SpirvShaderResourceBindingType::kSAMPLER:
                    slotsCount = GFX_MAX_SAMPLER_BINDED;
                    break;
                case SpirvShaderResourceBindingType::kSRV:
                    slotsCount = GFX_MAX_SR_BINDED;
                    break;
                case SpirvShaderResourceBindingType::kUAV:
                    slotsCount = GFX_MAX_UA_BINDED;
                    break;
                default:
                    LogWarn("Invalid Vulkan descriptor binding type %d.", static_cast<i32>(descriptor.bindingType));
                    continue;
                }
                if (slot >= slotsCount)
                {
                    LogWarn("Skipping Vulkan descriptor update with out-of-range slot %u (type=%d, max=%u).", slot, static_cast<i32>(descriptor.bindingType), slotsCount);
                    continue;
                }
                switch (descriptor.descriptorType)
                {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                {
                    const VkSampler handle = m_samplerHandles[slot];
                    if (handle == VK_NULL_HANDLE)
                    {
                        LogWarn("Skipping Vulkan sampler descriptor update with empty sampler at slot %u.", slot);
                        break;
                    }
                    needsWrite |= dsWriter.writeSampler(descriptorIndex, handle, index);
                    break;
                }
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                {
                    if (!handles)
                        break;
                    auto handle = (VulkanTextureView *)handles[slot];
                    if (!handle)
                    {
                        const auto dummy = m_device->helperResources.getDummyTexture(descriptor.resourceType);
                        switch (descriptor.resourceType)
                        {
                        case SpirvShaderResourceType::kTexture1D:
                        case SpirvShaderResourceType::kTexture2D:
                            handle = static_cast<VulkanTextureView *>(dummy->view(0));
                            break;
                        case SpirvShaderResourceType::kTexture3D:
                            handle = static_cast<VulkanTextureView *>(dummy->viewVolume());
                            break;
                        case SpirvShaderResourceType::kTextureCube:
                        case SpirvShaderResourceType::kTexture1DArray:
                        case SpirvShaderResourceType::kTexture2DArray:
                            handle = static_cast<VulkanTextureView *>(dummy->viewArray());
                            break;
                        default:
                            break;
                        }
                    }
                    if (!handle)
                        break;
                    VkImageView imageView;
                    VkImageLayout layout;
                    handle->descriptorAsImage(this, imageView, layout);
                    if (imageView == VK_NULL_HANDLE)
                        break;
                    needsWrite |= dsWriter.writeImage(descriptorIndex, imageView, layout, index);
                    break;
                }
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                {
                    if (!handles)
                        break;
                    auto handle = handles[slot];
                    if (!handle)
                    {
                        const auto dummy = m_device->helperResources.getDummyBuffer(descriptor.resourceFormat);
                        if (!dummy || !dummy->view())
                            break;
                        handle = (VulkanDescriptorOwnerResource *)dummy->view()->getNativePtr();
                    }
                    if (!handle)
                        break;
                    VkBufferView bufferView;
                    handle->descriptorAsUniformTexelBuffer(this, bufferView);
                    if (bufferView == VK_NULL_HANDLE)
                        break;
                    needsWrite |= dsWriter.writeUniformTexelBuffer(descriptorIndex, bufferView, index);
                    break;
                }
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                {
                    if (!handles)
                        break;
                    auto handle = handles[slot];
                    if (!handle)
                        break;
                    VkImageView imageView;
                    VkImageLayout layout;
                    handle->descriptorAsStorageImage(this, imageView, layout);
                    if (imageView == VK_NULL_HANDLE)
                        break;
                    needsWrite |= dsWriter.writeStorageImage(descriptorIndex, imageView, layout, index);
                    break;
                }
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                {
                    if (!handles)
                        break;
                    auto handle = handles[slot];
                    if (!handle)
                    {
                        if (descriptor.resourceFormat == PixelFormat::kUnknown)
                            break;
                        const auto dummy = m_device->helperResources.getDummyBuffer(descriptor.resourceFormat);
                        if (!dummy || !dummy->view())
                            break;
                        handle = (VulkanDescriptorOwnerResource *)dummy->view()->getNativePtr();
                    }
                    if (!handle)
                        break;
                    VkBuffer buffer;
                    VkDeviceSize offset, range;
                    handle->descriptorAsStorageBuffer(this, buffer, offset, range);
                    if (buffer == VK_NULL_HANDLE)
                        break;
                    needsWrite |= dsWriter.writeStorageBuffer(descriptorIndex, buffer, offset, range, index);
                    break;
                }
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                {
                    if (!handles)
                        break;
                    auto handle = handles[slot];
                    if (!handle)
                    {
                        const auto dummy = m_device->helperResources.getDummyBuffer(descriptor.resourceFormat);
                        if (!dummy || !dummy->view())
                            break;
                        handle = (VulkanDescriptorOwnerResource *)dummy->view()->getNativePtr();
                    }
                    if (!handle)
                        break;
                    VkBufferView bufferView;
                    handle->descriptorAsStorageTexelBuffer(this, bufferView);
                    if (bufferView == VK_NULL_HANDLE)
                        break;
                    needsWrite |= dsWriter.writeStorageTexelBuffer(descriptorIndex, bufferView, index);
                    break;
                }
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                {
                    if (!handles)
                        break;
                    auto handle = handles[slot];
                    VkBuffer buffer = VK_NULL_HANDLE;
                    VkDeviceSize offset = 0, range = 0;
                    u32 dynamicOffset = 0;
                    if (!handle)
                        handle = (VulkanConstantBuffer *)m_device->helperResources.getDummyConstantBuffer();
                    if (!handle)
                        break;
                    handle->descriptorAsDynamicUniformBuffer(this, buffer, offset, range, dynamicOffset);
                    if (buffer == VK_NULL_HANDLE)
                        break;
                    needsWrite |= dsWriter.writeDynamicUniformBuffer(descriptorIndex, buffer, offset, range, dynamicOffset, index);
                    break;
                }
                default:
                    // Unknown or invalid descriptor type
                    TK_ASSERT(false && "Unknown or invalid descriptor type");
                    break;
                }
            }
        }
    }

    void VulkanContext::updateDescriptorSets(VulkanComputePipelineState *pipelineState)
    {
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        const auto pipelineLayout = pipelineState->getLayout();
        TK_ASSERT(pipelineLayout);

        bool needsWrite = false;

        // No current descriptor pools set - acquire one and reset
        const bool newDescriptorPool = pipelineState->acquirePoolSet(cmdBuffer);
        needsWrite |= newDescriptorPool;

        // Update descriptors
        updateDescriptorSets(*pipelineState->descriptorInfo, pipelineState->dsWriter, needsWrite);

        // Allocate sets only when bindings or pool set actually changed
        if (needsWrite || pipelineState->descriptorSetInvalid)
        {
            if (!pipelineState->allocateDescriptorSets())
            {
                return;
            }
            const VkDescriptorSet descriptorSet = pipelineState->descriptorSetHandles[VulkanDescriptorSet::kCompute];
            pipelineState->dsWriter.setDescriptorSet(descriptorSet);

            vkUpdateDescriptorSets(m_device->device, pipelineState->dsWriteContainer.descriptorWrites.count(), pipelineState->dsWriteContainer.descriptorWrites.get(), 0, nullptr);
            pipelineState->descriptorSetInvalid = false;
        }
    }

    void VulkanContext::onDrawCall()
    {
        VulkanPipelineState *pipelineState = m_currentState;
        TK_ASSERT(pipelineState && pipelineState->isValid());
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        // End previous render pass if render targets layout was modified
        if (m_rtDirtyFlag && cmdBuffer->isInsideRenderPass())
            endRenderPass();

        if (pipelineState->hasDescriptorsPerStageMask)
        {
            // Get descriptor pools set
            bool needsWrite = pipelineState->acquirePoolSet(cmdBuffer);

            // Update descriptors for every used shader stage
            u32 remainingHasDescriptorsPerStageMask = pipelineState->hasDescriptorsPerStageMask;
            for (i32 stage = 0; stage < VulkanDescriptorSet::kGraphicsStagesCount && remainingHasDescriptorsPerStageMask; stage++)
            {
                if (remainingHasDescriptorsPerStageMask & 1)
                    updateDescriptorSets(*pipelineState->descriptorInfoPerStage[stage], pipelineState->dsWriter[stage], needsWrite);
                remainingHasDescriptorsPerStageMask >>= 1;
            }

            // Allocate sets only when bindings or pool set actually changed
            if (needsWrite || pipelineState->descriptorSetInvalid)
            {
                if (!pipelineState->currentTypedDescriptorPoolSet->allocateDescriptorSets(*pipelineState->descriptorSetsLayout, pipelineState->descriptorSetHandles.get()))
                    return;
                u32 remainingStagesMask = pipelineState->hasDescriptorsPerStageMask;
                u32 stage = 0;
                while (remainingStagesMask)
                {
                    if (remainingStagesMask & 1)
                        pipelineState->dsWriter[stage].setDescriptorSet(pipelineState->descriptorSetHandles[stage]);
                    remainingStagesMask >>= 1;
                    stage++;
                }

                vkUpdateDescriptorSets(m_device->device, pipelineState->dsWriteContainer.descriptorWrites.count(), pipelineState->dsWriteContainer.descriptorWrites.get(), 0, nullptr);
                pipelineState->descriptorSetInvalid = false;
            }
        }

#if GFX_ENABLE_ASSERTION_LOW_LAYERS
        // Check for missing vertex buffers layout
        VulkanVertexLayout *vertexLayout = m_vertexLayout ? m_vertexLayout : pipelineState->vertexBufferLayout;
        if (!vertexLayout && pipelineState && !pipelineState->vertexBufferLayout && (pipelineState->usedStagesMask & (1 << (i32)VulkanDescriptorSet::kVertex)) != 0 && !m_vertexLayout && m_vbCount)
        {
            LogError("Missing Vertex Layout (not assigned to GfxBuffer). Vertex Shader won't read valid data resulting incorrect visuals.");
        }
#endif

        // Start render pass if not during one
        if (cmdBuffer->isOutsideRenderPass())
            beginRenderPass();
        else if (m_barriers.hasBarrier())
        {
            // TODO: implement better image/buffer barriers - and remove this renderPass split to flush barriers
            endRenderPass();
            beginRenderPass();
        }

        // Bind pipeline
        if (m_psDirtyFlag && pipelineState && (m_rtDepth || m_rtCount))
        {
            m_psDirtyFlag = false;
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            const auto pipeline = pipelineState->getState(m_renderPass, m_vertexLayout);
            vkCmdBindPipeline(cmdBuffer->getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        }

        // Bind descriptors sets to the graphics pipeline
        if (pipelineState->hasDescriptorsPerStageMask)
        {
            vkCmdBindDescriptorSets(
                cmdBuffer->getHandle(),
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineState->getLayout()->handle,
                0,
                pipelineState->descriptorSetHandles.count(),
                pipelineState->descriptorSetHandles.get(),
                pipelineState->dynamicOffsets.count(),
                pipelineState->dynamicOffsets.get());
        }

        m_rtDirtyFlag = false;
#if VK_ENABLE_BARRIERS_DEBUG
        LogWarn("Draw");
#endif
    }

    void VulkanContext::frameBegin()
    {
        // Base
        GfxContext::frameBegin();

        // Setup
        m_psDirtyFlag = 0;
        m_rtDirtyFlag = 0;
        m_cbDirtyFlag = 0;
        m_rtCount = 0;
        m_vbCount = 0;
        m_stencilRef = 0;
        m_renderPass = nullptr;
        m_currentState = nullptr;
        m_vertexLayout = nullptr;
        m_rtDepth = nullptr;
        TkMemory::memoryClear(m_rtHandles, sizeof(m_rtHandles));
        TkMemory::memoryClear(m_cbHandles, sizeof(m_cbHandles));
        TkMemory::memoryClear(m_srHandles, sizeof(m_srHandles));
        TkMemory::memoryClear(m_uaHandles, sizeof(m_uaHandles));
        TkMemory::memoryCopy(m_samplerHandles, m_device->helperResources.getStaticSamplers(), sizeof(VkSampler) * GFX_STATIC_SAMPLERS_COUNT);
        TkMemory::memoryClear(m_samplerHandles + GFX_STATIC_SAMPLERS_COUNT, sizeof(m_samplerHandles) - sizeof(VkSampler) * GFX_STATIC_SAMPLERS_COUNT);

        // Init command buffer
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        vkCmdSetStencilReference(cmdBuffer->getHandle(), VK_STENCIL_FRONT_AND_BACK, m_stencilRef);
        VkBuffer buffers[1] = {m_device->helperResources.getDummyVertexBuffer()->getHandle()};
        VkDeviceSize offsets[1] = {};
        vkCmdBindVertexBuffers(cmdBuffer->getHandle(), GFX_MAX_VB_BINDED, 1, buffers, offsets);

#if VK_RESET_QUERY_POOLS
        // reset pending queries
        if (m_device->queriesToReset.hasItems())
        {
            for (auto query : m_device->queriesToReset)
                query->reset(cmdBuffer);
            m_device->queriesToReset.clear();
        }
#endif
    }

    void VulkanContext::frameEnd()
    {
        const auto cmdBuffer = getCmdBufferManager()->getActiveCmdBuffer();
        if (cmdBuffer && cmdBuffer->isInsideRenderPass())
        {
            endRenderPass();
        }

        // Execute any queued layout transitions that weren't already handled by the render pass
        flushBarriers();

        // Base
        GfxContext::frameEnd();
    }

#if GFX_ALLOW_PROFILE_EVENTS

    void VulkanContext::eventBegin(const char *name)
    {
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        cmdBuffer->beginEvent(name);
    }

    void VulkanContext::eventEnd()
    {
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        cmdBuffer->endEvent();
    }

#endif

    void *VulkanContext::getNativePtr() const
    {
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        return (void *)cmdBuffer->getHandle();
    }

    // bool VulkanContext::isDepthBufferBinded()
    // {
    //     return m_rtDepth != nullptr;
    // }

    void VulkanContext::clear(GfxTextureView *rt, const Color &color)
    {
        auto rtVulkan = static_cast<VulkanTextureView *>(rt);

        if (rtVulkan)
        {
            // TODO: detect if inside render pass and use ClearAttachments
            // TODO: delay clear for attachments before render pass to use render pass clear values for faster clearing

            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();

            addImageBarrier(rtVulkan, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            flushBarriers();

            vkCmdClearColorImage(cmdBuffer->getHandle(), rtVulkan->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (const VkClearColorValue *)color.raw, 1, &rtVulkan->info.subresourceRange);
        }
    }

    void VulkanContext::clearDepth(GfxTextureView *depthBuffer, float depthValue, u8 stencilValue)
    {
        const auto rtVulkan = static_cast<VulkanTextureView *>(depthBuffer);
        if (rtVulkan)
        {
            // TODO: detect if inside render pass and use ClearAttachments
            // TODO: delay clear for attachments before render pass to use render pass clear values for faster clearing

            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();

            addImageBarrier(rtVulkan, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            flushBarriers();

            VkClearDepthStencilValue clear;
            clear.depth = depthValue;
            clear.stencil = stencilValue;
            vkCmdClearDepthStencilImage(cmdBuffer->getHandle(), rtVulkan->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &rtVulkan->info.subresourceRange);
        }
    }

    void VulkanContext::clearUA(GfxBuffer *buf, const FVec4 &value)
    {
        const auto bufVulkan = static_cast<VulkanBuffer *>(buf);
        if (bufVulkan)
        {
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();

            // TODO: add support for other components if buffer has them
            u32 *data = (u32 *)&value;
            vkCmdFillBuffer(cmdBuffer->getHandle(), bufVulkan->getHandle(), 0, bufVulkan->getSize(), *data);
        }
    }

    void VulkanContext::clearUA(GfxBuffer *buf, const u32 value[4])
    {
        const auto bufVulkan = static_cast<VulkanBuffer *>(buf);
        if (bufVulkan)
        {
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();

            // TODO: add support for other components if buffer has them
            vkCmdFillBuffer(cmdBuffer->getHandle(), bufVulkan->getHandle(), 0, bufVulkan->getSize(), value[0]);
        }
    }

    void VulkanContext::clearUA(GfxTexture *texture, const u32 value[4])
    {
        const auto texVulkan = static_cast<VulkanTexture *>(texture);
        if (texVulkan)
        {
            auto rtVulkan = static_cast<VulkanTextureView *>(texVulkan->view(0));
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();

            addImageBarrier(rtVulkan, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            flushBarriers();

            vkCmdClearColorImage(cmdBuffer->getHandle(), rtVulkan->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (const VkClearColorValue *)value, 1, &rtVulkan->info.subresourceRange);
        }
    }

    void VulkanContext::clearUA(GfxTexture *texture, const FVec4 &value)
    {
        const auto texVulkan = static_cast<VulkanTexture *>(texture);
        if (texVulkan)
        {
            auto rtVulkan = ((VulkanTextureView *)(texVulkan->isVolume() ? texVulkan->viewVolume() : texVulkan->view(0)));
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            if (cmdBuffer->isInsideRenderPass())
                endRenderPass();

            addImageBarrier(rtVulkan, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            flushBarriers();

            vkCmdClearColorImage(cmdBuffer->getHandle(), rtVulkan->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (const VkClearColorValue *)value.raw, 1, &rtVulkan->info.subresourceRange);
        }
    }

    void VulkanContext::resetRenderTarget()
    {
        if (m_rtDepth || m_rtCount != 0)
        {
            m_rtDirtyFlag = true;
            m_psDirtyFlag = true;
            m_rtCount = 0;
            m_rtDepth = nullptr;
            TkMemory::memoryClear(m_rtHandles, sizeof(m_rtHandles));

            const auto cmdBuffer = m_cmdBufferManager->getActiveCmdBuffer();
            if (cmdBuffer && cmdBuffer->isInsideRenderPass())
                endRenderPass();
        }
    }

    void VulkanContext::setRenderTarget(GfxTextureView *rt)
    {
        const auto rtVulkan = static_cast<VulkanTextureView *>(rt);

        if (m_rtDepth != nullptr || m_rtCount != 1 || m_rtHandles[0] != rtVulkan)
        {
            m_rtDirtyFlag = true;
            m_psDirtyFlag = true;
            m_rtCount = 1;
            m_rtDepth = nullptr;
            m_rtHandles[0] = rtVulkan;
        }
    }

    void VulkanContext::setRenderTarget(GfxTextureView *depthBuffer, GfxTextureView *rt)
    {
        const auto rtVulkan = static_cast<VulkanTextureView *>(rt);
        const auto depthBufferVulkan = static_cast<VulkanTextureView *>(depthBuffer);
        const auto rtCount = rtVulkan ? 1 : 0;

        if (m_rtDepth != depthBufferVulkan || m_rtCount != rtCount || m_rtHandles[0] != rtVulkan)
        {
            m_rtDirtyFlag = true;
            m_psDirtyFlag = true;
            m_rtCount = rtCount;
            m_rtDepth = depthBufferVulkan;
            m_rtHandles[0] = rtVulkan;
        }
    }

    void VulkanContext::setRenderTarget(GfxTextureView *depthBuffer, const TkSpan<GfxTextureView *> &rts)
    {
        TK_ASSERT(TkMath::isInRange(rts.length(), 1, GFX_MAX_RT_BINDED));

        const auto depthBufferVulkan = static_cast<VulkanTextureView *>(depthBuffer);

        VulkanTextureView *rtvs[GFX_MAX_RT_BINDED];
        for (i32 i = 0; i < rts.length(); i++)
        {
            rtvs[i] = static_cast<VulkanTextureView *>(rts[i]);
        }
        const i32 rtvsSize = sizeof(VulkanTextureView *) * rts.length();

        if (m_rtDepth != depthBufferVulkan || m_rtCount != rts.length() || TkMemory::memoryCompare(m_rtHandles, rtvs, rtvsSize) != 0)
        {
            m_rtDirtyFlag = true;
            m_psDirtyFlag = true;
            m_rtCount = rts.length();
            m_rtDepth = depthBufferVulkan;
            TkMemory::memoryCopy(m_rtHandles, rtvs, rtvsSize);
        }
    }

    void VulkanContext::setBlendFactor(const FVec4 &value)
    {
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        vkCmdSetBlendConstants(cmdBuffer->getHandle(), value.raw);
    }

    void VulkanContext::setStencilRef(u32 value)
    {
        if (m_stencilRef != value)
        {
            m_stencilRef = value;
            const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
            vkCmdSetStencilReference(cmdBuffer->getHandle(), VK_STENCIL_FRONT_AND_BACK, m_stencilRef);
        }
    }

    void VulkanContext::resetSR()
    {
        TkMemory::memoryClear(m_srHandles, sizeof(m_srHandles));
    }

    void VulkanContext::resetUA()
    {
        TkMemory::memoryClear(m_uaHandles, sizeof(m_uaHandles));
    }

    void VulkanContext::resetCB()
    {
        m_cbDirtyFlag = false;
        TkMemory::memoryClear(m_cbHandles, sizeof(m_cbHandles));
    }

    void VulkanContext::bindCB(i32 slot, GfxConstantBuffer *cb)
    {
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_CB_BINDED);

        const auto cbVulkan = static_cast<VulkanConstantBuffer *>(cb);

        if (m_cbHandles[slot] != cbVulkan)
        {
            m_cbDirtyFlag = true;
            m_cbHandles[slot] = cbVulkan;
        }
    }

    void VulkanContext::bindSR(i32 slot, GfxResourceView *view)
    {
#if TK_DEBUG
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_SR_BINDED);
        if (view && ((VulkanDescriptorOwnerResource *)view->getNativePtr())->hasSRV() == false)
            logInvalidResourceUsage(slot, view, InvalidBindPoint::SRV);
#endif
        const auto handle = view ? (VulkanDescriptorOwnerResource *)view->getNativePtr() : nullptr;
        if (m_srHandles[slot] != handle)
        {
            m_srHandles[slot] = handle;
            if (view)
                *view->lastRenderTime = m_lastRenderTime;
        }
    }

    void VulkanContext::bindUA(i32 slot, GfxResourceView *view)
    {
#if TK_DEBUG
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_UA_BINDED);
        if (view && ((VulkanDescriptorOwnerResource *)view->getNativePtr())->hasUAV() == false)
            logInvalidResourceUsage(slot, view, InvalidBindPoint::UAV);
#endif
        const auto handle = view ? (VulkanDescriptorOwnerResource *)view->getNativePtr() : nullptr;
        if (m_uaHandles[slot] != handle)
        {
            m_uaHandles[slot] = handle;
            if (view)
                *view->lastRenderTime = m_lastRenderTime;
        }
    }

    void VulkanContext::bindVB(const TkSpan<GfxBuffer *> &vertexBuffers, const u32 *vertexBuffersOffsets, GfxVertexLayout *vertexLayout)
    {
        m_vbCount = vertexBuffers.length();
        if (!vertexLayout)
            vertexLayout = GfxVertexLayout::get(vertexBuffers);
        auto *vkVertexLayout = (VulkanVertexLayout *)vertexLayout;
        if (m_vertexLayout != vkVertexLayout)
        {
            m_vertexLayout = vkVertexLayout;
            m_psDirtyFlag = true;
        }
        if (vertexBuffers.length() == 0)
            return;
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        VkBuffer buffers[GFX_MAX_VB_BINDED];
        VkDeviceSize offsets[GFX_MAX_VB_BINDED];
        for (i32 i = 0; i < vertexBuffers.length(); i++)
        {
            auto vbVulkan = static_cast<VulkanBuffer *>(vertexBuffers[i]);
            if (!vbVulkan)
                vbVulkan = m_device->helperResources.getDummyVertexBuffer();
            buffers[i] = vbVulkan->getHandle();
            offsets[i] = vertexBuffersOffsets ? vertexBuffersOffsets[i] : 0;
        }
        vkCmdBindVertexBuffers(cmdBuffer->getHandle(), 0, vertexBuffers.length(), buffers, offsets);
    }

    void VulkanContext::bindIB(GfxBuffer *indexBuffer)
    {
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        const auto ibVulkan = static_cast<VulkanBuffer *>(indexBuffer)->getHandle();
        vkCmdBindIndexBuffer(cmdBuffer->getHandle(), ibVulkan, 0, indexBuffer->getFormat() == PixelFormat::kR32_UInt ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
    }

    void VulkanContext::bindSampler(i32 slot, GfxSampler *sampler)
    {
        if (slot < GFX_STATIC_SAMPLERS_COUNT || slot >= GFX_MAX_SAMPLER_BINDED)
        {
            LogWarn("Ignoring Vulkan sampler bind with out-of-range slot %d.", slot);
            return;
        }
        const auto handle = sampler ? ((VulkanSampler *)sampler)->sampler : VK_NULL_HANDLE;
        m_samplerHandles[slot] = handle;
    }

    void VulkanContext::updateCB(GfxConstantBuffer *cb, const void *data)
    {
        TK_ASSERT(data && cb);
        auto cbVulkan = static_cast<VulkanConstantBuffer *>(cb);
        const u32 size = cbVulkan->getSize();
        if (size == 0)
            return;
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        // Allocate bytes for the buffer
        const auto allocation = m_device->uniformBufferUploader->allocate(size, 0, this);

        // Copy data
        TkMemory::memoryCopy(allocation.address, data, allocation.size);

        // Cache the allocation to update the descriptor
        cbVulkan->allocation = allocation;

        // Mark CB slot as dirty if this CB is binded to the pipeline
        for (i32 i = 0; i < lengthOf(m_cbHandles); i++)
        {
            if (m_cbHandles[i] == cbVulkan)
            {
                m_cbDirtyFlag = true;
                break;
            }
        }
    }

    void VulkanContext::dispatch(GfxShaderProgramCS *shader, u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ)
    {
        TK_ASSERT(shader);
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        auto shaderVulkan = (VulkanShaderProgramCS *)shader;

        // End any render pass
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        auto pipelineState = shaderVulkan->getOrCreateState();
        updateDescriptorSets(pipelineState);
        flushBarriers();

        // Bind pipeline
        vkCmdBindPipeline(cmdBuffer->getHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipelineState->getHandle());

        // Bind descriptors sets to the compute pipeline
        pipelineState->bind(cmdBuffer);

        // Dispatch
        vkCmdDispatch(cmdBuffer->getHandle(), threadGroupCountX, threadGroupCountY, threadGroupCountZ);

        // Place a barrier between dispatches, so that UAVs can be read+write in subsequent passes
        // TODO: optimize it by moving inputs/outputs into higher-layer so eg. Global SDF can manually optimize it
        vkCmdPipelineBarrier(cmdBuffer->getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

#if VK_ENABLE_BARRIERS_DEBUG
        LOG(Warning, "Dispatch");
#endif
    }

    void VulkanContext::dispatchIndirect(GfxShaderProgramCS *shader, GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        TK_ASSERT(shader && bufferForArgs);
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        auto shaderVulkan = (VulkanShaderProgramCS *)shader;
        const auto bufferForArgsVulkan = (VulkanBuffer *)bufferForArgs;

        // End any render pass
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        auto pipelineState = shaderVulkan->getOrCreateState();
        updateDescriptorSets(pipelineState);
        addBufferBarrier(bufferForArgsVulkan, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
        flushBarriers();

        // Bind pipeline
        vkCmdBindPipeline(cmdBuffer->getHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipelineState->getHandle());

        // Bind descriptors sets to the compute pipeline
        pipelineState->bind(cmdBuffer);

        // Dispatch
        vkCmdDispatchIndirect(cmdBuffer->getHandle(), bufferForArgsVulkan->getHandle(), offsetForArgs);

        // Place a barrier between dispatches, so that UAVs can be read+write in subsequent passes
        // TODO: optimize it by moving inputs/outputs into higher-layer so eg. Global SDF can manually optimize it
        vkCmdPipelineBarrier(cmdBuffer->getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

#if VK_ENABLE_BARRIERS_DEBUG
        LOG(Warning, "DispatchIndirect");
#endif
    }

    void VulkanContext::resolveMultisample(GfxTexture *sourceMultisampleTexture, GfxTexture *destTexture, i32 sourceSubResource, i32 destSubResource, PixelFormat format)
    {
        TK_ASSERT(sourceMultisampleTexture && sourceMultisampleTexture->isMultiSample());
        TK_ASSERT(destTexture && !destTexture->isMultiSample());

        // TODO: use render pass to resolve attachments

        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        const auto dstResourceVulkan = dynamic_cast<VulkanTexture *>(destTexture);
        const auto srcResourceVulkan = dynamic_cast<VulkanTexture *>(sourceMultisampleTexture);

        const i32 srcMipIndex = sourceSubResource % dstResourceVulkan->mipLevels();
        const i32 srcArrayIndex = sourceSubResource / dstResourceVulkan->mipLevels();
        const i32 dstMipIndex = destSubResource % dstResourceVulkan->mipLevels();
        const i32 dstArrayIndex = destSubResource / dstResourceVulkan->mipLevels();

        i32 width, height, depth;
        sourceMultisampleTexture->getMipSize(srcMipIndex, width, height, depth);

        addImageBarrier(dstResourceVulkan, dstMipIndex, dstArrayIndex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        addImageBarrier(srcResourceVulkan, srcMipIndex, srcArrayIndex, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        flushBarriers();

        VkImageResolve region;
        region.srcSubresource.aspectMask = srcResourceVulkan->defaultAspectMask;
        region.srcSubresource.mipLevel = srcMipIndex;
        region.srcSubresource.baseArrayLayer = srcArrayIndex;
        region.srcSubresource.layerCount = 1;
        region.srcOffset = {0, 0, 0};
        region.dstSubresource.aspectMask = dstResourceVulkan->defaultAspectMask;
        region.dstSubresource.mipLevel = dstMipIndex;
        region.dstSubresource.baseArrayLayer = dstArrayIndex;
        region.dstSubresource.layerCount = 1;
        region.dstOffset = {0, 0, 0};
        region.extent = {(u32)width, (u32)height, (u32)depth};

        vkCmdResolveImage(cmdBuffer->getHandle(), srcResourceVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstResourceVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void VulkanContext::drawInstanced(u32 verticesCount, u32 instanceCount, i32 startInstance, i32 startVertex)
    {
        onDrawCall();
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        vkCmdDraw(cmdBuffer->getHandle(), verticesCount, instanceCount, startVertex, startInstance);
    }

    void VulkanContext::drawIndexedInstanced(u32 indicesCount, u32 instanceCount, i32 startInstance, i32 startVertex, i32 startIndex)
    {
        onDrawCall();
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        vkCmdDrawIndexed(cmdBuffer->getHandle(), indicesCount, instanceCount, startIndex, startVertex, startInstance);
    }

    void VulkanContext::drawInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        TK_ASSERT(bufferForArgs && enumHasAnyFlags(bufferForArgs->getFlags(), GfxBufferFlags::kArgument));
        onDrawCall();
        auto bufferForArgsVK = (VulkanBuffer *)bufferForArgs;
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        vkCmdDrawIndirect(cmdBuffer->getHandle(), bufferForArgsVK->getHandle(), (VkDeviceSize)offsetForArgs, 1, sizeof(VkDrawIndirectCommand));
    }

    void VulkanContext::drawIndexedInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        TK_ASSERT(bufferForArgs && enumHasAnyFlags(bufferForArgs->getFlags(), GfxBufferFlags::kArgument));
        onDrawCall();
        auto bufferForArgsVK = (VulkanBuffer *)bufferForArgs;
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        vkCmdDrawIndexedIndirect(cmdBuffer->getHandle(), bufferForArgsVK->getHandle(), (VkDeviceSize)offsetForArgs, 1, sizeof(VkDrawIndexedIndirectCommand));
    }

    void VulkanContext::setViewport(const Viewport &viewport)
    {
        vkCmdSetViewport(m_cmdBufferManager->getCmdBuffer()->getHandle(), 0, 1, (VkViewport *)&viewport);
    }

    void VulkanContext::setScissor(const Rectangle &scissorRect)
    {
        VkRect2D rect;
        rect.offset.x = (i32)scissorRect.location.x;
        rect.offset.y = (i32)scissorRect.location.y;
        rect.extent.width = (u32)scissorRect.size.x;
        rect.extent.height = (u32)scissorRect.size.y;
        vkCmdSetScissor(m_cmdBufferManager->getCmdBuffer()->getHandle(), 0, 1, &rect);
    }

    GfxPipelineState *VulkanContext::getState() const
    {
        return m_currentState;
    }

    void VulkanContext::setState(GfxPipelineState *state)
    {
        if (m_currentState != state)
        {
            m_currentState = static_cast<VulkanPipelineState *>(state);
            m_psDirtyFlag = true;
        }
    }

    void VulkanContext::clearState()
    {
        resetRenderTarget();
        resetSR();
        resetUA();
        resetCB();
        setState(nullptr);

        flushState();
    }

    void VulkanContext::flushState()
    {
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        if (cmdBuffer->isInsideRenderPass())
        {
            endRenderPass();
        }

        flushBarriers();
    }

    void VulkanContext::flush()
    {
        // Flush remaining and buffered commands
        flushState();
        m_currentState = nullptr;

        // Execute commands
        m_cmdBufferManager->submitActiveCmdBuffer();
        m_cmdBufferManager->prepareForNewActiveCommandBuffer();
        TK_ASSERT(m_cmdBufferManager->hasPendingActiveCmdBuffer() && m_cmdBufferManager->getActiveCmdBuffer()->getState() == VulkanCmdBuffer::State::kIsInsideBegin);
    }

    void VulkanContext::updateBuffer(GfxBuffer *buffer, const void *data, u32 size, u32 offset)
    {
        TK_ASSERT(data);
        TK_ASSERT(buffer && buffer->getSize() >= size);

        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        const auto bufferVulkan = static_cast<VulkanBuffer *>(buffer);

        // Memory transfer barrier
        // TODO: batch pipeline barriers
        const VkMemoryBarrier barrierBefore = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT};
        vkCmdPipelineBarrier(cmdBuffer->getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &barrierBefore, 0, nullptr, 0, nullptr);

        // Use direct update for small buffers
        const u32 alignedSize = TkMath::alignUp<u32>(size, 4);
        if (size <= 16 * 1024 && alignedSize <= buffer->getSize())
        {
            // addBufferBarrier(bufferVulkan, VK_ACCESS_TRANSFER_WRITE_BIT);
            // flushBarriers();

            vkCmdUpdateBuffer(cmdBuffer->getHandle(), bufferVulkan->getHandle(), offset, alignedSize, data);
        }
        else
        {
            auto staging = m_device->stagingManager.acquireBuffer(size, GfxResourceUsage::kStagingUpload);
            staging->setData(data, size);

            VkBufferCopy region;
            region.size = size;
            region.srcOffset = 0;
            region.dstOffset = offset;
            vkCmdCopyBuffer(cmdBuffer->getHandle(), ((VulkanBuffer *)staging)->getHandle(), ((VulkanBuffer *)buffer)->getHandle(), 1, &region);

            m_device->stagingManager.releaseBuffer(cmdBuffer, staging);
        }

        // Memory transfer barrier
        // TODO: batch pipeline barriers
        const VkMemoryBarrier barrierAfter = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT};
        vkCmdPipelineBarrier(cmdBuffer->getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &barrierAfter, 0, nullptr, 0, nullptr);
    }

    void VulkanContext::copyBuffer(GfxBuffer *dstBuffer, GfxBuffer *srcBuffer, u32 size, u32 dstOffset, u32 srcOffset)
    {
        TK_ASSERT(dstBuffer && srcBuffer);
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        // Ensure to end active render pass
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        auto dstBufferVulkan = static_cast<VulkanBuffer *>(dstBuffer);
        auto srcBufferVulkan = static_cast<VulkanBuffer *>(srcBuffer);

        // Transition resources
        addBufferBarrier(dstBufferVulkan, VK_ACCESS_TRANSFER_WRITE_BIT);
        addBufferBarrier(srcBufferVulkan, VK_ACCESS_TRANSFER_READ_BIT);
        flushBarriers();

        VkBufferCopy bufferCopy;
        bufferCopy.srcOffset = srcOffset;
        bufferCopy.dstOffset = dstOffset;
        bufferCopy.size = size;
        vkCmdCopyBuffer(cmdBuffer->getHandle(), srcBufferVulkan->getHandle(), dstBufferVulkan->getHandle(), 1, &bufferCopy);
    }

    void VulkanContext::updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch)
    {
        TK_ASSERT(texture && texture->isAllocated() && data);

        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        const auto textureVulkan = static_cast<VulkanTexture *>(texture);

        addImageBarrier(textureVulkan, mipIndex, arrayIndex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        flushBarriers();

        auto buffer = m_device->stagingManager.acquireBuffer(slicePitch, GfxResourceUsage::kStagingUpload);
        buffer->setData(data, slicePitch);

        // Setup buffer copy region
        i32 mipWidth, mipHeight, mipDepth;
        texture->getMipSize(mipIndex, mipWidth, mipHeight, mipDepth);
        VkBufferImageCopy bufferCopyRegion;
        TkMemory::memoryClear(&bufferCopyRegion, sizeof(bufferCopyRegion));
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = mipIndex;
        bufferCopyRegion.imageSubresource.baseArrayLayer = arrayIndex;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = static_cast<u32>(mipWidth);
        bufferCopyRegion.imageExtent.height = static_cast<u32>(mipHeight);
        bufferCopyRegion.imageExtent.depth = static_cast<u32>(mipDepth);

        // Copy mip level from staging buffer
        vkCmdCopyBufferToImage(cmdBuffer->getHandle(), ((VulkanBuffer *)buffer)->getHandle(), textureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

        m_device->stagingManager.releaseBuffer(cmdBuffer, buffer);
    }

    void VulkanContext::copyTexture(GfxTexture *dstResource, u32 dstSubresource, u32 dstX, u32 dstY, u32 dstZ, GfxTexture *srcResource, u32 srcSubresource)
    {
        TK_ASSERT(dstResource && srcResource);
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        // Ensure to end active render pass
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        const auto dstTextureVulkan = static_cast<VulkanTexture *>(dstResource);
        const auto srcTextureVulkan = static_cast<VulkanTexture *>(srcResource);

        const i32 dstMipIndex = (i32)dstSubresource % dstTextureVulkan->mipLevels();
        const i32 dstArrayIndex = (i32)dstSubresource / dstTextureVulkan->mipLevels();
        const i32 srcMipIndex = (i32)srcSubresource % srcTextureVulkan->mipLevels();
        const i32 srcArrayIndex = (i32)srcSubresource / srcTextureVulkan->mipLevels();
        i32 mipWidth, mipHeight, mipDepth;
        srcTextureVulkan->getMipSize(srcMipIndex, mipWidth, mipHeight, mipDepth);

        if (dstTextureVulkan->isStaging())
        {
            // Staging Texture -> Staging Texture
            if (srcTextureVulkan->isStaging())
            {
                TK_ASSERT(dstTextureVulkan->stagingBuffer && srcTextureVulkan->stagingBuffer);
                copyResource(dstTextureVulkan->stagingBuffer, srcTextureVulkan->stagingBuffer);
            }
            // Texture -> Staging Texture
            else
            {
                // Transition resources
                TK_ASSERT(dstTextureVulkan->stagingBuffer);
                addBufferBarrier(dstTextureVulkan->stagingBuffer, VK_ACCESS_TRANSFER_WRITE_BIT);
                addImageBarrier(srcTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                flushBarriers();

                // Copy
                VkBufferImageCopy region;
                TkMemory::memoryClear(&region, sizeof(VkBufferImageCopy));
                region.bufferOffset = 0; // TODO: calculate it based on dstSubresource and dstX/dstY/dstZ
                TK_ASSERT(dstX == dstY == dstZ == 0);
                TK_ASSERT(dstSubresource == 0);
                region.bufferRowLength = mipWidth;
                region.bufferImageHeight = mipHeight;
                region.imageOffset.x = 0;
                region.imageOffset.y = 0;
                region.imageOffset.z = 0;
                region.imageExtent.width = mipWidth;
                region.imageExtent.height = mipHeight;
                region.imageExtent.depth = mipDepth;
                region.imageSubresource.baseArrayLayer = srcArrayIndex;
                region.imageSubresource.layerCount = 1;
                region.imageSubresource.mipLevel = srcMipIndex;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                vkCmdCopyImageToBuffer(cmdBuffer->getHandle(), srcTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTextureVulkan->stagingBuffer->getHandle(), 1, &region);
            }
        }
        else
        {
            // Transition resources
            addImageBarrier(dstTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            addImageBarrier(srcTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            flushBarriers();

            // Copy
            VkImageCopy region;
            TkMemory::memoryClear(&region, sizeof(region));
            region.extent.width = mipWidth;
            region.extent.height = mipHeight;
            region.extent.depth = mipDepth;
            region.dstOffset.x = (i32)dstX;
            region.dstOffset.y = (i32)dstY;
            region.dstOffset.z = (i32)dstZ;
            region.srcSubresource.baseArrayLayer = srcArrayIndex;
            region.srcSubresource.layerCount = 1;
            region.srcSubresource.mipLevel = srcMipIndex;
            region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.dstSubresource.baseArrayLayer = dstArrayIndex;
            region.dstSubresource.layerCount = 1;
            region.dstSubresource.mipLevel = dstMipIndex;
            region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vkCmdCopyImage(cmdBuffer->getHandle(), srcTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }
    }

    void VulkanContext::resetCounter(GfxBuffer *buffer)
    {
        TK_ASSERT(buffer);
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        const auto bufferVulkan = static_cast<VulkanBuffer *>(buffer);
        const auto counter = bufferVulkan->counter;
        TK_ASSERT(counter != nullptr);

        addBufferBarrier(counter, VK_ACCESS_TRANSFER_WRITE_BIT);
        flushBarriers();

        u32 value = 0;
        vkCmdUpdateBuffer(cmdBuffer->getHandle(), counter->getHandle(), 0, 4, &value);
    }

    void VulkanContext::copyCounter(GfxBuffer *dstBuffer, u32 dstOffset, GfxBuffer *srcBuffer)
    {
        TK_ASSERT(dstBuffer && srcBuffer);
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        const auto dstBufferVulkan = static_cast<VulkanBuffer *>(dstBuffer);
        const auto srcBufferVulkan = static_cast<VulkanBuffer *>(srcBuffer);
        const auto counter = srcBufferVulkan->counter;
        TK_ASSERT(counter != nullptr);

        addBufferBarrier(dstBufferVulkan, VK_ACCESS_TRANSFER_WRITE_BIT);
        addBufferBarrier(counter, VK_ACCESS_TRANSFER_READ_BIT);
        flushBarriers();

        VkBufferCopy bufferCopy;
        bufferCopy.srcOffset = 0;
        bufferCopy.dstOffset = dstOffset;
        bufferCopy.size = 4;
        vkCmdCopyBuffer(cmdBuffer->getHandle(), counter->getHandle(), dstBufferVulkan->getHandle(), 1, &bufferCopy);
    }

    void VulkanContext::copyResource(GfxResource *dstResource, GfxResource *srcResource)
    {
        TK_ASSERT(dstResource && srcResource);
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        // Ensure to end active render pass
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        auto dstTextureVulkan = dynamic_cast<VulkanTexture *>(dstResource);
        auto srcTextureVulkan = dynamic_cast<VulkanTexture *>(srcResource);
        auto dstBufferVulkan = dynamic_cast<VulkanBuffer *>(dstResource);
        auto srcBufferVulkan = dynamic_cast<VulkanBuffer *>(srcResource);

        // Buffer -> Buffer
        if (srcBufferVulkan && dstBufferVulkan)
        {
            // Transition resources
            addBufferBarrier(dstBufferVulkan, VK_ACCESS_TRANSFER_WRITE_BIT);
            addBufferBarrier(srcBufferVulkan, VK_ACCESS_TRANSFER_READ_BIT);
            flushBarriers();

            // Copy
            VkBufferCopy bufferCopy;
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = 0;
            bufferCopy.size = srcBufferVulkan->getSize();
            TK_ASSERT(bufferCopy.size == dstBufferVulkan->getSize());
            vkCmdCopyBuffer(cmdBuffer->getHandle(), srcBufferVulkan->getHandle(), dstBufferVulkan->getHandle(), 1, &bufferCopy);
        }
        // Texture -> Texture
        else if (srcTextureVulkan && dstTextureVulkan)
        {
            if (dstTextureVulkan->isStaging())
            {
                // Staging Texture -> Staging Texture
                if (srcTextureVulkan->isStaging())
                {
                    TK_ASSERT(dstTextureVulkan->stagingBuffer && srcTextureVulkan->stagingBuffer);
                    copyResource(dstTextureVulkan->stagingBuffer, srcTextureVulkan->stagingBuffer);
                }
                // Texture -> Staging Texture
                else
                {
                    // Transition resources
                    TK_ASSERT(dstTextureVulkan->stagingBuffer);
                    addBufferBarrier(dstTextureVulkan->stagingBuffer, VK_ACCESS_TRANSFER_WRITE_BIT);
                    addImageBarrier(srcTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                    flushBarriers();

                    // Copy
                    i32 copyOffset = 0;
                    const i32 arraySize = srcTextureVulkan->arraySize();
                    const i32 mipMaps = srcTextureVulkan->mipLevels();
                    for (i32 arraySlice = 0; arraySlice < arraySize; arraySlice++)
                    {
                        VkBufferImageCopy regions[GFX_MAX_TEXTURE_MIP_LEVELS];
                        u32 mipWidth = srcTextureVulkan->width();
                        u32 mipHeight = srcTextureVulkan->height();
                        u32 mipDepth = srcTextureVulkan->depth();

                        for (i32 mipLevel = 0; mipLevel < mipMaps; mipLevel++)
                        {
                            VkBufferImageCopy &region = regions[mipLevel];
                            region.bufferOffset = copyOffset;
                            region.bufferRowLength = mipWidth;
                            region.bufferImageHeight = mipHeight;
                            region.imageOffset = {0, 0, 0};
                            region.imageExtent = {mipWidth, mipHeight, mipDepth};
                            region.imageSubresource.baseArrayLayer = arraySlice;
                            region.imageSubresource.layerCount = 1;
                            region.imageSubresource.mipLevel = mipLevel;
                            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                            // TODO: pitch/slice alignment on Vulkan?
                            copyOffset += dstTextureVulkan->computeSubresourceSize(mipLevel, 1, 1);

                            if (mipWidth != 1)
                                mipWidth >>= 1;
                            if (mipHeight != 1)
                                mipHeight >>= 1;
                            if (mipDepth != 1)
                                mipDepth >>= 1;
                        }

                        vkCmdCopyImageToBuffer(cmdBuffer->getHandle(), srcTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTextureVulkan->stagingBuffer->getHandle(), mipMaps, regions);
                    }
                }
            }
            else
            {
                // Transition resources
                addImageBarrier(dstTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                addImageBarrier(srcTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                flushBarriers();

                // Copy
                const i32 arraySize = srcTextureVulkan->arraySize();
                const i32 mipMaps = srcTextureVulkan->mipLevels();
                TK_ASSERT(dstTextureVulkan->mipLevels() == mipMaps);
                VkImageCopy regions[GFX_MAX_TEXTURE_MIP_LEVELS];
                u32 mipWidth = srcTextureVulkan->width();
                u32 mipHeight = srcTextureVulkan->height();
                u32 mipDepth = srcTextureVulkan->depth();
                for (i32 mipLevel = 0; mipLevel < mipMaps; mipLevel++)
                {
                    VkImageCopy &region = regions[mipLevel];
                    region.extent = {mipWidth, mipHeight, mipDepth};
                    region.srcOffset = {0, 0, 0};
                    region.srcSubresource.baseArrayLayer = 0;
                    region.srcSubresource.layerCount = arraySize;
                    region.srcSubresource.mipLevel = mipLevel;
                    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.dstOffset = {0, 0, 0};
                    region.dstSubresource.baseArrayLayer = 0;
                    region.dstSubresource.layerCount = arraySize;
                    region.dstSubresource.mipLevel = mipLevel;
                    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                    if (mipWidth != 1)
                        mipWidth >>= 1;
                    if (mipHeight != 1)
                        mipHeight >>= 1;
                    if (mipDepth != 1)
                        mipDepth >>= 1;
                }
                vkCmdCopyImage(cmdBuffer->getHandle(), srcTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipMaps, regions);
            }
        }
        else
        {
            TK_ASSERT(false && "Cannot copy data between buffer and texture.");
        }
    }

    void VulkanContext::copySubresource(GfxResource *dstResource, u32 dstSubresource, GfxResource *srcResource, u32 srcSubresource)
    {
        TK_ASSERT(dstResource && srcResource);
        const auto cmdBuffer = m_cmdBufferManager->getCmdBuffer();

        // Ensure to end active render pass
        if (cmdBuffer->isInsideRenderPass())
            endRenderPass();

        auto dstTextureVulkan = dynamic_cast<VulkanTexture *>(dstResource);
        auto srcTextureVulkan = dynamic_cast<VulkanTexture *>(srcResource);
        auto dstBufferVulkan = dynamic_cast<VulkanBuffer *>(dstResource);
        auto srcBufferVulkan = dynamic_cast<VulkanBuffer *>(srcResource);

        // Buffer -> Buffer
        if (srcBufferVulkan && dstBufferVulkan)
        {
            TK_ASSERT(dstSubresource == 0 && srcSubresource == 0);

            // Transition resources
            addBufferBarrier(dstBufferVulkan, VK_ACCESS_TRANSFER_WRITE_BIT);
            addBufferBarrier(srcBufferVulkan, VK_ACCESS_TRANSFER_READ_BIT);
            flushBarriers();

            // Copy
            VkBufferCopy bufferCopy;
            bufferCopy.srcOffset = 0;
            bufferCopy.dstOffset = 0;
            bufferCopy.size = srcBufferVulkan->getSize();
            TK_ASSERT(bufferCopy.size == dstBufferVulkan->getSize());
            vkCmdCopyBuffer(cmdBuffer->getHandle(), srcBufferVulkan->getHandle(), dstBufferVulkan->getHandle(), 1, &bufferCopy);
        }
        // Texture -> Texture
        else if (srcTextureVulkan && dstTextureVulkan)
        {
            const i32 dstMipMaps = dstTextureVulkan->mipLevels();
            const i32 dstMipIndex = dstSubresource % dstMipMaps;
            const i32 dstArrayIndex = dstSubresource / dstMipMaps;
            const i32 srcMipMaps = srcTextureVulkan->mipLevels();
            const i32 srcMipIndex = srcSubresource % srcMipMaps;
            const i32 srcArrayIndex = srcSubresource / srcMipMaps;

            if (dstTextureVulkan->isStaging())
            {
                // Staging Texture -> Staging Texture
                if (srcTextureVulkan->isStaging())
                {
                    TK_ASSERT(dstTextureVulkan->stagingBuffer && srcTextureVulkan->stagingBuffer);
                    copyResource(dstTextureVulkan->stagingBuffer, srcTextureVulkan->stagingBuffer);
                }
                // Texture -> Staging Texture
                else
                {
                    // Transition resources
                    TK_ASSERT(dstTextureVulkan->stagingBuffer);
                    addBufferBarrier(dstTextureVulkan->stagingBuffer, VK_ACCESS_TRANSFER_WRITE_BIT);
                    addImageBarrier(srcTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                    flushBarriers();

                    // Copy
                    i32 copyOffset = 0;
                    u32 subResourceCount = 0;
                    for (i32 arraySlice = 0; arraySlice < dstTextureVulkan->arraySize() && subResourceCount < dstSubresource; arraySlice++)
                    {
                        for (i32 mipLevel = 0; mipLevel < dstMipMaps && subResourceCount < dstSubresource; mipLevel++)
                        {
                            // TODO: pitch/slice alignment on Vulkan?
                            copyOffset += dstTextureVulkan->computeSubresourceSize(mipLevel, 1, 1);
                            subResourceCount++;
                        }
                    }
                    VkBufferImageCopy region;
                    region.bufferOffset = copyOffset;
                    region.bufferRowLength = TkMath::max<u32>(dstTextureVulkan->width() >> dstMipIndex, 1);
                    region.bufferImageHeight = TkMath::max<u32>(dstTextureVulkan->height() >> dstMipIndex, 1);
                    region.imageOffset = {0, 0, 0};
                    region.imageExtent = {TkMath::max<u32>(srcTextureVulkan->width() >> srcMipIndex, 1), TkMath::max<u32>(srcTextureVulkan->height() >> srcMipIndex, 1), TkMath::max<u32>(srcTextureVulkan->depth() >> srcMipIndex, 1)};
                    region.imageSubresource.baseArrayLayer = srcArrayIndex;
                    region.imageSubresource.layerCount = 1;
                    region.imageSubresource.mipLevel = srcMipIndex;
                    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    vkCmdCopyImageToBuffer(cmdBuffer->getHandle(), srcTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTextureVulkan->stagingBuffer->getHandle(), 1, &region);
                }
            }
            else
            {
                // Transition resources
                addImageBarrier(dstTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                addImageBarrier(srcTextureVulkan, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                flushBarriers();

                // Copy
                i32 mipWidth, mipHeight, mipDepth;
                srcTextureVulkan->getMipSize(srcMipIndex, mipWidth, mipHeight, mipDepth);
                VkImageCopy region;
                ;
                region.extent = {TkMath::max<u32>(mipWidth, 1), TkMath::max<u32>(mipHeight, 1), TkMath::max<u32>(mipDepth, 1)};
                region.srcOffset = {0, 0, 0};
                region.srcSubresource.baseArrayLayer = srcArrayIndex;
                region.srcSubresource.layerCount = 1;
                region.srcSubresource.mipLevel = srcMipIndex;
                region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.dstOffset = {0, 0, 0};
                region.dstSubresource.baseArrayLayer = dstArrayIndex;
                region.dstSubresource.layerCount = 1;
                region.dstSubresource.mipLevel = dstMipIndex;
                region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                vkCmdCopyImage(cmdBuffer->getHandle(), srcTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTextureVulkan->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            }
        }
        else
        {
            TK_ASSERT(false && "Cannot copy data between buffer and texture.");
        }
    }

}

#endif
