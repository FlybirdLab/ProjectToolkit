/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NetStream.hpp"
#include "INetSerializable.hpp"

namespace toolkit
{
    NetStream::NetStream()
        : ITkStream {}, OTkStream {}
    {
    }

    NetStream::~NetStream()
    {
        if (m_allocated)
            TkMemory::free(TkMemory::getDefault(), m_buffer);
    }

    void NetStream::init(u32 minCapacity)
    {
        // Unlink buffer if was reading from memory
        if (!m_allocated)
            m_buffer = nullptr;

        // Allocate if buffer is missing or too small
        if (!m_buffer || m_length < minCapacity)
        {
            // Release previous
            if (m_buffer)
                TkMemory::free(TkMemory::getDefault(), m_buffer);

            // Allocate new one
            m_buffer = (byte *)TkMemory::allocate(TkMemory::getDefault(), minCapacity);
            m_length = minCapacity;
            m_allocated = true;
        }

        // Reset pointer to the start
        m_position = m_buffer;
    }

    void NetStream::init(byte *buffer, u32 length)
    {
        if (m_allocated)
            TkMemory::free(TkMemory::getDefault(), m_buffer);
        m_position = m_buffer = buffer;
        m_length = length;
        m_allocated = false;
    }

    void NetStream::read(INetSerializable &obj)
    {
        obj.deserialize(this);
    }

    void NetStream::read(INetSerializable *obj)
    {
        obj->deserialize(this);
    }

    void NetStream::write(INetSerializable &obj)
    {
        obj.serialize(this);
    }

    void NetStream::write(INetSerializable *obj)
    {
        obj->serialize(this);
    }

    void NetStream::flush()
    {
        // Nothing to do
    }

    void NetStream::close()
    {
        if (m_allocated)
            TkMemory::free(TkMemory::getDefault(), m_buffer);
        m_position = m_buffer = nullptr;
        m_length = 0;
        m_allocated = false;
    }

    u32 NetStream::length()
    {
        return m_length;
    }

    u32 NetStream::pos()
    {
        return static_cast<u32>(m_position - m_buffer);
    }

    void NetStream::setPos(u32 seek)
    {
        TK_ASSERT(m_length > 0);
        m_position = m_buffer + seek;
    }

    void NetStream::readBytes(void *data, u32 bytes)
    {
        if (bytes > 0)
        {
            TK_ASSERT(data && length() - pos() >= bytes);
            TkMemory::memoryCopy(data, m_position, bytes);
            m_position += bytes;
        }
    }

    void NetStream::writeBytes(const void *data, u32 bytes)
    {
        // Calculate current position
        const u32 position = pos();

        // Check if there is need to update a buffer size
        if (m_length - position < bytes)
        {
            // Perform reallocation
            u32 newLength = m_length != 0 ? m_length * 2 : 256;
            while (newLength < position + bytes)
                newLength *= 2;
            byte *newBuf = (byte *)TkMemory::allocate(TkMemory::getDefault(), newLength);
            if (m_buffer && m_length)
                TkMemory::memoryCopy(newBuf, m_buffer, m_length);
            if (m_allocated)
                TkMemory::free(TkMemory::getDefault(), m_buffer);

            // Update state
            m_buffer = newBuf;
            m_length = newLength;
            m_position = m_buffer + position;
            m_allocated = true;
        }

        // Copy data
        TkMemory::memoryCopy(m_position, data, bytes);
        m_position += bytes;
    }

}