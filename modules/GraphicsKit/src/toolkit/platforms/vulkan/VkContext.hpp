/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include "toolkit/collection/StaticArray.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/graphics/GfxContext.hpp"

namespace toolkit
{

// Enables using batched pipeline barriers to improve performance.
#define VK_ENABLE_BARRIERS_BATCHING 1
// Enables pipeline barriers debugging.
#define VK_ENABLE_BARRIERS_DEBUG (TK_DEBUG && 0)
// Size of the pipeline barriers buffer size (will be auto-flushed on overflow).
#define VK_BARRIER_BUFFER_SIZE 16

    // The Vulkan pipeline resources layout barrier batching structure.
    struct VulkanPipelineBarrier
    {
        VkPipelineStageFlags sourceStage = 0;
        VkPipelineStageFlags destStage = 0;
        TkStaticArray<VkImageMemoryBarrier, VK_BARRIER_BUFFER_SIZE> imageBarriers;
        TkStaticArray<VkBufferMemoryBarrier, VK_BARRIER_BUFFER_SIZE> bufferBarriers;
#if VK_ENABLE_BARRIERS_DEBUG
        TkStaticArray<VulkanTextureView *, VK_BARRIER_BUFFER_SIZE> imageBarriersDebug;
#endif

        TK_FORCE_INLINE bool isFull() const
        {
            return imageBarriers.count() == VK_BARRIER_BUFFER_SIZE || bufferBarriers.count() == VK_BARRIER_BUFFER_SIZE;
        }

        TK_FORCE_INLINE bool hasBarrier() const
        {
            return imageBarriers.count() + bufferBarriers.count() != 0;
        }

        void execute(const class VulkanCmdBuffer *cmdBuffer);
    };

    // GPU Context for Vulkan backend.
    class VulkanContext : public GfxContext
    {
    private:
        class VulkanDevice *m_device;
        class VulkanQueue *m_queue;
        class VulkanCmdBufferManager *m_cmdBufferManager;
        VulkanPipelineBarrier m_barriers;

        i32 m_psDirtyFlag : 1;
        i32 m_rtDirtyFlag : 1;
        i32 m_cbDirtyFlag : 1;

        i32 m_rtCount;
        i32 m_vbCount;
        u32 m_stencilRef;

        class VulkanRenderPass *m_renderPass;
        class VulkanPipelineState *m_currentState;
        class VulkanVertexLayout *m_vertexLayout;
        class VulkanTextureView *m_rtDepth;
        VulkanTextureView *m_rtHandles[GFX_MAX_RT_BINDED];
        class VulkanDescriptorOwnerResource *m_cbHandles[GFX_MAX_CB_BINDED];
        VulkanDescriptorOwnerResource *m_srHandles[GFX_MAX_SR_BINDED];
        VulkanDescriptorOwnerResource *m_uaHandles[GFX_MAX_UA_BINDED];
        VkSampler m_samplerHandles[GFX_MAX_SAMPLER_BINDED];
        VulkanDescriptorOwnerResource **m_handles[(i32)SpirvShaderResourceBindingType::MAX];
#if TK_DEBUG
        u32 m_handlesSizes[(i32)SpirvShaderResourceBindingType::MAX];
#endif

        typedef TkArray<class VulkanDescriptorPool *> DescriptorPoolArray;
        TkHashMap<u32, DescriptorPoolArray> m_descriptorPools;

    public:
        VulkanContext(VulkanDevice *device, VulkanQueue *queue);
        ~VulkanContext();

    public:
        TK_FORCE_INLINE VulkanQueue *getQueue() const
        {
            return m_queue;
        }

        TK_FORCE_INLINE VulkanCmdBufferManager *getCmdBufferManager() const
        {
            return m_cmdBufferManager;
        }

        void addImageBarrier(VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout, const VkImageSubresourceRange &subresourceRange, VulkanTextureView *handle);
        void addImageBarrier(VulkanTextureView *handle, VkImageLayout dstLayout);
        void addImageBarrier(class VulkanTexture *texture, i32 mipSlice, i32 arraySlice, VkImageLayout dstLayout);
        void addImageBarrier(VulkanTexture *texture, VkImageLayout dstLayout);
        void addBufferBarrier(class VulkanBuffer *buffer, VkAccessFlags dstAccess);

        void flushBarriers();

