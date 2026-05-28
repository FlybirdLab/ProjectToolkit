/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/serialization/Serialization.hpp>

namespace toolkit
{
    class JsonSerializer
    {
    public:
        static TkString saveToString(ISerializable *object, bool pretty = false)
        {
            TkString result;
            if (pretty)
            {
                PrettyJsonWriter writer(result);
                object ? serialization::serialize(writer, *object, nullptr) : writer.nullValue();
            }
            else
            {
                CompactJsonWriter writer(result);
                object ? serialization::serialize(writer, *object, nullptr) : writer.nullValue();
            }
            return result;
        }

        template <typename T>
        static TkString saveToString(const T &value, bool pretty = false)
        {
            return serialization::saveToString(value, pretty);
        }

        static bool loadFromString(ISerializable *object, TkStringView text, ISerializeModifier *modifier = nullptr)
        {
            JsonDocument document;
            if (!document.parse(text))
                return false;
            if (object != nullptr)
            {
                JsonValue root = document.getRoot();
                object->deserialize(root, modifier);
            }
            return true;
        }

        template <typename T>
        static bool loadFromString(T &value, TkStringView text, ISerializeModifier *modifier = nullptr)
        {
            return serialization::loadFromString(value, text, modifier);
        }

        static TkArray<byte> saveToBytes(ISerializable *object, bool pretty = false)
        {
            TkString text = saveToString(object, pretty);
            TkArray<byte> result;
            result.resize(text.length(), false);
            if (text.length() > 0)
                TkMemory::memoryCopy(result.get(), text.get(), static_cast<psize>(text.length()));
            return result;
        }

        template <typename T>
        static TkArray<byte> saveToBytes(const T &value, bool pretty = false)
        {
            return serialization::saveToBytes(value, pretty);
        }

        static bool loadFromBytes(ISerializable *object, TkSpan<const byte> bytes, ISerializeModifier *modifier = nullptr)
        {
            return loadFromString(object, TkStringView(reinterpret_cast<const char *>(bytes.get()), bytes.length()), modifier);
        }

        static bool loadFromBytes(ISerializable *object, const TkArray<byte> &bytes, ISerializeModifier *modifier = nullptr)
        {
            return loadFromBytes(object, TkSpan<const byte>(bytes.get(), bytes.count()), modifier);
        }

        template <typename T>
        static bool loadFromBytes(T &value, TkSpan<const byte> bytes, ISerializeModifier *modifier = nullptr)
        {
            return serialization::loadFromBytes(value, bytes, modifier);
        }

        template <typename T>
        static bool loadFromBytes(T &value, const TkArray<byte> &bytes, ISerializeModifier *modifier = nullptr)
        {
            return serialization::loadFromBytes(value, bytes, modifier);
        }
    };
}
