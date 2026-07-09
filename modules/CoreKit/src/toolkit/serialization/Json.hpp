/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/serialization/common.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit
{
    class JsonValue
    {
        friend class JsonDocument;
        friend class JsonTools;

    public:
        JsonValue() = default;

        bool isNull() const;
        bool isBool() const;
        bool isInt() const;
        bool isUint() const;
        bool isInt64() const;
        bool isUint64() const;
        bool isNumber() const;
        bool isString() const;
        bool isArray() const;
        bool isObject() const;
        JsonValueType getType() const;

        bool getBool(bool defaultValue = false) const;
        i32 getInt(i32 defaultValue = 0) const;
        u32 getUint(u32 defaultValue = 0) const;
        i64 getInt64(i64 defaultValue = 0) const;
        u64 getUint64(u64 defaultValue = 0) const;
        f32 getFloat(f32 defaultValue = 0.0f) const;
        f64 getDouble(f64 defaultValue = 0.0) const;
        TkStringView getString() const;

        i32 getArrayCount() const;
        JsonValue getArrayItem(i32 index) const;

        i32 getMemberCount() const;
        TkStringView getMemberName(i32 index) const;
        JsonValue getMemberValue(i32 index) const;
        JsonValue findMember(TkStringView name) const;
        bool hasMember(TkStringView name) const;

        explicit operator bool() const
        {
            return m_value != nullptr;
        }

    private:
        explicit JsonValue(const void *value)
            : m_value(value)
        {
        }

    private:
        const void *m_value = nullptr;
    };

    class JsonDocument final : public NonCopyable
    {
        friend class JsonTools;

    public:
        JsonDocument();
        ~JsonDocument();

        bool parse(TkStringView text);
        void clear();

        JsonValue getRoot() const;
        TkStringView getError() const;

    private:
        void changeIds(const TkHashMap<TkUuid, TkUuid> &mapping);

    private:
        void *m_document = nullptr;
        TkString m_error;
    };
}
