/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/serialization/ISerializeModifier.hpp>
#include <toolkit/serialization/Json.hpp>

namespace toolkit
{
    class ISerializable
    {
    public:
        using SerializeStream = JsonWriter;
        using DeserializeStream = JsonValue;

    public:
        virtual ~ISerializable() = default;

        virtual void serialize(SerializeStream &stream, const void *otherObj = nullptr) = 0;
        virtual void deserialize(DeserializeStream stream, ISerializeModifier *modifier = nullptr) = 0;

        void deserializeIfExists(DeserializeStream stream, TkStringView memberName, ISerializeModifier *modifier = nullptr);
    };
}
