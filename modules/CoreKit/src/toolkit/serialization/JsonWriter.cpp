/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "JsonWriter.hpp"

#include <toolkit/collection/Array.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/types/Version.hpp>
#include <toolkit/utils/Convertor.hpp>

namespace toolkit
{
    void JsonWriter::key(TkStringView value)
    {
        key(value.getText(), value.length());
    }

    void JsonWriter::string(TkStringView value)
    {
        string(value.getText(), value.length());
    }

    void JsonWriter::string(const TkString &value)
    {
        string(TkStringView(value));
    }

    void JsonWriter::string(const char *value)
    {
        string(value ? TkStringView(value) : TkStringView::Empty);
    }

    void JsonWriter::rawValue(TkStringView value)
    {
        rawValue(value.getText(), value.length());
    }

    void JsonWriter::blob(TkSpan<const byte> data)
    {
        TkConvertor convertor;
        TkArray<char> encoded;
        convertor.base64Encode(data.get(), data.length(), encoded);
        string(encoded.get(), encoded.count());
    }

    void JsonWriter::uuid(const TkUuid &value)
    {
        string(value.toString(UuidFormatType::kN));
    }

    void JsonWriter::dateTime(const TkDateTime &value)
    {
        int64Value(value.ticks);
    }

    void JsonWriter::timeSpan(const TkTimeSpan &value)
    {
        int64Value(value.ticks);
    }

    void JsonWriter::version(const TkVersion &value)
    {
        string(value.toString());
    }
}
