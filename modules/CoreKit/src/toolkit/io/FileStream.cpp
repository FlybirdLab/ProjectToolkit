/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "FileStream.hpp"

#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

#define USE_FILE_POS (1)

    FileIStream *FileIStream::open(const TkStringView &path)
    {
        OpenFileParams p{
            .path = path,
            .mode = FileMode::kOpenExisting,
            .access = FileAccess::kRead,
            .share = FileShare::kRead};
        const auto file = TkFileSystem::openFile(p);
        if (file == nullptr)
        {
            LogWarn("Cannot open file '%s'", path);
            return nullptr;
        }
        return TkMemory::newObject<FileIStream>(TkMemory::getDefault(), file);
    }

    void FileIStream::unlink()
    {
        m_file = nullptr;
    }

    FileIStream::FileIStream(void *file)
        : m_file(file), m_virtualPosInBuffer(0), m_bufferSize(0), m_filePosition(TkFileSystem::getFilePos(file))
    {
    }

    FileIStream::~FileIStream()
    {
        // Ensure to be file closed and deleted
        if (m_file)
            TkFileSystem::closeFile(m_file);
    }

    void FileIStream::flush()
    {
        // Check if need to flush
        /*if (_virtualPosInBuffer > 0)
        {
            // Update buffer
            u32 bytesRead;
            _hasError |= _file->Read(_buffer, kFileStreamBufferSize, &bytesRead) != 0;
            _virtualPosInBuffer = 0;
        }*/
    }

    void FileIStream::close()
    {
        if (m_file)
            TkFileSystem::closeFile(m_file);
    }

    u32 FileIStream::length()
    {
        return TkFileSystem::getFileSize(m_file);
    }

    u32 FileIStream::pos()
    {
        return m_filePosition - m_bufferSize + m_virtualPosInBuffer;
    }

    void FileIStream::setPos(u32 seek)
    {
        // Skip if position won't change
        if (pos() == seek)
            return;

        // Try to seek with virtual position
        u32 bufferStartPos = m_filePosition - m_bufferSize;
        if (seek >= pos() && seek < m_filePosition)
        {
            m_virtualPosInBuffer = seek - bufferStartPos;
            return;
        }

        // Seek
        TkFileSystem::setFilePos(m_file, seek);
        m_filePosition = TkFileSystem::getFilePos(m_file);

        // Update buffer
        m_hasError |= !TkFileSystem::readFile(m_file, m_buffer, kFileStreamBufferSize, &m_bufferSize);
        m_filePosition += m_bufferSize;
        m_virtualPosInBuffer = 0;
    }

    void FileIStream::readBytes(void *data, u32 bytes)
    {
        // Skip if nothing to read
        if (bytes == 0)
        {
            return;
        }

        // Ensure to flush the buffer if it's empty
        if (m_bufferSize == 0)
        {
            TK_CHECK(m_virtualPosInBuffer == 0);
            m_hasError |= !TkFileSystem::readFile(m_file, m_buffer, kFileStreamBufferSize, &m_bufferSize);
            m_filePosition += m_bufferSize;
        }

        // Check if buffer has enough data for this read
        const u32 bufferBytesLeft = static_cast<u32>(m_bufferSize - m_virtualPosInBuffer);
        if (bytes <= bufferBytesLeft)
        {
            TkMemory::memoryCopy(data, m_buffer + m_virtualPosInBuffer, bytes);
            m_virtualPosInBuffer += bytes;
        }
        else
        {
            // Flush already buffered bytes and read more it the buffer (reduce amount of flushes)
            if (m_virtualPosInBuffer > 0)
            {
                TkMemory::memoryCopy(data, m_buffer + m_virtualPosInBuffer, bufferBytesLeft);
                data = (byte *)data + bufferBytesLeft;
                bytes -= bufferBytesLeft;
                m_virtualPosInBuffer = 0;
                m_hasError |= !TkFileSystem::readFile(m_file, m_buffer, kFileStreamBufferSize, &m_bufferSize);
                m_filePosition += m_bufferSize;
            }

            // Read as much as can using whole buffer
            while (bytes >= kFileStreamBufferSize)
            {
                TkMemory::memoryCopy(data, m_buffer, kFileStreamBufferSize);
                data = (byte *)data + kFileStreamBufferSize;
                bytes -= kFileStreamBufferSize;
                m_hasError |= !TkFileSystem::readFile(m_file, m_buffer, kFileStreamBufferSize, &m_bufferSize);
                m_filePosition += m_bufferSize;
            }

            // Read the rest of the buffer but without flushing its data
            if (bytes > 0)
            {
                TkMemory::memoryCopy(data, m_buffer, bytes);
                m_virtualPosInBuffer = bytes;
            }
        }
    }

    FileOStream *FileOStream::open(const TkStringView &path)
    {
        OpenFileParams p{path, FileMode::kCreateAlways, FileAccess::kWrite, FileShare::kRead};
        auto file = TkFileSystem::openFile(p);
        if (file == nullptr)
        {
            LogWarn("Cannot open file '%s'", path);
            return nullptr;
        }
        return TkMemory::newObject<FileOStream>(TkMemory::getDefault(), file);
    }

    void FileOStream::unlink()
    {
        m_file = nullptr;
    }

    FileOStream::FileOStream(void *file)
        : m_file(file), m_virtualPosInBuffer(0)
    {
        TK_ASSERT(m_file);
    }

    FileOStream::~FileOStream()
    {
        // Ensure to be file flushed, closed and deleted
        if (m_file)
        {
            flush();
            TkFileSystem::closeFile(m_file);
        }
    }

    void FileOStream::flush()
    {
        // Check if need to flush
        if (m_virtualPosInBuffer > 0)
        {
            // Update buffer
            u32 bytesWritten;
            m_hasError |= !TkFileSystem::writeFile(m_file, m_buffer, m_virtualPosInBuffer, &bytesWritten);
            m_virtualPosInBuffer = 0;
        }
    }

    void FileOStream::close()
    {
        if (m_file)
        {
            flush();
            TkFileSystem::closeFile(m_file);
        }
    }

    u32 FileOStream::length()
    {
        flush();
        return TkFileSystem::getFileSize(m_file);
    }

    u32 FileOStream::pos()
    {
        flush();
        return TkFileSystem::getFilePos(m_file);
    }

    void FileOStream::setPos(u32 seek)
    {
        flush();
        TkFileSystem::setFilePos(m_file, seek);
    }

    void FileOStream::writeBytes(const void *data, u32 bytes)
    {
        const u32 bufferBytesLeft = kFileStreamBufferSize - m_virtualPosInBuffer;
        if (bytes <= bufferBytesLeft)
        {
            TkMemory::memoryCopy(m_buffer + m_virtualPosInBuffer, data, bytes);
            m_virtualPosInBuffer += bytes;
        }
        else
        {
            u32 bytesWritten;

            // flush already written bytes and write more it the buffer (reduce amount of flushes)
            if (m_virtualPosInBuffer > 0)
            {
                TkMemory::memoryCopy(m_buffer + m_virtualPosInBuffer, data, bufferBytesLeft);
                data = (byte *)data + bufferBytesLeft;
                bytes -= bufferBytesLeft;
                m_virtualPosInBuffer = 0;
                m_hasError |= !TkFileSystem::writeFile(m_file, m_buffer, kFileStreamBufferSize, &bytesWritten);
            }

            // Write as much as can using whole buffer
            while (bytes >= kFileStreamBufferSize)
            {
                TkMemory::memoryCopy(m_buffer, data, kFileStreamBufferSize);
                data = (byte *)data + kFileStreamBufferSize;
                bytes -= kFileStreamBufferSize;
                m_hasError |= !TkFileSystem::writeFile(m_file, m_buffer, kFileStreamBufferSize, &bytesWritten);
            }

            // Write the rest of the buffer but without flushing its data
            if (bytes > 0)
            {
                TkMemory::memoryCopy(m_buffer, data, bytes);
                m_virtualPosInBuffer = bytes;
            }
        }
    }

}