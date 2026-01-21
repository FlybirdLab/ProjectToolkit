/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Stream.hpp"

#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{

    void ITkStream::read(TkString &data)
    {
        i32 length;
        readI32(&length);
        if (length <= 0 || length > kStreamMaxStringSize)
        {
            if (length != 0)
                m_hasError = true;
            data.clear();
            return;
        }

        data.reserve(length);
        char *ptr = data.get();
        TK_ASSERT(ptr != nullptr);
        readBytes(ptr, length * sizeof(char));
    }

    void ITkStream::read(TkString &data, i16 lock)
    {
        i32 length;
        readI32(&length);
        if (length <= 0 || length > kStreamMaxStringSize)
        {
            if (length != 0)
                m_hasError = true;
            data.clear();
            return;
        }

        data.reserve(length);
        char *ptr = data.get();
        TK_ASSERT(ptr != nullptr);
        readBytes(ptr, length * sizeof(char));

        for (i32 i = 0; i < length; i++)
        {
            *ptr = *ptr ^ lock;
            ptr++;
        }
    }

    void OTkStream::writeText(const TkStringView& text)
    {
        writeBytes(text.get(), sizeof(char) * text.length());
    }

    void OTkStream::write(const TkStringView& data)
    {
        const i32 length = data.length();
        TK_ASSERT(length < kStreamMaxStringSize);
        writeI32(length);
        writeBytes(*data, length * sizeof(char));
    }

    void OTkStream::write(const TkStringView& data, i16 lock)
    {
        TK_ASSERT(data.length() < kStreamMaxStringSize);
        writeI32(data.length());
        for (i32 i = 0; i < data.length(); i++)
            writeU8((u8)((u8)data[i] ^ lock));
    }

    void OTkStream::writeString(const TkStringView& data)
    {
        write(data);
    }
}