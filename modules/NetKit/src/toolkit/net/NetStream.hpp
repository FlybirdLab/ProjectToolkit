/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>
#include <toolkit/io/Stream.hpp>

namespace toolkit
{
    class INetSerializable;

    // Objects and values serialization stream for sending data over network. Uses memory buffer for both read and write operations.
    class TK_API NetStream final : public ITkStream, public OTkStream
    {
    public:
        NetStream();
        ~NetStream();

        // The ClientId of the network client that is a data sender. Can be used to detect who send the incoming RPC or replication data. Set to the current client when writing data.
        u32 senderId = 0;

        // Gets the pointer to the native stream memory buffer.
        byte *getBuffer() const
        {
            return m_buffer;
        }

        // Initialize the stream for writing. Allocates the memory or reuses already existing memory. Resets the current stream position to beginning.
        void init(u32 minCapacity = 1024);
        // Initialize the stream for reading.
        void init(byte *buffer, u32 length);

        // Write bytes to the stream
        TK_FORCE_INLINE void writeData(const void *data, i32 bytes)
        {
            writeBytes(data, bytes);
        }

        // Read bytes from the stream
        TK_FORCE_INLINE void readData(void *data, i32 bytes)
        {
            readBytes(data, bytes);
        }

        using ITkStream::read;
        void read(INetSerializable &obj);
        void read(INetSerializable *obj);

        using OTkStream::write;
        void write(INetSerializable &obj);
        void write(INetSerializable *obj);

    public:
        void flush() override;
        void close() override;
        u32 length() override;
        u32 pos() override;
        void setPos(u32 seek) override;

    public:
        void readBytes(void *data, u32 bytes) override;
        void writeBytes(const void *data, u32 bytes) override;

    private:
        byte *m_buffer = nullptr;
        byte *m_position = nullptr;
        u32 m_length = 0;
        bool m_allocated = false;
    };

}