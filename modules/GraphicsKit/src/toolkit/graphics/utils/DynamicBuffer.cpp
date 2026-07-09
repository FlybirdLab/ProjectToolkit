/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "DynamicBuffer.hpp"
#include "../GfxManager.hpp"
#include "../GfxDevice.hpp"
#include "../GfxContext.hpp"
#include "PixelFormat.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/utils/Convertor.hpp"

namespace toolkit
{

    DynamicBuffer::DynamicBuffer(u32 initialCapacity, u32 stride, const TkString &name)
        : m_buffer(nullptr), m_name(name), m_stride(stride), data(initialCapacity)
    {
    }

    DynamicBuffer::~DynamicBuffer()
    {
        GFX_DEL_RESOURCE(m_buffer);
    }

    void DynamicBuffer::flush(GfxContext *context)
    {
        const u32 size = data.count();
        if (size == 0)
            return;

        // Lazy-resize buffer
        if (m_buffer == nullptr)
            m_buffer = GfxDevice::sInstance->createBuffer(m_name);
        if (m_buffer->getSize() < size || m_buffer->getDescription().usage != usage)
        {
            const i32 numElements = TkMath::alignUp<i32>(static_cast<i32>((size / m_stride) * 1.3f), 32);
            GfxBufferDescription desc;
            initDesc(desc, numElements);
            desc.usage = usage;
            if (!m_buffer->init(desc))
            {
                LogError("Cannot setup dynamic buffer '%i'! Size: %s", m_name, TkConvertor::bytesToText(size));
                return;
            }
        }

        // Upload data to the buffer
        if (context)
        {
            context->updateBuffer(m_buffer, data.get(), size);
        }
        else if (GfxDevice::sInstance->isRendering())
        {
            GfxManager::sharedMutex.lock();
            GfxDevice::sInstance->getMainContext()->updateBuffer(m_buffer, data.get(), size);
            GfxManager::sharedMutex.unlock();
        }
        else
        {
            m_buffer->setData(data.get(), size);
        }
    }

    void DynamicBuffer::dispose()
    {
        GFX_DEL_RESOURCE(m_buffer);
        data.resize(0);
    }

    GfxVertexLayout *DynamicVertexBuffer::getLayout() const
    {
        return m_layout ? m_layout : (getBuffer() ? getBuffer()->getVertexLayout() : nullptr);
    }

    void DynamicVertexBuffer::setLayout(GfxVertexLayout *layout)
    {
        m_layout = layout;
        GFX_DEL_RESOURCE(m_buffer);
    }

    void DynamicVertexBuffer::initDesc(GfxBufferDescription &desc, i32 numElements)
    {
        desc = GfxBufferDescription::vertex(m_layout, m_stride, numElements, GfxResourceUsage::kDynamic);
    }

    void DynamicIndexBuffer::initDesc(GfxBufferDescription &desc, i32 numElements)
    {
        desc = GfxBufferDescription::index(m_stride, numElements, GfxResourceUsage::kDynamic);    }

    DynamicStructuredBuffer::DynamicStructuredBuffer(u32 initialCapacity, u32 stride, bool isUnorderedAccess, const TkString &name)
        : DynamicBuffer(initialCapacity, stride, name), m_isUnorderedAccess(isUnorderedAccess)
    {
        usage = GfxResourceUsage::kDefault; // The most common use-case is just for a single upload of data prepared by CPU
    }

    void DynamicStructuredBuffer::initDesc(GfxBufferDescription &desc, i32 numElements)
    {
        desc = GfxBufferDescription::structured(numElements, m_stride, m_isUnorderedAccess);
    }

    DynamicTypedBuffer::DynamicTypedBuffer(u32 initialCapacity, PixelFormat format, bool isUnorderedAccess, const TkString &name)
        : DynamicBuffer(initialCapacity, GfxPixelFormat::sizeInBytes(format), name), m_format(format), m_isUnorderedAccess(isUnorderedAccess)
    {
        usage = GfxResourceUsage::kDefault; // The most common use-case is just for a single upload of data prepared by CPU
    }

    void DynamicTypedBuffer::initDesc(GfxBufferDescription &desc, i32 numElements)
    {
        auto bufferFlags = GfxBufferFlags::kShaderResource;
        if (m_isUnorderedAccess)
            bufferFlags |= GfxBufferFlags::kUnorderedAccess;
        desc = GfxBufferDescription::buffer(numElements * m_stride, bufferFlags, m_format, nullptr, m_stride);
    }

}