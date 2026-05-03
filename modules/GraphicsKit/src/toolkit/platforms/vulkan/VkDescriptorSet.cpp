/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkDescriptorSet.hpp"

#if GFX_API_VULKAN

#include "VkDevice.hpp"
#include "VkUtils.hpp"
#include "toolkit/threading/Sync.hpp"
#include "toolkit/base/CheckSum.hpp"

namespace toolkit
{

    void VulkanDescriptorSetLayoutInfo::addBindingsForStage(VkShaderStageFlagBits stageFlags, VulkanDescriptorSet::Stage descSet, const SpirvShaderDescriptorInfo *descriptorInfo)
    {
        const i32 descriptorSetIndex = descSet;
        if (descriptorSetIndex >= setLayouts.count())
            setLayouts.resize(descriptorSetIndex + 1);
        SetLayout &descSetLayout = setLayouts[descriptorSetIndex];

        VkDescriptorSetLayoutBinding binding;
        binding.stageFlags = stageFlags;
        binding.pImmutableSamplers = nullptr;
        for (u32 descriptorIndex = 0; descriptorIndex < descriptorInfo->descriptorTypesCount; descriptorIndex++)
        {
            auto &descriptor = descriptorInfo->descriptorTypes[descriptorIndex];
            binding.binding = descriptorIndex;
            binding.descriptorType = descriptor.descriptorType;
            binding.descriptorCount = descriptor.count;

            layoutTypes[binding.descriptorType]++;
            descSetLayout.layoutBindings.add(binding);
            hash = TkCheckSum::memCrc32(&binding, sizeof(binding), hash);
        }
    }

    bool VulkanDescriptorSetLayoutInfo::operator==(const VulkanDescriptorSetLayoutInfo &other) const
    {
        if (other.setLayouts.count() != setLayouts.count())
            return false;
#if VK_HASH_POOLS_WITH_LAYOUT_TYPES
        if (other.setLayoutsHash != setLayoutsHash)
            return false;
#endif
        for (i32 index = 0; index < other.setLayouts.count(); index++)
        {
            const i32 bindingsCount = setLayouts[index].layoutBindings.count();
            if (other.setLayouts[index].layoutBindings.count() != bindingsCount)
                return false;
            if (bindingsCount != 0 && TkMemory::memoryCompare(other.setLayouts[index].layoutBindings.get(), setLayouts[index].layoutBindings.get(), bindingsCount * sizeof(VkDescriptorSetLayoutBinding)))
                return false;
        }
        return true;
    }

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice *device)
        : device(device)
    {
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        for (VkDescriptorSetLayout &handle : handles)
            device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kDescriptorSetLayout, handle);
    }

    void VulkanDescriptorSetLayout::compile()
    {
#if TK_DEBUG
        TK_ASSERT(handles.isEmpty());
        const VkPhysicalDeviceLimits &limits = device->physicalDeviceLimits;
        TK_ASSERT(layoutTypes[VK_DESCRIPTOR_TYPE_SAMPLER] + layoutTypes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] < limits.maxDescriptorSetSamplers);
        TK_ASSERT(layoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] + layoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] < limits.maxDescriptorSetUniformBuffers);
        TK_ASSERT(layoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] < limits.maxDescriptorSetUniformBuffersDynamic);
        TK_ASSERT(layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] + layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] < limits.maxDescriptorSetStorageBuffers);
        TK_ASSERT(layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] < limits.maxDescriptorSetStorageBuffersDynamic);
        TK_ASSERT(layoutTypes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] + layoutTypes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] + layoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER] < limits.maxDescriptorSetSampledImages);
        TK_ASSERT(layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] + layoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER] < limits.maxDescriptorSetStorageImages);
