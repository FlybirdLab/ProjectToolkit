/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include "toolkit/threading/Sync.hpp"

namespace toolkit
{
#define VK_DESCRIPTOR_TYPE_BEGIN VK_DESCRIPTOR_TYPE_SAMPLER
#define VK_DESCRIPTOR_TYPE_END VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT

    class VulkanDescriptorSet
    {
    public:
        enum Stage
        {
            // Vertex shader stage
            kVertex,
            // Pixel shader stage
            kPixel,
#if GFX_ALLOW_GEOMETRY_SHADERS
            // Geometry shader stage
            kGeometry,
#endif
#if GFX_ALLOW_TESSELLATION_SHADERS
            // Hull shader stage
            kHull,
            // Domain shader stage
            kDomain,
#endif
            // Graphics pipeline stages count
            kGraphicsStagesCount,
            // Compute pipeline slot
            kCompute = 0,
            // The maximum amount of slots for all stages
            kMax = kGraphicsStagesCount,
        };

    public:
        template <typename T>
        static inline bool copyAndReturnNotEqual(T &a, T b)
        {
            const bool result = a != b;
            a = b;
            return result;
        }
    };

    class VulkanDescriptorSetLayoutInfo
    {
    public:
        struct SetLayout
        {
            TkArray<VkDescriptorSetLayoutBinding> layoutBindings;
        };

        u32 hash = 0;
        u32 setLayoutsHash = 0;
        u32 layoutTypes[VK_DESCRIPTOR_TYPE_END + 1];
        TkArray<SetLayout> setLayouts;

    public:
        VulkanDescriptorSetLayoutInfo()
        {
            TkMemory::memoryClear(layoutTypes, sizeof(layoutTypes));
        }

        void addBindingsForStage(VkShaderStageFlagBits stageFlags, VulkanDescriptorSet::Stage descSet, const SpirvShaderDescriptorInfo *descriptorInfo);

        void copyFrom(const VulkanDescriptorSetLayoutInfo &info)
        {
            TkMemory::memoryCopy(layoutTypes, info.layoutTypes, sizeof(layoutTypes));
            hash = info.hash;
            setLayoutsHash = info.setLayoutsHash;
            setLayouts = info.setLayouts;
        }

        bool operator==(const VulkanDescriptorSetLayoutInfo &other) const;

        friend inline u32 getHash(const VulkanDescriptorSetLayoutInfo &key)
        {
            return key.hash;
        }
    };

    class VulkanDescriptorSetLayout : public VulkanDescriptorSetLayoutInfo
    {
    public:
        typedef TkStaticArray<VkDescriptorSetLayout, VulkanDescriptorSet::kMax> HandlesArray;

        class VulkanDevice *device;
        HandlesArray handles;
        VkDescriptorSetAllocateInfo allocateInfo;

        VulkanDescriptorSetLayout(VulkanDevice *device);
        ~VulkanDescriptorSetLayout();

        void compile();

        friend inline u32 getHash(const VulkanDescriptorSetLayout &key)
        {
            return key.hash;
        }
    };

    class VulkanDescriptorPool
    {
    private:
        VulkanDevice *m_device;
        VkDescriptorPool m_handle;

        u32 m_descriptorSetsMax;
        u32 m_allocatedDescriptorSetsCount;
        u32 m_allocatedDescriptorSetsCountMax;

        const VulkanDescriptorSetLayout &m_layout;

    public:
        VulkanDescriptorPool(VulkanDevice *device, const VulkanDescriptorSetLayout &layout);
        ~VulkanDescriptorPool();

    public:
        inline VkDescriptorPool getHandle() const
        {
            return m_handle;
        }

        inline bool isEmpty() const
        {
            return m_allocatedDescriptorSetsCount == 0;
        }

        inline bool canAllocate(const VulkanDescriptorSetLayout &layout) const
        {
            return m_descriptorSetsMax > m_allocatedDescriptorSetsCount + layout.setLayouts.count();
        }

        inline u32 getAllocatedDescriptorSetsCount() const
        {
            return m_allocatedDescriptorSetsCount;
        }

    public:
        void track(const VulkanDescriptorSetLayout &layout);
        void trackRemoveUsage(const VulkanDescriptorSetLayout &layout);
        void reset();
        bool allocateDescriptorSets(const VulkanDescriptorSetLayout &layout, const VkDescriptorSetAllocateInfo &descriptorSetAllocateInfo, VkDescriptorSet *result);
    };

