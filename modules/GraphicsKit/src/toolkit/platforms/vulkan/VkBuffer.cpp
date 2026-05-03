/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkBuffer.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/async/GfxTasks.hpp"
#include "toolkit/threading/Thread.hpp"

#if GFX_API_VULKAN

#include "VkUtils.hpp"
#include "VkContext.hpp"

namespace toolkit
{

    void VulkanBufferView::init(VulkanDevice *device, VulkanBuffer *owner, VkBuffer buffer, VkDeviceSize size, VkBufferUsageFlags usage, PixelFormat format)
    {
        TK_ASSERT(view == VK_NULL_HANDLE);

        m_parent = owner;
        this->device = device;
        this->owner = owner;
        this->buffer = buffer;
        this->size = size;

        if ((owner->isShaderResource() && !(owner->getDescription().flags & GfxBufferFlags::kStructured)) || (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) == VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
        {
            VkBufferViewCreateInfo viewInfo;
            VkUtils::zeroStruct(viewInfo, VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO);
            viewInfo.buffer = this->buffer;
            viewInfo.format = VkUtils::toVulkanFormat(format);
            viewInfo.offset = 0;
            viewInfo.range = this->size;
            if (viewInfo.format == VK_FORMAT_UNDEFINED)
                return; // Skip for structured buffers that use custom structure type and have unknown format
            VALIDATE_VK_RESULT(vkCreateBufferView(device->device, &viewInfo, nullptr, &this->view));
        }
    }

    void VulkanBufferView::release()
    {
        if (view != VK_NULL_HANDLE)
        {
            device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::kBufferView, view);
            view = VK_NULL_HANDLE;
        }
#if TK_DEBUG
        device = nullptr;
        owner = nullptr;
        buffer = VK_NULL_HANDLE;
#endif
    }

    void VulkanBufferView::descriptorAsUniformTexelBuffer(VulkanContext *context, VkBufferView &bufferView)
    {
        TK_ASSERT(view != VK_NULL_HANDLE);
        bufferView = view;
        context->addBufferBarrier(owner, VK_ACCESS_SHADER_READ_BIT);
    }

    void VulkanBufferView::descriptorAsStorageBuffer(VulkanContext *context, VkBuffer &buffer, VkDeviceSize &offset, VkDeviceSize &range)
    {
        TK_ASSERT(buffer);
        buffer = this->buffer;
        offset = 0;
        range = size;
        context->addBufferBarrier(owner, VK_ACCESS_SHADER_READ_BIT);
    }

    void VulkanBufferView::descriptorAsStorageTexelBuffer(VulkanContext *context, VkBufferView &bufferView)
    {
        TK_ASSERT(view != VK_NULL_HANDLE);
        bufferView = view;
        context->addBufferBarrier(owner, VK_ACCESS_SHADER_READ_BIT);
    }

    GfxBufferView *VulkanBuffer::view() const
    {
        return (GfxBufferView *)&m_view;
    }

    void *VulkanBuffer::map(GfxResourceMapMode mode)
    {
        void *mapped = nullptr;
        const VkResult result = vmaMapMemory(m_device->allocator, m_allocation, (void **)&mapped);
        LOG_VK_RESULT(result);
        return mapped;
    }

    void VulkanBuffer::unmap()
    {
        vmaUnmapMemory(m_device->allocator, m_allocation);
    }

    bool VulkanBuffer::onInit()
    {
        const bool useSRV = isShaderResource();
        const bool useUAV = isUnorderedAccess();

        // Setup buffer description
        VkBufferCreateInfo bufferInfo;
        VkUtils::zeroStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
        bufferInfo.size = m_desc.size;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (useSRV && !(m_desc.flags & GfxBufferFlags::kStructured))
            bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        if (useUAV || enumHasAnyFlags(m_desc.flags, GfxBufferFlags::kRawBuffer | GfxBufferFlags::kStructured))
            bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (useUAV && useSRV)
            bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        if (enumHasAnyFlags(m_desc.flags, GfxBufferFlags::kArgument))
            bufferInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        if (enumHasAnyFlags(m_desc.flags, GfxBufferFlags::kArgument) && useUAV)
            bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT; // For some reason, glslang marks indirect uav buffers (UpdateProbesInitArgs, IndirectArgsBuffer) as Storage Texel Buffers
        if (enumHasAnyFlags(m_desc.flags, GfxBufferFlags::kVertexBuffer))
            bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (enumHasAnyFlags(m_desc.flags, GfxBufferFlags::kIndexBuffer))
            bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (isStaging() || enumHasAnyFlags(m_desc.flags, GfxBufferFlags::kUnorderedAccess))
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        // Create buffer
        VmaAllocationCreateInfo allocInfo = {};
        switch (m_desc.usage)
        {
        case GfxResourceUsage::kDynamic:
            allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            break;
        case GfxResourceUsage::kStagingUpload:
            allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
            break;
        case GfxResourceUsage::kStagingReadback:
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
            break;
        case GfxResourceUsage::kStaging:
            allocInfo.usage = VMA_MEMORY_USAGE_CPU_COPY;
            break;
        default:
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        }
        const VkResult result = vmaCreateBuffer(m_device->allocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
        LOG_VK_RESULT_WITH_RETURN(result);
#if GFX_ENABLE_RESOURCE_NAMING
        VK_SET_DEBUG_NAME(m_device, m_buffer, VK_OBJECT_TYPE_BUFFER, getName());
#endif
        m_memoryUsage = m_desc.size;
        access = 0;

        // Check if set initial data
        if (m_desc.initData)
        {
            if (isDynamic() || isStaging())
            {
                // Faster path using Map/Unmap sequence
                setData(m_desc.initData, m_desc.size);
            }
            else if (m_device->isRendering() && TkThread::isInMainThread())
            {
                // Upload resource data now
                m_device->getMainContext()->updateBuffer(this, m_desc.initData, m_desc.size);
            }
            else
            {
                // Create async resource copy task
                auto copyTask = TkMemory::newObject<GfxUploadBufferTask>(TkMemory::getDefault(), this, 0, TkSpan<byte>((const byte *)m_desc.initData, m_desc.size), true);
                // TK_ASSERT(copyTask->hasReference(this));
                copyTask->start();
            }
        }

        // Check if need to use a counter
        if (enumHasAnyFlags(m_desc.flags, GfxBufferFlags::kCounter | GfxBufferFlags::kAppend))
        {
#if GFX_ENABLE_RESOURCE_NAMING
            TkString name = TkString(getName()) + ".Counter";
            counter = TkMemory::newObject<VulkanBuffer>(TkMemory::getDefault(), m_device, name);
#else
            counter = TkMemory::newObject<VulkanBuffer>(TkMemory::getDefault(),m_device, TkStringView::Empty);
#endif
            if (!counter->init(GfxBufferDescription::raw(4, GfxBufferFlags::kUnorderedAccess)))
            {
                LogError("Cannot create counter buffer.");
                return false;
            }
        }
        // Check if need to bind buffer to the shaders
        else if (useSRV || useUAV)
        {
            // Create buffer view
            m_view.init(m_device, this, m_buffer, m_desc.size, bufferInfo.usage, m_desc.format);
        }

        return true;
    }

    void VulkanBuffer::onReleaseData()
    {
        m_view.release();
        GFX_DEL_RESOURCE(counter);
        if (m_allocation != VK_NULL_HANDLE)
        {
            m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::kBuffer, m_buffer, m_allocation);
            m_buffer = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
        }

        // Base
        GfxBuffer::onReleaseData();
    }

}

#endif