#endif

        handles.resize(setLayouts.count());
        for (i32 i = 0; i < setLayouts.count(); i++)
        {
            auto &layout = setLayouts.get()[i];
            VkDescriptorSetLayoutCreateInfo layoutInfo;
            VkUtils::zeroStruct(layoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
            layoutInfo.bindingCount = layout.layoutBindings.count();
            layoutInfo.pBindings = layout.layoutBindings.get();
            VALIDATE_VK_RESULT(vkCreateDescriptorSetLayout(device->device, &layoutInfo, nullptr, &handles[i]));
        }

#if VK_HASH_POOLS_WITH_LAYOUT_TYPES
        if (setLayoutsHash == 0)
            setLayoutsHash = TkCheckSum::memCrc32(layoutTypes, sizeof(layoutTypes));
#endif

        VkUtils::zeroStruct(allocateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
        allocateInfo.descriptorSetCount = handles.count();
        allocateInfo.pSetLayouts = handles.get();
    }

    VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice *device, const VulkanDescriptorSetLayout &layout)
        : m_device(device), m_handle(VK_NULL_HANDLE), m_descriptorSetsMax(0), m_allocatedDescriptorSetsCount(0), m_allocatedDescriptorSetsCountMax(0), m_layout(layout)
    {
        TkStaticArray<VkDescriptorPoolSize, VK_DESCRIPTOR_TYPE_END + 1> types;

        // The maximum amount of descriptor sets layout allocations to hold
        const u32 MaxSetsAllocations = 256;
        m_descriptorSetsMax = MaxSetsAllocations * (VK_HASH_POOLS_WITH_LAYOUT_TYPES ? 1 : m_layout.setLayouts.count());
        for (u32 typeIndex = VK_DESCRIPTOR_TYPE_BEGIN; typeIndex <= VK_DESCRIPTOR_TYPE_END; typeIndex++)
        {
            const VkDescriptorType descriptorType = (VkDescriptorType)typeIndex;
            const u32 typesUsed = m_layout.layoutTypes[descriptorType];
            if (typesUsed > 0)
            {
                VkDescriptorPoolSize &type = types.addOne();
                TkMemory::memoryClear(&type, sizeof(type));
                type.type = descriptorType;
                type.descriptorCount = typesUsed * MaxSetsAllocations;
            }
        }

        VkDescriptorPoolCreateInfo createInfo;
        VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
        createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        createInfo.poolSizeCount = types.count();
        createInfo.pPoolSizes = types.get();
        createInfo.maxSets = m_descriptorSetsMax;
        VALIDATE_VK_RESULT(vkCreateDescriptorPool(m_device->device, &createInfo, nullptr, &m_handle));
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        if (m_handle != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(m_device->device, m_handle, nullptr);
        }
    }

    void VulkanDescriptorPool::track(const VulkanDescriptorSetLayout &layout)
    {
#if TK_DEBUG
        for (u32 typeIndex = VK_DESCRIPTOR_TYPE_BEGIN; typeIndex <= VK_DESCRIPTOR_TYPE_END; typeIndex++)
        {
            TK_ASSERT(m_layout.layoutTypes[typeIndex] == layout.layoutTypes[typeIndex]);
        }
#endif
        m_allocatedDescriptorSetsCount += layout.setLayouts.count();
        m_allocatedDescriptorSetsCountMax = TkMath::max(m_allocatedDescriptorSetsCount, m_allocatedDescriptorSetsCountMax);
    }

    void VulkanDescriptorPool::trackRemoveUsage(const VulkanDescriptorSetLayout &layout)
    {
#if TK_DEBUG
        for (u32 typeIndex = VK_DESCRIPTOR_TYPE_BEGIN; typeIndex <= VK_DESCRIPTOR_TYPE_END; typeIndex++)
        {
            TK_ASSERT(m_layout.layoutTypes[typeIndex] == layout.layoutTypes[typeIndex]);
        }
#endif
        m_allocatedDescriptorSetsCount -= layout.setLayouts.count();
    }

    void VulkanDescriptorPool::reset()
    {
        if (m_handle != VK_NULL_HANDLE)
        {
            VALIDATE_VK_RESULT(vkResetDescriptorPool(m_device->device, m_handle, 0));
        }
        m_allocatedDescriptorSetsCount = 0;
    }

    bool VulkanDescriptorPool::allocateDescriptorSets(const VkDescriptorSetAllocateInfo &descriptorSetAllocateInfo, VkDescriptorSet *result)
    {
        VkDescriptorSetAllocateInfo allocateInfo = descriptorSetAllocateInfo;
        allocateInfo.descriptorPool = m_handle;
        return vkAllocateDescriptorSets(m_device->device, &allocateInfo, result) == VK_SUCCESS;
    }

    VulkanTypedDescriptorPoolSet::~VulkanTypedDescriptorPoolSet()
    {
        for (auto pool = m_poolListHead; pool;)
        {
            const auto next = pool->next;
            TkMemory::deletePtr(pool->element);
            TkMemory::deletePtr(pool);
            pool = next;
        }
    }

    bool VulkanTypedDescriptorPoolSet::allocateDescriptorSets(const VulkanDescriptorSetLayout &layout, VkDescriptorSet *outSets)
    {
        if (layout.handles.hasItems())
        {
            auto *pool = m_poolListCurrent->element;
            while (!pool->allocateDescriptorSets(layout.allocateInfo, outSets))
            {
                pool = getFreePool(true);
            }
            return true;
        }
        return true;
    }

    VulkanDescriptorPool *VulkanTypedDescriptorPoolSet::getFreePool(bool forceNewPool)
    {
        if (!forceNewPool)
        {
            return m_poolListCurrent->element;
        }
        if (m_poolListCurrent->next)
        {
            m_poolListCurrent = m_poolListCurrent->next;
            return m_poolListCurrent->element;
        }
        return pushNewPool();
    }

    VulkanDescriptorPool *VulkanTypedDescriptorPoolSet::pushNewPool()
    {
        auto *newPool = TkMemory::newObject<VulkanDescriptorPool>(TkMemory::getDefault(), m_device, m_layout);
        if (m_poolListCurrent)
        {
            m_poolListCurrent->next = TkMemory::newObject<PoolList>(TkMemory::getDefault(), newPool);
            m_poolListCurrent = m_poolListCurrent->next;
        }
        else
        {
            m_poolListCurrent = m_poolListHead = TkMemory::newObject<PoolList>(TkMemory::getDefault(), newPool);
        }
        return newPool;
    }

    void VulkanTypedDescriptorPoolSet::reset()
    {
        for (PoolList *pool = m_poolListHead; pool; pool = pool->next)
        {
            pool->element->reset();
        }
        m_poolListCurrent = m_poolListHead;
    }

    VulkanDescriptorPoolSetContainer::VulkanDescriptorPoolSetContainer(VulkanDevice *device)
        : m_device(device), lastFrameUsed(device ? device->getFrameIndex() : 0)
    {
    }

    VulkanDescriptorPoolSetContainer::~VulkanDescriptorPoolSetContainer()
    {
        m_typedDescriptorPools.clearDelete();
    }

    VulkanTypedDescriptorPoolSet *VulkanDescriptorPoolSetContainer::acquireTypedPoolSet(const VulkanDescriptorSetLayout &layout)
    {
        const u32 hash = VK_HASH_POOLS_WITH_LAYOUT_TYPES ? layout.setLayoutsHash : getHash(layout);
        VulkanTypedDescriptorPoolSet *typedPool;
        bool hasResult = m_typedDescriptorPools.containsKey(hash);
        if (hasResult)
        {
            typedPool = m_typedDescriptorPools[hash];
        }
        else
        {
            typedPool = TkMemory::newObject<VulkanTypedDescriptorPoolSet>(TkMemory::getDefault(), m_device, this, layout);
            m_typedDescriptorPools.add(hash, typedPool);
        }
        return typedPool;
    }

    void VulkanDescriptorPoolSetContainer::reset()
    {
        for (auto i = m_typedDescriptorPools.Begin(); i.isNotEnd(); ++i)
        {
            VulkanTypedDescriptorPoolSet *typedPool = i->value;
            typedPool->reset();
        }
    }

    VulkanDescriptorPoolsManager::VulkanDescriptorPoolsManager(VulkanDevice *device)
        : m_device(device)
    {
    }

    VulkanDescriptorPoolsManager::~VulkanDescriptorPoolsManager()
    {
        m_poolSets.clearDelete();
    }

    VulkanDescriptorPoolSetContainer *VulkanDescriptorPoolsManager::acquirePoolSetContainer()
    {
        TkMutexGuard lock(m_mtx);
        const u64 currentFrame = m_device ? m_device->getFrameIndex() : 0;
        for (auto *poolSet : m_poolSets)
        {
            // Reuse only after GraphicsKit's own frame counter says the pool is old enough.
            // The Vulkan backend must stay decoupled from the higher-level Engine module.
            if (poolSet->refs == 0 && currentFrame - TkMath::min(currentFrame, poolSet->lastFrameUsed) > VK_RESOURCE_DELETE_SAFE_FRAMES_COUNT)
            {
                poolSet->lastFrameUsed = currentFrame;
                poolSet->reset();
                return poolSet;
            }
        }
        const auto poolSet = TkMemory::newObject<VulkanDescriptorPoolSetContainer>(TkMemory::getDefault(), m_device);
        m_poolSets.add(poolSet);
        return poolSet;
    }

    void VulkanDescriptorPoolsManager::collect()
    {
        TkMutexGuard lock(m_mtx);
        const u64 currentFrame = m_device ? m_device->getFrameIndex() : 0;
        for (i32 i = m_poolSets.count() - 1; i >= 0; i--)
        {
            const auto poolSet = m_poolSets[i];
            if (poolSet->refs == 0 && currentFrame - TkMath::min(currentFrame, poolSet->lastFrameUsed) > VK_RESOURCE_DELETE_SAFE_FRAMES_COUNT)
            {
                m_poolSets.removeAt(i);
                TkMemory::deletePtr(poolSet);
                break;
            }
        }
    }

    VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice *device, const VulkanDescriptorSetLayoutInfo &layout)
        : device(device), handle(VK_NULL_HANDLE), descriptorSetLayout(device)
    {
        descriptorSetLayout.copyFrom(layout);
        descriptorSetLayout.compile();

        VkPipelineLayoutCreateInfo createInfo;
        VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
        createInfo.setLayoutCount = descriptorSetLayout.handles.count();
        createInfo.pSetLayouts = descriptorSetLayout.handles.get();
        VALIDATE_VK_RESULT(vkCreatePipelineLayout(device->device, &createInfo, nullptr, &handle));
    }

    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        if (handle != VK_NULL_HANDLE)
            device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kPipelineLayout, handle);
    }

    u32 VulkanDescriptorSetWriter::setupDescriptorWrites(const SpirvShaderDescriptorInfo &info, VkWriteDescriptorSet *writeDescriptors, VkDescriptorImageInfo *imageInfo, VkDescriptorBufferInfo *bufferInfo, VkBufferView *texelBufferView, u8 *bindingToDynamicOffset)
    {
        TK_ASSERT(info.descriptorTypesCount <= SpirvShaderDescriptorInfo::kMaxDescriptors);
        this->writeDescriptors = writeDescriptors;
        writesCount = info.descriptorTypesCount;
        this->bindingToDynamicOffset = bindingToDynamicOffset;
        u32 dynamicOffsetIndex = 0;
        for (u32 i = 0; i < info.descriptorTypesCount; i++)
        {
            const auto &descriptor = info.descriptorTypes[i];
            writeDescriptors->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptors->dstBinding = i;
            writeDescriptors->descriptorCount = descriptor.count;
            writeDescriptors->descriptorType = descriptor.descriptorType;

            switch (writeDescriptors->descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                this->bindingToDynamicOffset[i] = dynamicOffsetIndex;
                dynamicOffsetIndex++;
                writeDescriptors->pBufferInfo = bufferInfo++;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                writeDescriptors->pBufferInfo = bufferInfo;
                bufferInfo += descriptor.count;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                writeDescriptors->pImageInfo = imageInfo;
                imageInfo += descriptor.count;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                writeDescriptors->pTexelBufferView = texelBufferView;
                texelBufferView += descriptor.count;
                break;
            default:
                TK_ASSERT(false && "setupDescriptorWrites");
                break;
            }

            writeDescriptors++;
        }
        return dynamicOffsetIndex;
    }

}

#endif