    class VulkanDescriptorPoolSetContainer;

    class VulkanTypedDescriptorPoolSet
    {
        friend VulkanDescriptorPoolSetContainer;

    private:
        VulkanDevice *m_device;
        const VulkanDescriptorPoolSetContainer *m_owner;
        const VulkanDescriptorSetLayout &m_layout;

        class PoolList
        {
        public:
            VulkanDescriptorPool *element;
            PoolList *next;

            PoolList(VulkanDescriptorPool *element, PoolList *next = nullptr)
            {
                this->element = element;
                this->next = next;
            }
        };

        PoolList *m_poolListHead = nullptr;
        PoolList *m_poolListCurrent = nullptr;

    public:
        VulkanTypedDescriptorPoolSet(VulkanDevice *device, const VulkanDescriptorPoolSetContainer *owner, const VulkanDescriptorSetLayout &layout)
            : m_device(device), m_owner(owner), m_layout(layout)
        {
            pushNewPool();
        };

        ~VulkanTypedDescriptorPoolSet();

        bool allocateDescriptorSets(const VulkanDescriptorSetLayout &layout, VkDescriptorSet *outSets);

        const VulkanDescriptorPoolSetContainer *getOwner() const
        {
            return m_owner;
        }

    private:
        VulkanDescriptorPool *getFreePool(bool forceNewPool = false);
        VulkanDescriptorPool *pushNewPool();
        void reset();
    };

    class VulkanDescriptorPoolSetContainer
    {
    private:
        VulkanDevice *m_device;
        TkHashMap<u32, VulkanTypedDescriptorPoolSet *> m_typedDescriptorPools;

    public:
        VulkanDescriptorPoolSetContainer(VulkanDevice *device);
        ~VulkanDescriptorPoolSetContainer();

    public:
        VulkanTypedDescriptorPoolSet *acquireTypedPoolSet(const VulkanDescriptorSetLayout &layout);
        void reset();

        mutable u64 refs = 0;
        mutable u64 lastFrameUsed;
    };

    class VulkanDescriptorPoolsManager
    {
    private:
        VulkanDevice *m_device = nullptr;
        TkMutex m_mtx;
        TkArray<VulkanDescriptorPoolSetContainer *> m_poolSets;

    public:
        VulkanDescriptorPoolsManager(VulkanDevice *device);
        ~VulkanDescriptorPoolsManager();

        VulkanDescriptorPoolSetContainer *acquirePoolSetContainer();
        void collect();
    };

    class VulkanPipelineLayout
    {
    public:
        VulkanDevice *device;
        VkPipelineLayout handle;
        VulkanDescriptorSetLayout descriptorSetLayout;

        VulkanPipelineLayout(VulkanDevice *device, const VulkanDescriptorSetLayoutInfo &layout);
        ~VulkanPipelineLayout();
    };

    struct VulkanDescriptorSetWriteContainer
    {
        TkArray<VkDescriptorImageInfo> descriptorImageInfo;
        TkArray<VkDescriptorBufferInfo> descriptorBufferInfo;
        TkArray<VkBufferView> descriptorTexelBufferView;
        TkArray<VkWriteDescriptorSet> descriptorWrites;
        TkArray<byte> bindingToDynamicOffset;

        void release()
        {
            descriptorImageInfo.resize(0);
            descriptorBufferInfo.resize(0);
            descriptorTexelBufferView.resize(0);
            descriptorWrites.resize(0);
            bindingToDynamicOffset.resize(0);
        }
    };

    class VulkanDescriptorSetWriter
    {
    public:
        VkWriteDescriptorSet *writeDescriptors = nullptr;
        byte *bindingToDynamicOffset = nullptr;
        u32 *dynamicOffsets = nullptr;
        u32 writesCount = 0;

    public:
        u32 setupDescriptorWrites(const SpirvShaderDescriptorInfo &info, VkWriteDescriptorSet *writeDescriptors, VkDescriptorImageInfo *imageInfo, VkDescriptorBufferInfo *bufferInfo, VkBufferView *texelBufferView, byte *bindingToDynamicOffset);

