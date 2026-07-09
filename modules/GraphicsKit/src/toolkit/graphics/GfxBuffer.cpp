/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxBuffer.hpp"
#include "GfxManager.hpp"
#include "GfxVertexLayout.hpp"
#include "async/GfxTasks.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/threading/Task.hpp"
#include "toolkit/threading/Thread.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/collection/DataCollection.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "GfxDevice.hpp"

namespace toolkit
{

    GfxBufferDescription GfxBufferDescription::buffer(u32 size, GfxBufferFlags flags, PixelFormat format, const void *initData, u32 stride, GfxResourceUsage usage)
    {
        GfxBufferDescription desc;
        desc.size = size;
        desc.stride = stride;
        desc.flags = flags;
        desc.format = format;
        desc.initData = initData;
        desc.usage = usage;
        desc.vertexLayout = nullptr;
        return desc;
    }

    GfxBufferDescription GfxBufferDescription::typed(i32 count, PixelFormat viewFormat, bool isUnorderedAccess, GfxResourceUsage usage)
    {
        auto bufferFlags = GfxBufferFlags::kShaderResource;
        if (isUnorderedAccess)
            bufferFlags |= GfxBufferFlags::kUnorderedAccess;
        const auto stride = GfxPixelFormat::sizeInBytes(viewFormat);
        return buffer(count * stride, bufferFlags, viewFormat, nullptr, stride, usage);
    }

    GfxBufferDescription GfxBufferDescription::typed(const void *data, i32 count, PixelFormat viewFormat, bool isUnorderedAccess, GfxResourceUsage usage)
    {
        auto bufferFlags = GfxBufferFlags::kShaderResource;
        if (isUnorderedAccess)
            bufferFlags |= GfxBufferFlags::kUnorderedAccess;
        const auto stride = GfxPixelFormat::sizeInBytes(viewFormat);
        return buffer(count * stride, bufferFlags, viewFormat, data, stride, usage);
    }

    GfxBufferDescription GfxBufferDescription::vertex(GfxVertexLayout *layout, u32 elementStride, u32 elementsCount, const void *data)
    {
        GfxBufferDescription desc;
        desc.size = elementsCount * elementStride;
        desc.stride = elementStride;
        desc.flags = GfxBufferFlags::kVertexBuffer;
        desc.format = PixelFormat::kUnknown;
        desc.initData = data;
        desc.usage = GfxResourceUsage::kDefault;
        desc.vertexLayout = layout;
        return desc;
    }

    GfxBufferDescription GfxBufferDescription::vertex(GfxVertexLayout *layout, u32 elementStride, u32 elementsCount, GfxResourceUsage usage)
    {
        GfxBufferDescription desc;
        desc.size = elementsCount * elementStride;
        desc.stride = elementStride;
        desc.flags = GfxBufferFlags::kVertexBuffer;
        desc.format = PixelFormat::kUnknown;
        desc.initData = nullptr;
        desc.usage = GfxResourceUsage::kDefault;
        desc.vertexLayout = layout;
        return desc;
    }

    GfxBufferDescription GfxBufferDescription::vertex(GfxVertexLayout *layout, u32 elementsCount, const void *data)
    {
        const u32 stride = layout ? layout->getStride() : 0;
        TK_CHECK_RETURN_DEBUG(stride, GfxBufferDescription());
        return vertex(layout, stride, elementsCount, data);
    }

    GfxBufferDescription GfxBufferDescription::vertex(GfxVertexLayout *layout, u32 elementsCount, GfxResourceUsage usage)
    {
        const u32 stride = layout ? layout->getStride() : 0;
        TK_CHECK_RETURN_DEBUG(stride, GfxBufferDescription());
        return vertex(layout, stride, elementsCount, usage);
    }

    void GfxBufferDescription::clear()
    {
        TkMemory::memoryClear(this, sizeof(GfxBufferDescription));
    }

    GfxBufferDescription GfxBufferDescription::toStagingUpload() const
    {
        auto desc = *this;
        desc.usage = GfxResourceUsage::kStagingUpload;
        desc.flags = GfxBufferFlags::kNone;
        desc.initData = nullptr;
        return desc;
    }

    GfxBufferDescription GfxBufferDescription::toStagingReadback() const
    {
        auto desc = *this;
        desc.usage = GfxResourceUsage::kStagingReadback;
        desc.flags = GfxBufferFlags::kNone;
        desc.initData = nullptr;
        return desc;
    }

    GfxBufferDescription GfxBufferDescription::toStaging() const
    {
        auto desc = *this;
        desc.usage = GfxResourceUsage::kStaging;
        desc.flags = GfxBufferFlags::kNone;
        desc.initData = nullptr;
        return desc;
    }

    bool GfxBufferDescription::equals(const GfxBufferDescription &other) const
    {
        return size == other.size && stride == other.stride && flags == other.flags && format == other.format && usage == other.usage && initData == other.initData && vertexLayout == other.vertexLayout;
    }

