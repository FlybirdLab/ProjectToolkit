/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{
    enum class JsonValueType : u8;

    class JsonValue;
    class JsonDocument;
    class JsonWriter;
    class CompactJsonWriter;
    class PrettyJsonWriter;

    class ISerializable;
    class ISerializeModifier;
}

namespace toolkit::serialization
{
    template <typename T>
    bool shouldSerialize(const T &value, const void *otherObj);

    template <typename T>
    void serialize(JsonWriter &stream, const T &value, const void *otherObj = nullptr);

    template <typename T>
    void deserialize(JsonValue stream, T &value, ISerializeModifier *modifier = nullptr);
}