        bool writeUniformBuffer(u32 descriptorIndex, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, u32 index = 0) const
        {
            TK_ASSERT(descriptorIndex < writesCount);
            TK_ASSERT(writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            auto *bufferInfo = const_cast<VkDescriptorBufferInfo *>(writeDescriptors[descriptorIndex].pBufferInfo + index);
            bool edited = VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->buffer, buffer);
            edited |= VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->offset, offset);
            edited |= VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->range, range);
            return edited;
        }

        bool writeDynamicUniformBuffer(u32 descriptorIndex, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, u32 dynamicOffset, u32 index = 0) const
        {
            TK_ASSERT(descriptorIndex < writesCount);
            TK_ASSERT(writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
            auto *bufferInfo = const_cast<VkDescriptorBufferInfo *>(writeDescriptors[descriptorIndex].pBufferInfo + index);
            bool edited = VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->buffer, buffer);
            edited |= VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->offset, offset);
            edited |= VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->range, range);
            const byte dynamicOffsetIndex = bindingToDynamicOffset[descriptorIndex];
            dynamicOffsets[dynamicOffsetIndex] = dynamicOffset;
            return edited;
        }

        bool writeSampler(u32 descriptorIndex, VkSampler sampler, u32 index = 0) const
        {
            TK_ASSERT(descriptorIndex < writesCount);
            TK_ASSERT(writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            auto *imageInfo = const_cast<VkDescriptorImageInfo *>(writeDescriptors[descriptorIndex].pImageInfo + index);
            bool edited = VulkanDescriptorSet::copyAndReturnNotEqual(imageInfo->sampler, sampler);
            return edited;
        }

        bool writeImage(u32 descriptorIndex, VkImageView imageView, VkImageLayout layout, u32 index = 0) const
        {
            TK_ASSERT(descriptorIndex < writesCount);
            TK_ASSERT(writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            auto *imageInfo = const_cast<VkDescriptorImageInfo *>(writeDescriptors[descriptorIndex].pImageInfo + index);
            bool edited = VulkanDescriptorSet::copyAndReturnNotEqual(imageInfo->imageView, imageView);
            edited |= VulkanDescriptorSet::copyAndReturnNotEqual(imageInfo->imageLayout, layout);
            return edited;
        }

        bool writeStorageImage(u32 descriptorIndex, VkImageView imageView, VkImageLayout layout, u32 index = 0) const
        {
            TK_ASSERT(descriptorIndex < writesCount);
            TK_ASSERT(writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            auto *imageInfo = const_cast<VkDescriptorImageInfo *>(writeDescriptors[descriptorIndex].pImageInfo + index);
            bool edited = VulkanDescriptorSet::copyAndReturnNotEqual(imageInfo->imageView, imageView);
            edited |= VulkanDescriptorSet::copyAndReturnNotEqual(imageInfo->imageLayout, layout);
            return edited;
        }

        bool writeStorageTexelBuffer(u32 descriptorIndex, VkBufferView bufferView, u32 index = 0) const
        {
            TK_ASSERT(descriptorIndex < writesCount);
            TK_ASSERT(writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
            auto *bufferInfo = const_cast<VkBufferView *>(writeDescriptors[descriptorIndex].pTexelBufferView + index);
            return VulkanDescriptorSet::copyAndReturnNotEqual(*bufferInfo, bufferView);
        }

        bool writeStorageBuffer(u32 descriptorIndex, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, u32 index = 0) const
        {
            TK_ASSERT(descriptorIndex < writesCount);
            TK_ASSERT(writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
            auto *bufferInfo = const_cast<VkDescriptorBufferInfo *>(writeDescriptors[descriptorIndex].pBufferInfo + index);
            bool edited = VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->buffer, buffer);
            edited |= VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->offset, offset);
            edited |= VulkanDescriptorSet::copyAndReturnNotEqual(bufferInfo->range, range);
            return edited;
        }

        bool writeUniformTexelBuffer(u32 descriptorIndex, VkBufferView view, u32 index = 0) const
        {
            TK_ASSERT(descriptorIndex < writesCount);
            TK_ASSERT(writeDescriptors[descriptorIndex].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
            auto *bufferInfo = const_cast<VkBufferView *>(writeDescriptors[descriptorIndex].pTexelBufferView + index);
            return VulkanDescriptorSet::copyAndReturnNotEqual(*bufferInfo, view);
        }

        void setDescriptorSet(VkDescriptorSet descriptorSet) const
        {
            for (u32 i = 0; i < writesCount; i++)
                writeDescriptors[i].dstSet = descriptorSet;
        }
    };

}

#endif
