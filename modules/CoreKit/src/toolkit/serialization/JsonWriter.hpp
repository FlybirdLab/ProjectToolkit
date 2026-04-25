/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/serialization/common.hpp>
#include <toolkit/types/Span.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/types/UUID.hpp>

#define JSON_KEY(stream, keyName) (stream).key(toolkit::TkStringView(keyName, toolkit::lengthOf(keyName) - 1))

namespace toolkit
{
    class JsonWriter
    {
    public:
        virtual ~JsonWriter() = default;

        virtual void key(const char *str, i32 length) = 0;
        virtual void string(const char *str, i32 length) = 0;
        virtual void rawValue(const char *json, i32 length) = 0;
        virtual void boolValue(bool value) = 0;
        virtual void intValue(i32 value) = 0;
        virtual void int64Value(i64 value) = 0;
        virtual void uintValue(u32 value) = 0;
        virtual void uint64Value(u64 value) = 0;
        virtual void floatValue(f32 value) = 0;
        virtual void doubleValue(f64 value) = 0;
        virtual void nullValue() = 0;

        virtual void startObject() = 0;
        virtual void endObject() = 0;
        virtual void startArray() = 0;
        virtual void endArray(i32 count = 0) = 0;

    public:
        void key(TkStringView value);
        void string(TkStringView value);
        void string(const TkString &value);
        void string(const char *value);
        void rawValue(TkStringView value);
        void blob(TkSpan<const byte> data);
        void uuid(const TkUuid &value);
        void dateTime(const TkDateTime &value);
        void timeSpan(const TkTimeSpan &value);
        void version(const TkVersion &value);
    };
}
