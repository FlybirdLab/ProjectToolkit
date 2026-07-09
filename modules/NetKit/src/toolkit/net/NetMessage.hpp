/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/StringView.hpp"
#include "toolkit/types/UUID.hpp"
#include <toolkit/net/common.hpp>

namespace toolkit
{
    // Network message structure. Provides raw data writing and reading to the message buffer.
    struct TK_API NetMessage
    {
    public:
        NetMessage() = default;
        NetMessage(u8 *buffer, u32 messageId, u32 bufferSize, u32 length, u32 position)
            : buffer(buffer), messageId(messageId), bufferSize(bufferSize), length(length), position(position)
        {
        }

        ~NetMessage() = default;

    public:
        // Write raw bytes into the message.
        TK_FORCE_INLINE void writeBytes(const u8 *bytes, const i32 numBytes)
        {
            TK_ASSERT(position + numBytes <= bufferSize);
            TkMemory::memoryCopy(buffer + position, bytes, numBytes);
            position += numBytes;
            length = position;
        }

        // Read raw bytes from the message into the given byte array.
        TK_FORCE_INLINE void readBytes(u8 *bytes, const i32 numBytes)
        {
            TK_ASSERT(position + numBytes <= bufferSize);
            TkMemory::memoryCopy(bytes, buffer + position, numBytes);
            position += numBytes;
        }

        // Skips bytes from the message.
        TK_FORCE_INLINE void *skipBytes(const i32 numBytes)
        {
            TK_ASSERT(position + numBytes <= bufferSize);
            byte *result = buffer + position;
            position += numBytes;
            return result;
        }

        template <typename T>
        TK_FORCE_INLINE void writeStructure(const T &data)
        {
            writeBytes((const u8 *)&data, sizeof(data));
        }

        template <typename T>
        TK_FORCE_INLINE void readStructure(T &data)
        {
            readBytes((u8 *)&data, sizeof(data));
        }

#define DECL_READWRITE(type, name)                                                                                   \
    TK_FORCE_INLINE void write##name(type value) { writeBytes(reinterpret_cast<const u8 *>(&value), sizeof(type)); } \
    TK_FORCE_INLINE type read##name()                                                                                \
    {                                                                                                                \
        type value = 0;                                                                                              \
        readBytes(reinterpret_cast<u8 *>(&value), sizeof(type));                                                     \
        return value;                                                                                                \
    }
        DECL_READWRITE(i8, I8)
        DECL_READWRITE(u8, U8)
        DECL_READWRITE(i16, I16)
        DECL_READWRITE(u16, U16)
        DECL_READWRITE(i32, I32)
        DECL_READWRITE(u32, U32)
        DECL_READWRITE(i64, I64)
        DECL_READWRITE(u64, U64)
        DECL_READWRITE(f32, F32)
        DECL_READWRITE(f64, F64)
        DECL_READWRITE(bool, Bool)
#undef DECL_READWRITE

        // Write data of type String into the message.
        TK_FORCE_INLINE void writeString(const TkStringView &value)
        {
            writeU16(value.length()); // TODO: Use 1-byte length when possible
            writeBytes((const u8 *)value.get(), value.length());
        }

        // <summary>
        // Read and returns data of type String from the message. ANSI encoded. Data valid within message lifetime.
        // </summary>
        TK_FORCE_INLINE TkStringView readString()
        {
            const u16 length = readU16();
            return TkStringView(length ? (const char *)skipBytes(length) : nullptr, length);
        }

        // Write data of type Guid into the message.
        TK_FORCE_INLINE void writeUuid(const TkUuid &value)
        {
            writeBytes((const u8 *)&value, sizeof(TkUuid));
        }

        // Read and returns data of type Guid from the message.
        TK_FORCE_INLINE TkUuid readUuid()
        {
            TkUuid value;
            readBytes((u8 *)&value, sizeof(TkUuid));
            return value;
        }

        // Write identifier into the stream that is networked-synced (by a server). If both peers acknowledge a specific id then the data transfer is optimized to 32 bits.
        void writeNetId(const TkUuid &id);
        // Read identifier from the stream that is networked-synced (by a server). If both peers acknowledge a specific id then the data transfer is optimized to 32 bits.
        void readNetId(TkUuid &id);
        // Write name into the stream that is networked-synced (by a server). If both peers acknowledge a specific name then the data transfer is optimized to 32 bits.
        void writeNetName(const TkStringView &name);
        // Read name from the stream that is networked-synced (by a server). If both peers acknowledge a specific name then the data transfer is optimized to 32 bits.
        void readNetName(TkStringView &name);

    public:
        // Return true if the message is valid for reading or writing.
        bool isValid() const
        {
            return buffer != nullptr && bufferSize > 0;
        }

    public:
        // The raw message buffer.
        u8 *buffer = nullptr;
        // The unique, internal message identifier.
        u32 messageId = 0;
        // The size in bytes of the buffer that this message has.
        u32 bufferSize = 0;
        // The length in bytes of this message.
        u32 length = 0;
        // The position in bytes in buffer where the next read/write will occur.
        u32 position = 0;
    };

    template <>
    struct TIsPODType<NetMessage>
    {
        enum
        {
            Value = true
        };
    };

}