    TkString GfxBufferDescription::toString() const
    {
        return TkString::format("Size: %i, Stride: %i, Flags: %i, Format: %i, Usage: %i",
                              size,
                              stride,
                              (i32)flags,
                              (i32)format,
                              (i32)usage);
    }

    u32 getHash(const GfxBufferDescription &key)
    {
        u32 hashCode = key.size;
        hashCode = (hashCode * 397) ^ key.stride;
        hashCode = (hashCode * 397) ^ (u32)key.flags;
        hashCode = (hashCode * 397) ^ (u32)key.format;
        hashCode = (hashCode * 397) ^ (u32)key.usage;
        hashCode = (hashCode * 397) ^ HashFunc<void *>::get(key.vertexLayout);
        return hashCode;
    }

    GfxBufferView::GfxBufferView()
        : GfxResourceView {}
    {
    }

    GfxBuffer *GfxBuffer::newBuffer()
    {
        return GfxDevice::sInstance->createBuffer(TkString::Empty);
    }

    GfxBuffer::GfxBuffer()
        : GfxResource {}
    {
        // Buffer with size 0 is considered to be invalid
        m_desc.size = 0;
    }

    bool GfxBuffer::isStaging() const
    {
        return m_desc.usage == GfxResourceUsage::kStagingReadback || m_desc.usage == GfxResourceUsage::kStagingUpload || m_desc.usage == GfxResourceUsage::kStaging;
    }

    bool GfxBuffer::isDynamic() const
    {
        return m_desc.usage == GfxResourceUsage::kDynamic;
    }

    bool GfxBuffer::init(const GfxBufferDescription &desc)
    {
        // Validate description
#if TK_DEBUG
#define GET_NAME() getName()
#else
#define GET_NAME() ""
#endif
        if (!TkMath::isInRange<u32>(desc.size, 1, kMaxI32))
        {
            LogWarn("Cannot create buffer '%s'. Incorrect size %i.", GET_NAME(), desc.size);
            return false;
        }
        if (!TkMath::isInRange<u32>(desc.stride, 0, 1024))
        {
            LogWarn("Cannot create buffer '%s'. Incorrect stride %i.", GET_NAME(), desc.stride);
            return false;
        }
        if (toolkit::enumHasAnyFlags(desc.flags, GfxBufferFlags::kStructured))
        {
            if (desc.stride <= 0)
            {
                LogWarn("Cannot create buffer '%s'. Element size cannot be less or equal 0 for structured buffer.", GET_NAME());
                return false;
            }
        }
        if (toolkit::enumHasAnyFlags(desc.flags, GfxBufferFlags::kRawBuffer))
        {
            if (desc.format != PixelFormat::kR32_Typeless)
            {
                LogWarn("Cannot create buffer '%s'. Raw buffers must use format R32_Typeless.", GET_NAME());
                return false;
            }
        }
        if (toolkit::enumHasAnyFlags(desc.flags, GfxBufferFlags::kVertexBuffer))
        {
            if (desc.vertexLayout == nullptr)
            {
                // [Deprecated in v1.10] Change this into an error as VertexLayout becomes a requirement when layout is no longer set in a vertex shader
                LogWarn("Missing Vertex Layout in buffer '%s'. Vertex Buffers should provide layout information about contained vertex elements.", GET_NAME());
            }
        }
#undef GET_NAME

        // Release previous data
        releaseData();

        // Initialize
        m_desc = desc;
        if (!onInit())
        {
            releaseData();
            LogWarn("Cannot initialize buffer. Description: %s", desc.toString());
            return false;
        }

        return true;
    }

    GfxBuffer *GfxBuffer::toStagingReadback() const
    {
        const auto desc = m_desc.toStagingReadback();
        TkMutexGuard gpuLock(GfxDevice::sInstance->mtx);
        auto *staging = GfxDevice::sInstance->createBuffer("Staging.Readback");
        if (!staging->init(desc))
        {
            staging->releaseData();
            TkMemory::deletePtr(staging);
            return nullptr;
        }
        return staging;
    }

    GfxBuffer *GfxBuffer::toStagingUpload() const
    {
        const auto desc = m_desc.toStagingUpload();
        TkMutexGuard gpuLock(GfxDevice::sInstance->mtx);
        auto *staging = GfxDevice::sInstance->createBuffer("Staging.Upload");
        if (!staging->init(desc))
        {
            staging->releaseData();
            TkMemory::deletePtr(staging);
            return nullptr;
        }
        return staging;
    }

    bool GfxBuffer::resize(u32 newSize)
    {
        PROFILE();
        if (!isAllocated())
        {
            LogError("InvalidOperationException Buffer.Resize");
            return false;
        }

        // Setup new description
        auto desc = m_desc;
        desc.size = newSize;
        desc.initData = nullptr;

        // Recreate
        return init(desc);
    }

