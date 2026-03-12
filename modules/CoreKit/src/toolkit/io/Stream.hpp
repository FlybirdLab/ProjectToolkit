/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>

namespace toolkit {
    
    // Base class for all data streams (memory streams, file streams etc.)
    class TkStream
    {
    public:
        static constexpr i32 kFileStreamBufferSize = 4096;
        static constexpr i32 kStreamMaxStringSize = 4*1024; // 4 kB

    public:
        virtual ~TkStream() = default;

    public:
        // Returns true if error occurred during reading/writing to the stream
        virtual bool hasError() const
        {
            return m_hasError;
        }

        // Returns true if can read data from that stream.
        virtual bool canRead()
        {
            return false;
        }

        // Returns true if can write data to that stream.
        virtual bool canWrite()
        {
            return false;
        }

    public:
        // Flush the stream buffers
        virtual void flush() = 0;
        // Close the stream
        virtual void close() = 0;

        // Gets length of the stream
        virtual u32 length() = 0;
        // Gets current position in the stream
        virtual u32 pos() = 0;

        // Set new position in the stream
        virtual void setPos(u32 seek) = 0;

    protected:
        TkStream() = default;

        bool m_hasError = false;
    };

    
    // Base class for all data read streams
    class ITkStream : public TkStream
    {
    public:
        // Reads bytes from the stream
        virtual void readBytes(void* data, u32 bytes) = 0;

    public:
        byte readByte()
        {
            byte data;
            readBytes(&data, 1);
            return data;
        }

        bool readBool()
        {
            byte data;
            readBytes(&data, 1);
            return data != 0;
        }

        char readChar()
        {
            char data;
            readBytes(&data, 1);
            return data;
        }

        TK_FORCE_INLINE void readByte(byte* data)
        {
            readBytes(data, sizeof(byte));
        }

        TK_FORCE_INLINE void readChar(char* data)
        {
            readBytes(data, sizeof(char));
        }

        TK_FORCE_INLINE void readU8(u8* data)
        {
            readBytes(data, sizeof(u8));
        }

        TK_FORCE_INLINE void readI8(i8* data)
        {
            readBytes(data, sizeof(i8));
        }

        TK_FORCE_INLINE void readU16(u16* data)
        {
            readBytes(data, sizeof(u16));
        }

        TK_FORCE_INLINE void readI16(i16* data)
        {
            readBytes(data, sizeof(i16));
        }

        TK_FORCE_INLINE void readU32(u32* data)
        {
            readBytes(data, sizeof(u32));
        }

        TK_FORCE_INLINE void readI32(i32* data)
        {
            readBytes(data, sizeof(i32));
        }

        TK_FORCE_INLINE void readU64(u64* data)
        {
            readBytes(data, sizeof(u64));
        }

        TK_FORCE_INLINE void readI64(i64* data)
        {
            readBytes(data, sizeof(i64));
        }

        TK_FORCE_INLINE void readF32(f32* data)
        {
            readBytes(data, sizeof(f32));
        }

        TK_FORCE_INLINE void readF64(f64* data)
        {
            readBytes(data, sizeof(f64));
        }

        void read(TkString& data);
        void read(TkString& data, i16 lock);

        template<typename T>
        TK_FORCE_INLINE void read(T& data)
            requires(TPODType<T>)
        {
            readBytes((void*)&data, sizeof(T));
        }

        // Read data array
        template<typename T>
        void read(TkArray<T>& data)
        {
            i32 size;
            readI32(&size);
            data.resize(size, false);
            if (size > 0)
            {
                if constexpr (TPODType<T> && !TPointerType<T>)
                    readBytes(data.get(), size * sizeof(T));
                else
                {
                    for (i32 i = 0; i < size; i++)
                        read(data[i]);
                }
            }
        }

        // Read data dictionary
        template<typename KeyType, typename ValueType>
        void read(TkHashMap<KeyType, ValueType>& data)
        {
            i32 count;
            readI32(&count);
            data.clear();
            data.ensureCapacity(count);
            for (i32 i = 0; i < count; i++)
            {
                KeyType key;
                read(key);
                read(data[key]);
            }
        }

    public:
        bool canRead() override
        {
            return pos() != length();
        }
    };


    // Base class for all data write streams
    class OTkStream : public TkStream
    {
    public:
        virtual void writeBytes(const void* data, u32 bytes) = 0;

        TK_FORCE_INLINE void writeByte(byte data)
        {
            writeBytes(&data, sizeof(byte));
        }
        TK_FORCE_INLINE void writeBool(bool data)
        {
            writeBytes(&data, sizeof(bool));
        }
        TK_FORCE_INLINE void writeWchar(char data)
        {
            writeBytes(&data, sizeof(char));
        }
        TK_FORCE_INLINE void writeU8(u8 data)
        {
            writeBytes(&data, sizeof(u8));
        }
        TK_FORCE_INLINE void writeI8(i8 data)
        {
            writeBytes(&data, sizeof(i8));
        }
        TK_FORCE_INLINE void writeU16(u16 data)
        {
            writeBytes(&data, sizeof(u16));
        }
        TK_FORCE_INLINE void writeI16(i16 data)
        {
            writeBytes(&data, sizeof(i16));
        }
        TK_FORCE_INLINE void writeU32(u32 data)
        {
            writeBytes(&data, sizeof(u32));
        }
        TK_FORCE_INLINE void writeI32(i32 data)
        {
            writeBytes(&data, sizeof(i32));
        }
        TK_FORCE_INLINE void writeI64(i64 data)
        {
            writeBytes(&data, sizeof(i64));
        }
        TK_FORCE_INLINE void writeU64(u64 data)
        {
            writeBytes(&data, sizeof(u64));
        }
        TK_FORCE_INLINE void writeF32(f32 data)
        {
            writeBytes(&data, sizeof(f32));
        }
        TK_FORCE_INLINE void writeF64(f64 data)
        {
            writeBytes(&data, sizeof(f64));
        }

        void writeText(const char* text, i32 length)
        {
            writeBytes((const void*)text, sizeof(char) * length);
        }

        void writeBOM()
        {
            writeByte(0xEF);
            writeByte(0xBB);
            writeByte(0xBF);
        }

        void writeText(const TkStringView& text);

        void write(const TkStringView& data);
        void write(const TkStringView& data, i16 lock);

        template<typename T>
        TK_FORCE_INLINE void write(const T& data)
            requires(TPODType<T> && !TPointerType<T>)
        {
            writeBytes((const void*)&data, sizeof(T));
        }

        template<typename T>
        void write(const TkArray<T>& data)
        {
            const i32 size = data.count();
            writeI32(size);
            if (size > 0)
            {
                if constexpr (TPODType<T> && !TPointerType<T>)
                    writeBytes(data.get(), size * sizeof(T));
                else
                {
                    for (i32 i = 0; i < size; i++)
                        write(data[i]);
                }
            }
        }


        template<typename KeyType, typename ValueType>
        void write(const TkHashMap<KeyType, ValueType>& data)
        {
            const i32 count = data.count();
            writeI32(count);
            for (const auto& e : data)
            {
                write(e.Key);
                write(e.Value);
            }
        }
        
        // Write Ansi String to the stream
        void writeString(const TkStringView& data);

        // Write data array
        template<typename T>
        TK_FORCE_INLINE void writeArray(const TkArray<T>& data)
        {
            write(data);
        }

    public:
        bool canWrite() override
        {
            return true;
        }
    };

}
