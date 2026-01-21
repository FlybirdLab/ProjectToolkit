/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/io/Stream.hpp>

namespace toolkit {

    // Implementation of the stream that has access to the file and is optimized for fast reading from it
    class FileIStream : public ITkStream
    {
    public:
        NON_COPYABLE(FileIStream);

        FileIStream(void* file);
        ~FileIStream();

    public:
        // Gets the file handle.
        TK_FORCE_INLINE const void* getFile() const
        {
            return m_file;
        }

        // Unlink file object passed via constructor
        void unlink();

        // Open file to write data to it
        static FileIStream* open(const TkStringView& path);

        // [ReadStream]
        void flush() final override;
        void close() final override;
        u32 length() override;
        u32 pos() override;
        void setPos(u32 seek) override;
        void readBytes(void* data, u32 bytes) override;

    private:
        void* m_file;
        u32 m_virtualPosInBuffer; // Current position in the buffer (index)
        u32 m_bufferSize; // Amount of loaded bytes from the file to the buffer
        u32 m_filePosition; // Cached position in the file (native)
        byte m_buffer[kFileStreamBufferSize];
    };

    
    // Implementation of the stream that has access to the file and is optimized for fast writing to it.
    class FileOStream : public OTkStream
    {
    public:
        NON_COPYABLE(FileOStream);

        FileOStream(void* file);
        ~FileOStream();

    public:
        // Gets the file handle.
        TK_FORCE_INLINE const void* getFile() const
        {
            return m_file;
        }

        // Unlink file object passed via constructor
        void unlink();
        // Open file to read data from it
        static FileOStream* open(const TkStringView& path);

        // [WriteStream]
        void flush() final override;
        void close() final override;
        u32 length() override;
        u32 pos() override;
        void setPos(u32 seek) override;
        void writeBytes(const void* data, u32 bytes) override;

    private:
        void* m_file;
        u32 m_virtualPosInBuffer;
        byte m_buffer[kFileStreamBufferSize];
    };
}