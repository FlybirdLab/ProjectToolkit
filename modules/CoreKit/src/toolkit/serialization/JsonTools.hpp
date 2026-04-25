/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/serialization/Json.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/types/Version.hpp>

namespace toolkit
{
    class JsonTools
    {
    public:
        static void changeIds(JsonDocument &document, const TkHashMap<TkUuid, TkUuid> &mapping);

        static TkString getString(JsonValue node, TkStringView name, TkStringView defaultValue = TkStringView::Empty);
        static bool getBool(JsonValue node, TkStringView name, bool defaultValue = false);
        static i32 getInt(JsonValue node, TkStringView name, i32 defaultValue = 0);
        static u32 getUint(JsonValue node, TkStringView name, u32 defaultValue = 0);
        static i64 getInt64(JsonValue node, TkStringView name, i64 defaultValue = 0);
        static u64 getUint64(JsonValue node, TkStringView name, u64 defaultValue = 0);
        static f32 getFloat(JsonValue node, TkStringView name, f32 defaultValue = 0.0f);
        static f64 getDouble(JsonValue node, TkStringView name, f64 defaultValue = 0.0);
        static TkUuid getUuid(JsonValue value);
        static TkUuid getUuid(JsonValue node, TkStringView name, const TkUuid &defaultValue = TkUuid::Empty);
        static TkDateTime getDateTime(JsonValue value);
        static TkDateTime getDateTime(JsonValue node, TkStringView name, const TkDateTime &defaultValue = TkDateTime());
        static TkTimeSpan getTimeSpan(JsonValue value);
        static TkTimeSpan getTimeSpan(JsonValue node, TkStringView name, const TkTimeSpan &defaultValue = TkTimeSpan());
        static TkVersion getVersion(JsonValue value);
        static TkVersion getVersion(JsonValue node, TkStringView name, const TkVersion &defaultValue = TkVersion());
        static void getBlob(JsonValue value, TkArray<byte> &output);
        static void getBlob(JsonValue node, TkStringView name, TkArray<byte> &output);
    };
}
