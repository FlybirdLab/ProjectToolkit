/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/collection/Array.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    // Dynamic GPU buffer that allows to update and use GPU data (index/vertex/other) during single frame (supports dynamic resizing).
    class TK_API DynamicBuffer
    {
        NON_COPYABLE(DynamicBuffer);

    public:
        DynamicBuffer(u32 initialCapacity, u32 stride, const TkString &name);
        virtual ~DynamicBuffer();

    public:
        // GPU usage of the resource. Use Dynamic for resources that can be updated multiple timers per-frame.
        GfxResourceUsage usage = GfxResourceUsage::kDynamic;
        // The data container (raw bytes storage).
        TkArray<byte> data;

    public:
        // Get buffer (can be null due to 'late init' feature).
        TK_FORCE_INLINE GfxBuffer *getBuffer() const
        {
            return m_buffer;
        }

        // Clear data (begin for writing)
        TK_FORCE_INLINE void clear()
        {
            data.clear();
        }

        // Write bytes to the buffer
        template <typename T>
        TK_FORCE_INLINE void write(const T &data)
        {
            data.add((byte *)&data, sizeof(T));
        }

        // Write bytes to the buffer
        TK_FORCE_INLINE void write(const void *bytes, i32 size)
        {
            data.add((const byte *)bytes, size);
        }

        // Allocate bytes in the buffer by resizing the buffer for new memory and returns the pointer to the start of the allocated space.
        TK_FORCE_INLINE byte *writeReserve(i32 size)
        {
            const i32 start = data.count();
            data.addUninitialized(size);
            return data.get() + start;
        }

        // Allocate bytes in the buffer by resizing the buffer for new memory and returns the pointer to the start of the allocated space.
        template <typename T>
        TK_FORCE_INLINE T *writeReserve(i32 count)
        {
            return (T *)writeReserve(count * sizeof(T));
        }

        // Unlock buffer and flush data with a buffer (it will be ready for an immediate draw).
        void flush()
        {
            flush(nullptr);
        }
        // Unlock buffer and flush data with a buffer (it will be ready for a during next frame draw).
        void flush(class GfxContext *context);

        // Disposes the buffer resource and clears the used memory.
        void dispose();

    protected:
        virtual void initDesc(class GfxBufferDescription &desc, i32 numElements) = 0;

    protected:
        GfxBuffer *m_buffer;
        TkString m_name;
        u32 m_stride;
    };

    // Dynamic vertex buffer that allows to render any vertices during single frame (supports dynamic resizing)
    class TK_API DynamicVertexBuffer : public DynamicBuffer
    {
    public:
        DynamicVertexBuffer(u32 initialCapacity, u32 stride, const TkString &name = TkString::Empty, GfxVertexLayout *layout = nullptr)
            : DynamicBuffer(initialCapacity, stride, name), m_layout(layout)
        {
        }

        // Get the vertex buffer layout.
        GfxVertexLayout *getLayout() const;
        // Set the vertex buffer layout.
        void setLayout(GfxVertexLayout *layout);

    protected:
        void initDesc(GfxBufferDescription &desc, i32 numElements) override;

    private:
        GfxVertexLayout *m_layout;
    };

    // Dynamic index buffer that allows to render any indices during single frame (supports dynamic resizing)
    class TK_API DynamicIndexBuffer : public DynamicBuffer
    {
    public:
        DynamicIndexBuffer(u32 initialCapacity, u32 stride, const TkString &name = TkString::Empty)
            : DynamicBuffer(initialCapacity, stride, name)
        {
        }

    protected:
        void initDesc(GfxBufferDescription &desc, i32 numElements) override;
    };

    // Dynamic structured buffer that allows to upload data to the GPU from CPU (supports dynamic resizing).
    class TK_API DynamicStructuredBuffer : public DynamicBuffer
    {
    public:
        DynamicStructuredBuffer(u32 initialCapacity, u32 stride, bool isUnorderedAccess = false, const TkString &name = TkString::Empty);

    protected:
        void initDesc(GfxBufferDescription &desc, i32 numElements) override;

    private:
        bool m_isUnorderedAccess;
    };

    // Dynamic Typed buffer that allows to upload data to the GPU from CPU (supports dynamic resizing).
    class TK_API DynamicTypedBuffer : public DynamicBuffer
    {
    public:
        DynamicTypedBuffer(u32 initialCapacity, PixelFormat format, bool isUnorderedAccess = false, const TkString &name = TkString::Empty);

    protected:
        void initDesc(GfxBufferDescription &desc, i32 numElements) override;

    private:
        PixelFormat m_format;
        bool m_isUnorderedAccess;

    };

}