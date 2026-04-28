/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>
#include <toolkit/graphics/utils/PixelFormat.hpp>
#include <toolkit/graphics/GfxResource.hpp>

namespace toolkit
{
    typedef TkDataCollection<byte> ByteCollection;
    class TkTask;

    // A common description for all GPU buffers.
    struct TK_API GfxBufferDescription
    {
    public:
        // The buffer total size.
        u32 size;
        // The buffer structure stride (size in bytes per element).
        u32 stride;
        // The buffer flags.
        GfxBufferFlags flags;
        // The format of the data in a buffer.
        PixelFormat format;
        // The pointer to location of initial resource data. Null if not used.
        const void *initData;
        // Value that identifies how the buffer is to be read from and written to. The most common value is <see cref="GfxResourceUsage.Default"/>; see <strong><see cref="GfxResourceUsage"/></strong> for all possible values.
        GfxResourceUsage usage;
        // The vertex elements layout used by vertex buffers only.
        GfxVertexLayout *vertexLayout;

    public:
        // Get the number elements in the buffer.
        TK_FORCE_INLINE u32 getElementsCount() const
        {
            return stride > 0 ? size / stride : 0;
        }

        // Get a value indicating whether this instance is a shader resource.
        TK_FORCE_INLINE bool isShaderResource() const
        {
            return (flags & GfxBufferFlags::kShaderResource) != GfxBufferFlags::kNone;
        }

        // Get a value indicating whether this instance is a unordered access.
        TK_FORCE_INLINE bool isUnorderedAccess() const
        {
            return (flags & GfxBufferFlags::kUnorderedAccess) != GfxBufferFlags::kNone;
        }

    public:
        // Create the buffer description.
        static GfxBufferDescription buffer(u32 size, GfxBufferFlags flags, PixelFormat format = PixelFormat::kUnknown, const void *initData = nullptr, u32 stride = 0, GfxResourceUsage usage = GfxResourceUsage::kDefault);

        // Create typed buffer description.
        static GfxBufferDescription typed(i32 count, PixelFormat viewFormat, bool isUnorderedAccess = false, GfxResourceUsage usage = GfxResourceUsage::kDefault);
        // Create typed buffer description.
        static GfxBufferDescription typed(const void *data, i32 count, PixelFormat viewFormat, bool isUnorderedAccess = false, GfxResourceUsage usage = GfxResourceUsage::kDefault);

        // Create vertex buffer description.
        static GfxBufferDescription vertex(GfxVertexLayout *layout, u32 elementStride, u32 elementsCount, const void *data);
        // Create vertex buffer description.
        static GfxBufferDescription vertex(GfxVertexLayout *layout, u32 elementStride, u32 elementsCount, GfxResourceUsage usage = GfxResourceUsage::kDefault);
        // Create vertex buffer description.
        static GfxBufferDescription vertex(GfxVertexLayout *layout, u32 elementsCount, const void *data);
        // Create vertex buffer description.
        static GfxBufferDescription vertex(GfxVertexLayout *layout, u32 elementsCount, GfxResourceUsage usage = GfxResourceUsage::kDefault);

        // Create index buffer description.
        static GfxBufferDescription index(i32 elementStride, i32 elementsCount, const void *data)
        {
            const auto format = elementStride == 4 ? PixelFormat::kR32_UInt : PixelFormat::kR16_UInt;
            return buffer(elementsCount * elementStride, GfxBufferFlags::kIndexBuffer, format, data, elementStride, GfxResourceUsage::kDefault);
        }
        // Create index buffer description.
        static GfxBufferDescription index(i32 elementStride, i32 elementsCount, GfxResourceUsage usage = GfxResourceUsage::kDefault)
        {
            const auto format = elementStride == 4 ? PixelFormat::kR32_UInt : PixelFormat::kR16_UInt;
            return buffer(elementsCount * elementStride, GfxBufferFlags::kIndexBuffer, format, nullptr, elementStride, usage);
        }

        // Create structured buffer description.
        static GfxBufferDescription structured(i32 elementCount, i32 elementSize, bool isUnorderedAccess = false)
        {
            auto bufferFlags = GfxBufferFlags::kStructured | GfxBufferFlags::kShaderResource;
            if (isUnorderedAccess)
                bufferFlags |= GfxBufferFlags::kUnorderedAccess;
            return buffer(elementCount * elementSize, bufferFlags, PixelFormat::kUnknown, nullptr, elementSize);
        }
        // Create append buffer description (structured buffer).
        static GfxBufferDescription structuredAppend(i32 elementCount, i32 elementSize)
        {
            return buffer(elementCount * elementSize, GfxBufferFlags::kStructuredAppendBuffer | GfxBufferFlags::kShaderResource, PixelFormat::kUnknown, nullptr, elementSize);
        }
        // Create counter buffer description (structured buffer).
        static GfxBufferDescription structuredCounter(i32 elementCount, i32 elementSize)
        {
            return buffer(elementCount * elementSize, GfxBufferFlags::kStructuredCounterBuffer | GfxBufferFlags::kShaderResource, PixelFormat::kUnknown, nullptr, elementSize);
        }

        // Create argument buffer description.
        static GfxBufferDescription argument(i32 size, GfxResourceUsage usage = GfxResourceUsage::kDefault)
        {
            return buffer(size, GfxBufferFlags::kArgument, PixelFormat::kUnknown, nullptr, 0, usage);
        }
        // Create argument buffer description.
        static GfxBufferDescription argument(const void *data, i32 size, GfxResourceUsage usage = GfxResourceUsage::kDefault)
        {
            return buffer(size, GfxBufferFlags::kArgument, PixelFormat::kUnknown, data, 0, usage);
        }

