/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <toolkit/io/Stream.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit {

    // Super fast advanced data reading from raw bytes without any overhead at all
    class MemoryIStream : public ITkStream
    {
    public:
        MemoryIStream();
        MemoryIStream(const byte* bytes, u32 length);
        template<typename T>
        MemoryIStream(const TkArray<T>& data)
            : MemoryIStream(data.get(), data.count() * sizeof(T))
        {
        }

    public:
        // Init stream to the custom buffer location
        void init(const byte* bytes, u32 length);

        // Init stream to the custom buffer location
        template<typename T>
        TK_FORCE_INLINE void init(const TkArray<T>& data)
        {
            init(data.get(), data.count() * sizeof(T));
        }

        // Get the current handle to position in buffer.
        const byte* getPositionHandle() const
        {
            return m_position;
        }

    public:
        // Skip the data from the target buffer without reading from it. Moves the read pointer in the buffer forward.
        void* move(u32 bytes)
        {
            TK_ASSERT(length() - pos() >= bytes);
            const auto result = (void*)m_position;
            m_position += bytes;
            return result;
        }

        // Skip the data from the target buffer without reading from it. Moves the read pointer in the buffer forward.
        template<typename T>
        TK_FORCE_INLINE T* move()
        {
            return static_cast<T*>(move(sizeof(T)));
        }

        // Skip the data from the target buffer without reading from it. Moves the read pointer in the buffer forward.
        template<typename T>
        TK_FORCE_INLINE T* move(u32 count)
        {
            return static_cast<T*>(move(sizeof(T) * count));
        }

    public:
        void flush() override;
        void close() override;
        u32 length() override;
        u32 pos() override;
        void setPos(u32 seek) override;
        void readBytes(void* data, u32 bytes) override;

    private:
        const byte* m_buffer;
        const byte* m_position;
        u32 m_length;
    };


    // Implementation of of the stream that can be used for fast data writing to the memory.
    class MemoryOStream : public OTkStream
    {
    public:
        MemoryOStream();
        MemoryOStream(u32 capacity);
        ~MemoryOStream();

    public:
        // Get buffer handle
        TK_FORCE_INLINE byte* getHandle() const
        {
            return m_buffer;
        }

        // Get current capacity of the memory stream
        TK_FORCE_INLINE u32 getCapacity() const
        {
            return m_capacity;
        }

        // Get current stream length (capacity in bytes)
        TK_FORCE_INLINE u32 getLength() const
        {
            return m_capacity;
        }

        // Get current position in the stream (in bytes)
        TK_FORCE_INLINE u32 getPosition() const
        {
            return static_cast<u32>(m_position - m_buffer);
        }

        // Skip bytes from the target buffer without writing to it. moves the write pointer in the buffer forward.
        void* move(u32 bytes);

        // Skip the data from the target buffer without writing to it. moves the write pointer in the buffer forward.
        template<typename T>
        TK_FORCE_INLINE T* move()
        {
            return static_cast<T*>(move(sizeof(T)));
        }

        // Skip the data from the target buffer without writing to it. moves the write pointer in the buffer forward.
        template<typename T>
        TK_FORCE_INLINE T* move(u32 count)
        {
            return static_cast<T*>(move(sizeof(T) * count));
        }

    public:
        // Cleanups the buffers, resets the position and allocated the new memory chunk.
        void reset(u32 capacity);
        // Saves current buffer contents to the file
        bool saveToFile(const TkStringView& path) const;

    public:
        // [OStream]
        void flush() override;
        void close() override;
        u32 length() override;
        u32 pos() override;
        void setPos(u32 seek) override;
        void writeBytes(const void* data, u32 bytes) override;

    private:
        byte* m_buffer;
        byte* m_position;
        u32 m_capacity;
    };
    
}