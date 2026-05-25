/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "FileSystem.hpp"

#include "toolkit/common.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/collection/DataCollection.hpp"
#include "toolkit/types/StringBuilder.hpp"
#include "toolkit/types/UUID.hpp"
#include "toolkit/utils/Convertor.hpp"

#include <cstring>
#include "toolkit/utils/Convertor.hpp"
#include "System.hpp"

namespace toolkit
{

    bool TkFileSystem::readAsBytes(FileHandle handle, byte *data, i32 length)
    {
        PROFILE_NAMED("FileSystem::readAsBytes");

        if (!handle || !isFileOpened(handle))
            return false;

        bool result;
        const u32 size = TkMath::min<u32>(getFileSize(handle), length);
        if (size > 0)
            result = readFile(handle, data, size) != 0;
        else
            result = false;
        return result;
    }

    bool TkFileSystem::readAsBytes(FileHandle handle, TkArray<byte> &data)
    {
        PROFILE_NAMED("FileSystem::readAsBytes");

        if (!handle || !isFileOpened(handle))
            return false;
        const u32 size = getFileSize(handle);

        if (size >= kMaxI32)
        {
            LogError("It's too big. Size: %f MB", size / (1024 * 1024));
            return false;
        }
        data.resize(size, false);
        if (size == 0)
            return true;

        return readFile(handle, data.get(), size) != 0;
    }

    bool TkFileSystem::readAsBytes(FileHandle handle, TkDataCollection<byte> &data)
    {
        PROFILE_NAMED("File::ReadAllBytes");

        if (!handle || !isFileOpened(handle))
            return false;

        const u32 size = getFileSize(handle);

        if (size >= kMaxI32)
        {
            LogError("Failed to load file. It's too big. Size: %f MB", size / (1024 * 1024));
            return false;
        }

        data.allocate(size);

        if (size == 0)
            return true;

        u32 readBytes = 0;
        if (!readFile(handle, data.get(), size, &readBytes))
            return false;
        return readBytes == size;
    }

    bool TkFileSystem::readAsText(FileHandle handle, TkString &data)
    {
        PROFILE_NAMED("File::ReadAllText");
        data.clear();

        TkArray<byte> bytes;
        if (!readAsBytes(handle, bytes))
            return false;

        // Check for empty data
        if (bytes.isEmpty())
            return true;

        // Check BOM char
        switch (bytes[0])
        {
        case 0xEF: // UTF-8
        {
            // BOM: EF BB BF
            if (bytes.count() < 3)
                return false;
            data.set((char *)(bytes.get() + 3), bytes.count() - 3);
        }
        break;
        case 0xFE: // UTF-16 (BE)
        {
            // BOM: FE FF
            if (bytes.count() < 2)
                return false;
            const i32 length = bytes.count() / 2 - 1;
            // utf16ToAnsi may emit up to 4 UTF-8 bytes per source wchar plus a null terminator.
            const i32 maxBytes = length * 4;
            data.resize(maxBytes);
            TkConvertor::utf16ToAnsi((wchar *)(bytes.get() + 2), data.get(), length);
            data.resize((i32)::strlen(data.get()));
        }
        break;
        case 0xFF: // UTF-16 (LE)
        {
            // BOM: FF FE
            if (bytes.count() < 2)
                return false;
            const i32 length = bytes.count() / 2 - 1;
            const i32 maxBytes = length * 4;
            data.resize(maxBytes);
            TkConvertor::utf16ToAnsi((wchar *)(bytes.get() + 2), data.get(), length);
            data.resize((i32)::strlen(data.get()));
        }
        break;
        default: // ANSI
        {
            data.set((char *)bytes.get(), bytes.count());
        }
        break;
        }

        return true;
    }

    bool TkFileSystem::writeBytes(FileHandle handle, const byte *data, i32 length)
    {
        PROFILE_NAMED("File::WriteAllBytes");

        if (!handle || !isFileOpened(handle))
            return false;

        if (length == 0)
            return true;

        u32 written = 0;
        if (!writeFile(handle, data, length, &written))
            return false;
        return written == (u32)length;
    }

    bool TkFileSystem::writeBytes(FileHandle handle, const TkArray<byte> &data)
    {
        return writeBytes(handle, data.get(), data.count());
    }

    bool TkFileSystem::writeText(FileHandle handle, const TkString &data, Encoding encoding)
    {
        return writeText(handle, *data, data.length(), encoding);
    }

    bool TkFileSystem::writeText(FileHandle handle, const TkStringBuilder &data, Encoding encoding)
    {
        return writeText(handle, data.begin(), data.length(), encoding);
    }

    bool TkFileSystem::writeText(FileHandle handle, const char *data, i32 length, Encoding encoding)
    {
        PROFILE_NAMED("File::WriteAllText");

        switch (encoding)
        {
        case Encoding::kANSI:
        {
            return writeBytes(handle, (byte *)data, length * sizeof(char));
        }
        case Encoding::kUnicode:
        {
            // UTF-16 (LE)
            // BOM: FF FE

            TK_ASSERT("Not impl");
            return false;
        }
        case Encoding::kUnicodeBigEndian:
        {
            // UTF-16 (BE)
            // BOM: FE FF

            TK_ASSERT("Not impl");
            return false;
        }
        case Encoding::kUTF8:
        {
            return writeBytes(handle, (byte *)data, length * sizeof(char));
        }
        default:
            return false;
        }
    }

    u64 TkFileSystem::getDirSize(const TkStringView path)
    {
        u64 result = 0;
        TkArray<TkString> files;
        TkFileSystem::listDir(files, path);
        for (const auto &file : files)
            result += TkFileSystem::getFileSize(file);
        return result;
    }

    void TkFileSystem::getTempDirPath(TkString &tmpPath)
    {

        tmpPath = System::getTempDir() / TkUuid::create().toString(UuidFormatType::kN);
    }
}