        // Create raw buffer description.
        static GfxBufferDescription raw(i32 size, GfxBufferFlags additionalFlags = GfxBufferFlags::kNone, GfxResourceUsage usage = GfxResourceUsage::kDefault)
        {
            return buffer(size, GfxBufferFlags::kRawBuffer | additionalFlags, PixelFormat::kR32_Typeless, nullptr, sizeof(float), usage);
        }
        // Create raw buffer description.
        static GfxBufferDescription Raw(const void *data, i32 size, GfxBufferFlags additionalFlags = GfxBufferFlags::kNone, GfxResourceUsage usage = GfxResourceUsage::kDefault)
        {
            return buffer(size, GfxBufferFlags::kRawBuffer | additionalFlags, PixelFormat::kR32_Typeless, data, sizeof(float), usage);
        }

    public:
        void clear();
        GfxBufferDescription toStagingUpload() const;
        GfxBufferDescription toStagingReadback() const;
        GfxBufferDescription toStaging() const;
        bool equals(const GfxBufferDescription &other) const;
        TkString toString() const;

    public:
        TK_FORCE_INLINE bool operator==(const GfxBufferDescription &other) const
        {
            return equals(other);
        }
        TK_FORCE_INLINE bool operator!=(const GfxBufferDescription &other) const
        {
            return !equals(other);
        }
    };

    u32 getHash(const GfxBufferDescription &key);
    template <>
    struct HashFunc<GfxBufferDescription>
    {
        TK_FORCE_INLINE static u32 get(const GfxBufferDescription &k)
        {
            return getHash(k);
        }
    };

    // Defines a view for the. Used to bind buffer to the shaders (for input as shader resource or for input/output as unordered access).
    class TK_API GfxBufferView : public GfxResourceView
    {
    protected:
        GfxBufferView();
    };

    // All-in-one GPU buffer class. This class is able to create index buffers, vertex buffers, structured buffer and argument buffers.
    class TK_API GfxBuffer : public GfxResource
    {
        static GfxBuffer *newBuffer();

    protected:
        GfxBuffer();

    public:
        // Get a value indicating whether this buffer has been allocated.
        TK_FORCE_INLINE bool isAllocated() const
        {
            return m_desc.size > 0;
        }
        // Get buffer size in bytes.
        TK_FORCE_INLINE u32 getSize() const
        {
            return m_desc.size;
        }
        // Get buffer stride in bytes.
        TK_FORCE_INLINE u32 getStride() const
        {
            return m_desc.stride;
        }
        // Get buffer data format (if used).
        TK_FORCE_INLINE PixelFormat getFormat() const
        {
            return m_desc.format;
        }
        // Get buffer elements count (size divided by the stride).
        TK_FORCE_INLINE u32 getElementsCount() const
        {
            return m_desc.stride > 0 ? m_desc.size / m_desc.stride : 0;
        }
        // Get buffer flags.
        TK_FORCE_INLINE GfxBufferFlags getFlags() const
        {
            return m_desc.flags;
        }
        // Get vertex elements layout used by vertex buffers only.
        TK_FORCE_INLINE GfxVertexLayout *getVertexLayout() const
        {
            return m_desc.vertexLayout;
        }

        // Checks if buffer is a staging buffer (supports CPU access).
        bool isStaging() const;
        // Checks if buffer is a dynamic buffer.
        bool isDynamic() const;

        // Get a value indicating whether this buffer is a shader resource.
        TK_FORCE_INLINE bool isShaderResource() const
        {
            return m_desc.isShaderResource();
        }
        // Get a value indicating whether this buffer is a unordered access.
        TK_FORCE_INLINE bool isUnorderedAccess() const
        {
            return m_desc.isUnorderedAccess();
        }

        // Get buffer description structure.
        const GfxBufferDescription &getDescription() const
        {
            return m_desc;
        }
        // Get the view for the whole buffer.
        virtual GfxBufferView *view() const = 0;

    public:
        // Create new buffer.
        bool init(const GfxBufferDescription &desc);

        // Create new staging readback buffer with the same dimensions and properties as a source buffer (but without a data transferred; warning: caller must delete object).
        GfxBuffer *toStagingReadback() const;
        // Create new staging upload buffer with the same dimensions and properties as a source buffer (but without a data transferred; warning: caller must delete object).
        GfxBuffer *toStagingUpload() const;

        // Tries to resize the buffer (warning: contents will be lost).
        bool resize(u32 newSize);

    public:
        // Stops current thread execution to gather buffer data from the GPU. Cannot be called from main thread if the buffer is not a dynamic nor staging readback.
        bool downloadData(ByteCollection &result);
        // Create GPU async task that will gather buffer data from the GPU.
        TkTask *downloadDataAsync(ByteCollection &result);

        // Get the buffer data via map/memcpy/unmap sequence. Always supported for dynamic and staging buffers (other types support depends on graphics backend implementation).
        bool getData(ByteCollection &output);
        // Sets the buffer data via map/memcpy/unmap sequence. Always supported for dynamic buffers (other types support depends on graphics backend implementation).
        void setData(const void *data, u32 size);

        // Get a CPU pointer to the resource by mapping its contents. Denies the GPU access to that resource.
        virtual void *map(GfxResourceMapMode mode) = 0;
        // Invalidates the mapped pointer to a resource and restores the GPU's access to that resource.
        virtual void unmap() = 0;

    public:
        GfxResType getType() const final override;
        void onReleaseData() override;

    protected:
        virtual bool onInit() = 0;

    protected:
        GfxBufferDescription m_desc;
        bool m_isLocked = false;
    };

}