    bool GfxBuffer::downloadData(ByteCollection &result)
    {
        // Skip for empty ones
        if (getSize() == 0)
        {
            LogWarn("Cannot download GPU buffer data from an empty buffer.");
            return false;
        }
        if (m_desc.usage == GfxResourceUsage::kStagingReadback || m_desc.usage == GfxResourceUsage::kDynamic || m_desc.usage == GfxResourceUsage::kStaging)
        {
            // Use faster path for staging resources
            return getData(result);
        }
        PROFILE();

        // Ensure not running on main thread
        if (TkThread::isInMainThread())
        {
            // TODO: support mesh data download from GPU on a main thread during rendering
            LogWarn("Cannot download GPU buffer data on a main thread. Use staging readback buffer or invoke this function from another thread.");
            return false;
        }

        // Create async task
        auto task = downloadDataAsync(result);
        if (task == nullptr)
        {
            LogWarn("Cannot create async download task for resource %i.", getType());
            return false;
        }

        // Wait for work to be done
        task->start();
        const bool failed = task->wait();
        task->destroy();
        if (failed)
        {
            LogWarn("Resource \'%i\' copy failed.", getType());
            return false;
        }

        return true;
    }

    class BufferDownloadDataTask : public ThreadPoolTask
    {
    private:
        GfxBufferReference m_buffer;
        GfxBuffer *m_staging;
        ByteCollection &m_data;

    public:
        BufferDownloadDataTask(GfxBuffer *buffer, GfxBuffer *staging, ByteCollection &data)
            : m_buffer(buffer), m_staging(staging), m_data(data)
        {
        }

        ~BufferDownloadDataTask()
        {
            GFX_DEL_RESOURCE(m_staging);
        }

    protected:
        // [ThreadPoolTask]
        bool run() override
        {
            // Check resources
            const auto buffer = m_buffer.get();
            if (buffer == nullptr || m_staging == nullptr)
            {
                LogWarn("Cannot download buffer data. Missing objects.");
                return false;
            }

            // Gather data
            if (m_staging->getData(m_data))
            {
                LogWarn("Staging resource of \'%i\' get data failed.", buffer->getType());
                return false;
            }

            return true;
        }

        void onEnd() override
        {
            m_buffer.unlink();

            ThreadPoolTask::onEnd();
        }
    };

    TkTask *GfxBuffer::downloadDataAsync(ByteCollection &result)
    {
        // Skip for empty ones
        if (getSize() == 0)
            return nullptr;

        // Create the staging resource
        const auto staging = toStagingReadback();
        if (staging == nullptr)
        {
            LogWarn("Cannot create staging resource from %s.", toString());
            return nullptr;
        }

        // Create async resource copy task
        auto copyTask = TkMemory::newObject<GfxCopyResourceTask>(TkMemory::getDefault(), this, staging);
        // TK_ASSERT(copyTask->hasReference(this) && copyTask->HasReference(staging));

        // Create task to copy downloaded data to ByteCollection
        const auto getDataTask = TkMemory::newObject<BufferDownloadDataTask>(TkMemory::getDefault(), this, staging, result);
        // ASSERT(getDataTask->HasReference(this) && getDataTask->HasReference(staging));

        // Set continuation
        copyTask->continueWith(getDataTask);
        copyTask->setAutoDelete(false);

        return copyTask;
    }

    bool GfxBuffer::getData(ByteCollection &output)
    {
        PROFILE();
        void *mapped = map(GfxResourceMapMode::kRead);
        if (!mapped)
            return false;
        output.copy((byte *)mapped, getSize());
        unmap();
        return true;
    }

    void GfxBuffer::setData(const void *data, u32 size)
    {
        PROFILE();
        if (size == 0 || data == nullptr)
        {
            LogError("ArgumentNullException Buffer.SetData");
            return;
        }
        if (size > getSize())
        {
            LogError("ArgumentOutOfRangeException Buffer.SetData");
            return;
        }

        if (m_desc.usage == GfxResourceUsage::kDefault && GfxDevice::sInstance->isRendering())
        {
            // Upload using the context (will use internal staging buffer inside command buffer)
            GfxManager::sharedMutex.lock();
            GfxDevice::sInstance->getMainContext()->updateBuffer(this, data, size);
            GfxManager::sharedMutex.unlock();
            return;
        }

        void *mapped = map(GfxResourceMapMode::kWrite);
        if (!mapped)
            return;
        TkMemory::memoryCopy(mapped, data, size);
        unmap();
    }

    GfxResType GfxBuffer::getType() const
    {
        return GfxResType::kBuffer;
    }

    void GfxBuffer::onReleaseData()
    {
        m_desc.clear();
        m_isLocked = false;
    }

}