        // outSets must have been previously pre-allocated
        VulkanDescriptorPool *allocateDescriptorSets(const VkDescriptorSetAllocateInfo &descriptorSetAllocateInfo, const class VulkanDescriptorSetLayout &layout, VkDescriptorSet *outSets);

        void beginRenderPass();
        void endRenderPass();

    private:
        void updateDescriptorSets(const struct SpirvShaderDescriptorInfo &descriptorInfo, class VulkanDescriptorSetWriter &dsWriter, bool &needsWrite);
        void updateDescriptorSets(class VulkanComputePipelineState *pipelineState);
        void onDrawCall();

    public:
        void frameBegin() override;
        void frameEnd() override;
#if GFX_ALLOW_PROFILE_EVENTS
        void eventBegin(const char *name) override;
        void eventEnd() override;
#endif
        void *getNativePtr() const override;
        //TODO bool isDepthBufferBinded() override;
        void clear(GfxTextureView *rt, const Color &color) override;
        void clearDepth(GfxTextureView *depthBuffer, float depthValue, u8 stencilValue) override;
        void clearUA(GfxBuffer *buf, const FVec4 &value) override;
        void clearUA(GfxBuffer *buf, const u32 value[4]) override;
        void clearUA(GfxTexture *texture, const u32 value[4]) override;
        void clearUA(GfxTexture *texture, const FVec4 &value) override;
        void resetRenderTarget() override;
        void setRenderTarget(GfxTextureView *rt) override;
        void setRenderTarget(GfxTextureView *depthBuffer, GfxTextureView *rt) override;
        void setRenderTarget(GfxTextureView *depthBuffer, const TkSpan<GfxTextureView *> &rts) override;
        void setBlendFactor(const FVec4 &value) override;
        void setStencilRef(u32 value) override;
        void resetSR() override;
        void resetUA() override;
        void resetCB() override;
        void bindCB(i32 slot, GfxConstantBuffer *cb) override;
        void bindSR(i32 slot, GfxResourceView *view) override;
        void bindUA(i32 slot, GfxResourceView *view) override;
        void bindVB(const TkSpan<GfxBuffer *> &vertexBuffers, const u32 *vertexBuffersOffsets = nullptr, GfxVertexLayout *vertexLayout = nullptr) override;
        void bindIB(GfxBuffer *indexBuffer) override;
        void bindSampler(i32 slot, GfxSampler *sampler) override;
        void updateCB(GfxConstantBuffer *cb, const void *data) override;
        void dispatch(GfxShaderProgramCS *shader, u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) override;
        void dispatchIndirect(GfxShaderProgramCS *shader, GfxBuffer *bufferForArgs, u32 offsetForArgs) override;
        void resolveMultisample(GfxTexture *sourceMultisampleTexture, GfxTexture *destTexture, i32 sourceSubResource, i32 destSubResource, PixelFormat format) override;
        void drawInstanced(u32 verticesCount, u32 instanceCount, i32 startInstance, i32 startVertex) override;
        void drawIndexedInstanced(u32 indicesCount, u32 instanceCount, i32 startInstance, i32 startVertex, i32 startIndex) override;
        void drawInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs) override;
        void drawIndexedInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs) override;
        void setViewport(const Viewport &viewport) override;
        void setScissor(const Rectangle &scissorRect) override;
        GfxPipelineState *getState() const override;
        void setState(GfxPipelineState *state) override;
        void clearState() override;
        void flushState() override;
        void flush() override;
        void updateBuffer(GfxBuffer *buffer, const void *data, u32 size, u32 offset) override;
        void copyBuffer(GfxBuffer *dstBuffer, GfxBuffer *srcBuffer, u32 size, u32 dstOffset, u32 srcOffset) override;
        void updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch) override;
        void copyTexture(GfxTexture *dstResource, u32 dstSubresource, u32 dstX, u32 dstY, u32 dstZ, GfxTexture *srcResource, u32 srcSubresource) override;
        void resetCounter(GfxBuffer *buffer) override;
        void copyCounter(GfxBuffer *dstBuffer, u32 dstOffset, GfxBuffer *srcBuffer) override;
        void copyResource(GfxResource *dstResource, GfxResource *srcResource) override;
        void copySubresource(GfxResource *dstResource, u32 dstSubresource, GfxResource *srcResource, u32 srcSubresource) override;
    };

}

#endif