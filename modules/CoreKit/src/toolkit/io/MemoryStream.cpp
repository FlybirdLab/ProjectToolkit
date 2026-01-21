/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "MemoryStream.hpp"

#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/FileSystem.hpp"

namespace toolkit
{

    MemoryIStream::MemoryIStream()
        : m_buffer(nullptr), m_position(nullptr), m_length(0)
    {
    }

    MemoryIStream::MemoryIStream(const byte *bytes, u32 length)
        : m_buffer(bytes), m_position(bytes), m_length(length)
    {
    }

    void MemoryIStream::init(const byte *bytes, u32 length)
    {
        m_buffer = m_position = bytes;
        m_length = length;
    }

    void MemoryIStream::flush()
    {
    }

    void MemoryIStream::close()
    {
        m_position = m_buffer = nullptr;
        m_length = 0;
    }

    u32 MemoryIStream::length()
    {
        return m_length;
    }

    u32 MemoryIStream::pos()
    {
        return static_cast<u32>(m_position - m_buffer);
    }

    void MemoryIStream::setPos(u32 seek)
    {
        TK_ASSERT(m_length > 0);
        m_position = m_buffer + seek;
    }

    void MemoryIStream::readBytes(void *data, u32 bytes)
    {
        if (bytes > 0)
        {
            TK_ASSERT(data && length() - pos() >= bytes);
            TkMemory::memoryCopy(data, m_position, bytes);
            m_position += bytes;
        }
    }

    MemoryOStream::MemoryOStream()
        : m_buffer(nullptr), m_position(nullptr), m_capacity(0)
    {
    }

    MemoryOStream::MemoryOStream(u32 capacity)
        : m_capacity(capacity)
    {
        if (capacity > 0)
        {
            m_buffer = (byte *)TkMemory::allocate(TkMemory::getDefault(), capacity);
            if (m_buffer == nullptr)
            {
                TK_ASSERT(false && "OutOfMemory");
            }
        }
        else
        {
            m_buffer = nullptr;
        }
        m_position = m_buffer;
    }

    MemoryOStream::~MemoryOStream()
    {
        TkMemory::free(TkMemory::getDefault(), m_buffer);
    }

    void *MemoryOStream::move(u32 bytes)
    {
        const u32 position = getPosition();

        // Check if there is need to update a buffer size
        if (m_capacity - position < bytes)
        {
            // Perform reallocation
            u32 newCapacity = m_capacity != 0 ? m_capacity * 2 : 256;
            while (newCapacity < position + bytes)
                newCapacity *= 2;
            byte *newBuf = (byte *)TkMemory::allocate(TkMemory::getDefault(), newCapacity);
            if (newBuf == nullptr)
            {
                TK_ASSERT(false && "OutOfMemory");
            }
            TkMemory::memoryCopy(newBuf, m_buffer, m_capacity);
            TkMemory::free(TkMemory::getDefault(), m_buffer);

            // Update state
            m_buffer = newBuf;
            m_capacity = newCapacity;
            m_position = m_buffer + position;
        }

        // Skip bytes
        m_position += bytes;

        // Return pointer to begin
        return m_buffer + position;
    }

    void MemoryOStream::reset(u32 capacity)
    {
        // Check if resize
        if (capacity > m_capacity)
        {
            TkMemory::free(TkMemory::getDefault(), m_buffer);
            m_buffer = (byte *)TkMemory::allocate(TkMemory::getDefault(), capacity);
            if (m_buffer == nullptr)
            {
                TK_ASSERT(false && "OutOfMemory");
            }
            m_capacity = capacity;
        }

        // Reset pointer
        m_position = m_buffer;
    }

    bool MemoryOStream::saveToFile(const TkStringView &path) const
    {
        // Open file for writing
        OpenFileParams p{
            .path = path,
            .mode = FileMode::kCreateAlways,
            .access = FileAccess::kWrite,
            .share = FileShare::kRead,
        };
        auto *file = TkFileSystem::openFile(p);
        if (file == nullptr)
        {
            return false;
        }

        // Write data
        u32 bytesWritten;
        TkFileSystem::writeFile(file, getHandle(), getPosition(), &bytesWritten);

        TkFileSystem::closeFile(file);
        return true;
    }

    void MemoryOStream::flush()
    {
        // Nothing to do
    }

    void MemoryOStream::close()
    {
        TkMemory::free(TkMemory::getDefault(), m_buffer);
        m_buffer = nullptr;
        m_position = nullptr;
        m_capacity = 0;
    }

    u32 MemoryOStream::length()
    {
        return m_capacity;
    }

    u32 MemoryOStream::pos()
    {
        return static_cast<u32>(m_position - m_buffer);
    }

    void MemoryOStream::setPos(u32 seek)
    {
        m_position = m_buffer + seek;
    }

    void MemoryOStream::writeBytes(const void *data, u32 bytes)
    {
        // Calculate current position
        const u32 position = getPosition();

        // Check if there is need to update a buffer size
        if (m_capacity - position < bytes)
        {
            // Perform reallocation
            u32 newCapacity = m_capacity != 0 ? m_capacity * 2 : 256;
            while (newCapacity < position + bytes)
                newCapacity *= 2;
            byte *newBuf = (byte *)TkMemory::allocate(TkMemory::getDefault(), newCapacity);
            if (newBuf == nullptr)
            {
                TK_ASSERT(false && "OutOfMemory");
            }
            TkMemory::memoryCopy(newBuf, m_buffer, m_capacity);
            TkMemory::free(TkMemory::getDefault(), m_buffer);

            // Update state
            m_buffer = newBuf;
            m_capacity = newCapacity;
            m_position = m_buffer + position;
        }

        // Copy data
        TkMemory::memoryCopy(m_position, data, bytes);
        m_position += bytes;
    }

}