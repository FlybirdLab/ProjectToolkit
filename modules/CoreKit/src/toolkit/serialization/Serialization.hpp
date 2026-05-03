/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/base/Assert.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/serialization/ISerializable.hpp>
#include <toolkit/serialization/ISerializeModifier.hpp>
#include <toolkit/serialization/JsonTools.hpp>
#include <toolkit/serialization/JsonWriters.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/Span.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/types/Version.hpp>
#include <toolkit/utils/Convertor.hpp>
#include <toolkit/utils/StringUtils.hpp>

namespace toolkit
{
    template <typename T>
    bool serializeCustom(JsonWriter &, const T &, const void *)
    {
        return false;
    }

    template <typename T>
    bool deserializeCustom(JsonValue, T &, ISerializeModifier *)
    {
        return false;
    }

    template <typename T>
    bool shouldSerializeCustom(const T &, const void *)
    {
        return true;
    }
}

namespace toolkit::serialization
{
    template <typename T>
    bool shouldSerialize(const T &value, const void *otherObj);

    template <typename T>
    bool shouldSerialize(const TkArray<T> &value, const void *otherObj);

    template <typename KeyType, typename ValueType>
    bool shouldSerialize(const TkHashMap<KeyType, ValueType> &value, const void *otherObj);

    template <typename T>
    void serialize(JsonWriter &stream, const T &value, const void *otherObj);

    template <typename T>
    void deserialize(JsonValue stream, T &value, ISerializeModifier *modifier);

    void serialize(JsonWriter &stream, const bool &value, const void *otherObj);
    void serialize(JsonWriter &stream, const i8 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const u8 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const i16 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const u16 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const i32 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const u32 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const i64 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const u64 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const f32 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const f64 &value, const void *otherObj);
    void serialize(JsonWriter &stream, const TkString &value, const void *otherObj);
    void serialize(JsonWriter &stream, const TkStringView &value, const void *otherObj);
    void serialize(JsonWriter &stream, const char *const &value, const void *otherObj);
    void serialize(JsonWriter &stream, const TkUuid &value, const void *otherObj);
    void serialize(JsonWriter &stream, const TkDateTime &value, const void *otherObj);
    void serialize(JsonWriter &stream, const TkTimeSpan &value, const void *otherObj);
    void serialize(JsonWriter &stream, const TkVersion &value, const void *otherObj);
    void serialize(JsonWriter &stream, const ISerializable &value, const void *otherObj);

    void deserialize(JsonValue stream, bool &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, i8 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, u8 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, i16 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, u16 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, i32 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, u32 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, i64 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, u64 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, f32 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, f64 &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, TkString &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, TkUuid &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, TkDateTime &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, TkTimeSpan &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, TkVersion &value, ISerializeModifier *modifier);
    void deserialize(JsonValue stream, ISerializable &value, ISerializeModifier *modifier);

    template <typename T>
    void serialize(JsonWriter &stream, const TkArray<T> &value, const void *otherObj);

    template <typename T>
    void deserialize(JsonValue stream, TkArray<T> &value, ISerializeModifier *modifier);

    template <typename KeyType, typename ValueType>
    void serialize(JsonWriter &stream, const TkHashMap<KeyType, ValueType> &value, const void *otherObj);

    template <typename KeyType, typename ValueType>
    void deserialize(JsonValue stream, TkHashMap<KeyType, ValueType> &value, ISerializeModifier *modifier);

    inline i32 deserializeInt(JsonValue stream)
    {
        if (stream.isNumber())
            return stream.getInt();
        if (stream.isString())
        {
            i32 result = 0;
            TkStringUtils::fromString(stream.getString(), result);
            return result;
        }
        return 0;
    }

    inline i64 deserializeInt64(JsonValue stream)
    {
        if (stream.isNumber())
            return stream.getInt64();
        if (stream.isString())
        {
            i64 result = 0;
            TkStringUtils::fromString(stream.getString(), result);
            return result;
        }
        return 0;
    }

    inline u32 deserializeUint(JsonValue stream)
    {
        if (stream.isNumber())
            return stream.getUint();
        if (stream.isString())
        {
            u32 result = 0;
            TkStringUtils::fromString(stream.getString(), result);
            return result;
        }
        return 0;
    }

    inline u64 deserializeUint64(JsonValue stream)
    {
        if (stream.isNumber())
            return stream.getUint64();
        if (stream.isString())
        {
            u64 result = 0;
            TkStringUtils::fromString(stream.getString(), result);
            return result;
        }
        return 0;
    }

    template <typename T>
    bool shouldSerialize(const T &value, const void *otherObj)
    {
        if constexpr (TArithmeticType<TDecayT<T>> || TEnumType<TDecayT<T>>)
            return !otherObj || value != *static_cast<const T *>(otherObj);
        else
            return shouldSerializeCustom(value, otherObj);
    }

    inline bool shouldSerialize(const TkString &value, const void *otherObj)
    {
        return !otherObj || value != *static_cast<const TkString *>(otherObj);
    }

    inline bool shouldSerialize(const TkStringView &value, const void *otherObj)
    {
        return !otherObj || value != *static_cast<const TkStringView *>(otherObj);
    }

    inline bool shouldSerialize(const char *const &value, const void *otherObj)
    {
        if (!otherObj)
            return value != nullptr && value[0] != '\0';
        const char *other = *static_cast<const char *const *>(otherObj);
        return TkStringView(value ? value : "") != TkStringView(other ? other : "");
    }

    inline bool shouldSerialize(const TkUuid &value, const void *otherObj)
    {
        return otherObj ? value != *static_cast<const TkUuid *>(otherObj) : value.isValid();
    }

    inline bool shouldSerialize(const TkDateTime &value, const void *otherObj)
    {
        return !otherObj || value.ticks != static_cast<const TkDateTime *>(otherObj)->ticks;
    }

    inline bool shouldSerialize(const TkTimeSpan &value, const void *otherObj)
    {
        return !otherObj || value.ticks != static_cast<const TkTimeSpan *>(otherObj)->ticks;
    }

    inline bool shouldSerialize(const TkVersion &value, const void *otherObj)
    {
        return !otherObj || value != *static_cast<const TkVersion *>(otherObj);
    }

    inline bool shouldSerialize(const ISerializable &, const void *)
    {
        return true;
    }

    template <typename T>
    void serializeMember(JsonWriter &stream, TkStringView name, const T &value, const void *otherObj = nullptr)
    {
        stream.key(name);
        serialize(stream, value, otherObj);
    }

    template <typename T>
    void serializeMemberIfChanged(JsonWriter &stream, TkStringView name, const T &value, const void *otherObj = nullptr)
    {
        if (shouldSerialize(value, otherObj))
            serializeMember(stream, name, value, otherObj);
    }

    template <typename T>
    bool deserializeMember(JsonValue stream, TkStringView name, T &value, ISerializeModifier *modifier = nullptr)
    {
        JsonValue member = stream.findMember(name);
        if (!member)
            return false;
        deserialize(member, value, modifier);
        return true;
    }

    template <typename T>
    void deserializeMember(JsonValue stream, TkStringView name, T &value, const T &defaultValue, ISerializeModifier *modifier = nullptr)
    {
        if (!deserializeMember(stream, name, value, modifier))
            value = defaultValue;
    }

    inline void serialize(JsonWriter &stream, const bool &value, const void *)
    {
        stream.boolValue(value);
    }
    inline void serialize(JsonWriter &stream, const i8 &value, const void *)
    {
        stream.intValue(value);
    }
    inline void serialize(JsonWriter &stream, const u8 &value, const void *)
    {
        stream.uintValue(value);
    }
    inline void serialize(JsonWriter &stream, const i16 &value, const void *)
    {
        stream.intValue(value);
    }
    inline void serialize(JsonWriter &stream, const u16 &value, const void *)
    {
        stream.uintValue(value);
    }
    inline void serialize(JsonWriter &stream, const i32 &value, const void *)
    {
        stream.intValue(value);
    }
    inline void serialize(JsonWriter &stream, const u32 &value, const void *)
    {
        stream.uintValue(value);
    }
    inline void serialize(JsonWriter &stream, const i64 &value, const void *)
    {
        stream.int64Value(value);
    }
    inline void serialize(JsonWriter &stream, const u64 &value, const void *)
    {
        stream.uint64Value(value);
    }
    inline void serialize(JsonWriter &stream, const f32 &value, const void *)
    {
        stream.floatValue(value);
    }
    inline void serialize(JsonWriter &stream, const f64 &value, const void *)
    {
        stream.doubleValue(value);
    }
    inline void serialize(JsonWriter &stream, const TkString &value, const void *)
    {
        stream.string(value);
    }
    inline void serialize(JsonWriter &stream, const TkStringView &value, const void *)
    {
        stream.string(value);
    }
    inline void serialize(JsonWriter &stream, const char *const &value, const void *)
    {
        stream.string(value ? TkStringView(value) : TkStringView::Empty);
    }
    inline void serialize(JsonWriter &stream, const TkUuid &value, const void *)
    {
        stream.uuid(value);
    }
    inline void serialize(JsonWriter &stream, const TkDateTime &value, const void *)
    {
        stream.dateTime(value);
    }
    inline void serialize(JsonWriter &stream, const TkTimeSpan &value, const void *)
    {
        stream.timeSpan(value);
    }
    inline void serialize(JsonWriter &stream, const TkVersion &value, const void *)
    {
        stream.version(value);
    }
    inline void serialize(JsonWriter &stream, const ISerializable &value, const void *otherObj)
    {
        stream.startObject();
        const_cast<ISerializable &>(value).serialize(stream, otherObj);
        stream.endObject();
    }

    template <typename T>
    void serialize(JsonWriter &stream, const TkArray<T> &value, const void *)
    {
        stream.startArray();
        for (i32 i = 0; i < value.count(); ++i)
            serialize(stream, value[i], nullptr);
        stream.endArray(value.count());
    }

    template <typename T>
    bool shouldSerialize(const TkArray<T> &value, const void *otherObj)
    {
        if (!otherObj)
            return true;

        const auto *other = static_cast<const TkArray<T> *>(otherObj);
        if (value.count() != other->count())
            return true;

        for (i32 i = 0; i < value.count(); ++i)
        {
            if (shouldSerialize(value[i], &(*other)[i]))
                return true;
        }
        return false;
    }

    template <typename KeyType, typename ValueType>
    void serialize(JsonWriter &stream, const TkHashMap<KeyType, ValueType> &value, const void *)
    {
        stream.startArray();
        for (const auto &item : value)
        {
            stream.startObject();
            JSON_KEY(stream, "Key");
            serialize(stream, item.key, nullptr);
            JSON_KEY(stream, "Value");
            serialize(stream, item.value, nullptr);
            stream.endObject();
        }
        stream.endArray(value.count());
    }

    template <typename KeyType, typename ValueType>
    bool shouldSerialize(const TkHashMap<KeyType, ValueType> &value, const void *otherObj)
    {
        if (!otherObj)
            return true;

        const auto *other = static_cast<const TkHashMap<KeyType, ValueType> *>(otherObj);
        if (value.count() != other->count())
            return true;

        for (const auto &item : value)
        {
            auto itr = other->find(item.Key);
            if (itr == other->End() || shouldSerialize(item.Value, &itr->Value))
                return true;
        }
        return false;
    }

    template <typename T>
    void serialize(JsonWriter &stream, const T &value, const void *otherObj)
    {
        if constexpr (TEnumType<TDecayT<T>>)
            stream.uintValue(static_cast<u32>(value));
        else if constexpr (TIsBaseOf<ISerializable, T>::Value)
            serialize(stream, static_cast<const ISerializable &>(value), otherObj);
        else if (!serializeCustom(stream, value, otherObj))
        {
            TK_ASSERT(false && "Missing serialization overload for type");
            stream.nullValue();
        }
    }

    inline void deserialize(JsonValue stream, bool &value, ISerializeModifier *)
    {
        value = stream.getBool();
    }
    inline void deserialize(JsonValue stream, i8 &value, ISerializeModifier *)
    {
        value = static_cast<i8>(deserializeInt(stream));
    }
    inline void deserialize(JsonValue stream, u8 &value, ISerializeModifier *)
    {
        value = static_cast<u8>(deserializeUint(stream));
    }
    inline void deserialize(JsonValue stream, i16 &value, ISerializeModifier *)
    {
        value = static_cast<i16>(deserializeInt(stream));
    }
    inline void deserialize(JsonValue stream, u16 &value, ISerializeModifier *)
    {
        value = static_cast<u16>(deserializeUint(stream));
    }
    inline void deserialize(JsonValue stream, i32 &value, ISerializeModifier *)
    {
        value = deserializeInt(stream);
    }
    inline void deserialize(JsonValue stream, u32 &value, ISerializeModifier *)
    {
        value = deserializeUint(stream);
    }
    inline void deserialize(JsonValue stream, i64 &value, ISerializeModifier *)
    {
        value = deserializeInt64(stream);
    }
    inline void deserialize(JsonValue stream, u64 &value, ISerializeModifier *)
    {
        value = deserializeUint64(stream);
    }
    inline void deserialize(JsonValue stream, f32 &value, ISerializeModifier *)
    {
        value = stream.getFloat();
    }
    inline void deserialize(JsonValue stream, f64 &value, ISerializeModifier *)
    {
        value = stream.getDouble();
    }
    inline void deserialize(JsonValue stream, TkString &value, ISerializeModifier *)
    {
        value = stream.getString();
    }
    inline void deserialize(JsonValue stream, TkUuid &value, ISerializeModifier *modifier)
    {
        value = JsonTools::getUuid(stream);
        if (modifier != nullptr)
        {
            auto mapped = modifier->idsMapping.find(value);
            if (mapped != modifier->idsMapping.End())
                value = mapped->value;
        }
    }
    inline void deserialize(JsonValue stream, TkDateTime &value, ISerializeModifier *)
    {
        value = JsonTools::getDateTime(stream);
    }
    inline void deserialize(JsonValue stream, TkTimeSpan &value, ISerializeModifier *)
    {
        value = JsonTools::getTimeSpan(stream);
    }
    inline void deserialize(JsonValue stream, TkVersion &value, ISerializeModifier *)
    {
        value = JsonTools::getVersion(stream);
    }
    inline void deserialize(JsonValue stream, ISerializable &value, ISerializeModifier *modifier)
    {
        value.deserialize(stream, modifier);
    }

    template <typename T>
    void deserialize(JsonValue stream, TkArray<T> &value, ISerializeModifier *modifier)
    {
        if (stream.isArray())
        {
            const i32 count = stream.getArrayCount();
            value.resize(count, false);
            for (i32 i = 0; i < count; ++i)
                deserialize(stream.getArrayItem(i), value[i], modifier);
        }
        else if constexpr (TIsTheSame<T, byte>::Value)
        {
            JsonTools::getBlob(stream, value);
        }
    }

    template <typename KeyType, typename ValueType>
    void deserialize(JsonValue stream, TkHashMap<KeyType, ValueType> &value, ISerializeModifier *modifier)
    {
        value.clear();
        if (stream.isArray())
        {
            const i32 count = stream.getArrayCount();
            value.ensureCapacity(count);
            for (i32 i = 0; i < count; ++i)
            {
                JsonValue item = stream.getArrayItem(i);
                JsonValue keyNode = item.findMember("Key");
                JsonValue valueNode = item.findMember("Value");
                if (!keyNode || !valueNode)
                    continue;

                KeyType key{};
                deserialize(keyNode, key, modifier);
                deserialize(valueNode, value[key], modifier);
            }
            return;
        }

        if constexpr (TIsTheSame<KeyType, TkString>::Value)
        {
            if (!stream.isObject())
                return;

            const i32 count = stream.getMemberCount();
            value.ensureCapacity(count);
            for (i32 i = 0; i < count; ++i)
            {
                KeyType key = stream.getMemberName(i).toString();
                deserialize(stream.getMemberValue(i), value[key], modifier);
            }
        }
    }

    template <typename T>
    void deserialize(JsonValue stream, T &value, ISerializeModifier *modifier)
    {
        if constexpr (TEnumType<TDecayT<T>>)
            value = static_cast<T>(deserializeUint(stream));
        else if constexpr (TIsBaseOf<ISerializable, T>::Value)
            deserialize(stream, static_cast<ISerializable &>(value), modifier);
        else if (!deserializeCustom(stream, value, modifier))
            TK_ASSERT(false && "Missing deserialization overload for type");
    }

    template <typename T>
    TkString saveToString(const T &value, bool pretty = false)
    {
        TkString result;
        if (pretty)
        {
            PrettyJsonWriter writer(result);
            serialize(writer, value, nullptr);
        }
        else
        {
            CompactJsonWriter writer(result);
            serialize(writer, value, nullptr);
        }
        return result;
    }

    template <typename T>
    bool loadFromString(T &value, TkStringView text, ISerializeModifier *modifier = nullptr)
    {
        JsonDocument document;
        if (!document.parse(text))
            return false;
        deserialize(document.getRoot(), value, modifier);
        return true;
    }

    template <typename T>
    TkArray<byte> saveToBytes(const T &value, bool pretty = false)
    {
        TkString text = saveToString(value, pretty);
        TkArray<byte> result;
        result.resize(text.length(), false);
        if (text.length() > 0)
            TkMemory::memoryCopy(result.get(), text.get(), static_cast<psize>(text.length()));
        return result;
    }

    template <typename T>
    bool loadFromBytes(T &value, TkSpan<const byte> bytes, ISerializeModifier *modifier = nullptr)
    {
        return loadFromString(value, TkStringView(reinterpret_cast<const char *>(bytes.get()), bytes.length()), modifier);
    }

    template <typename T>
    bool loadFromBytes(T &value, const TkArray<byte> &bytes, ISerializeModifier *modifier = nullptr)
    {
        return loadFromBytes(value, TkSpan<const byte>(bytes.get(), bytes.count()), modifier);
    }

    inline void serializeBlob(JsonWriter &stream, TkSpan<const byte> data)
    {
        stream.blob(data);
    }

    inline void deserializeBlob(JsonValue stream, TkArray<byte> &output)
    {
        JsonTools::getBlob(stream, output);
    }
}

#define SERIALIZE_CUSTOM(TYPE, ...)                                                                 \
    namespace toolkit                                                                               \
    {                                                                                               \
        template <>                                                                                 \
        inline bool serializeCustom<TYPE>(JsonWriter & stream, const TYPE &t, const void *otherObj) \
        {                                                                                           \
            (void)stream;                                                                           \
            (void)t;                                                                                \
            (void)otherObj;                                                                         \
            __VA_ARGS__;                                                                            \
            return true;                                                                            \
        }                                                                                           \
    }

#define DESERIALIZE_CUSTOM(TYPE, ...)                                                                \
    namespace toolkit                                                                                \
    {                                                                                                \
        template <>                                                                                  \
        inline bool deserializeCustom<TYPE>(JsonValue stream, TYPE &t, ISerializeModifier *modifier) \
        {                                                                                            \
            (void)stream;                                                                            \
            (void)t;                                                                                 \
            (void)modifier;                                                                          \
            __VA_ARGS__;                                                                             \
            return true;                                                                             \
        }                                                                                            \
    }

#define SHOULD_SERIALIZE_CUSTOM(TYPE, ...)                                           \
    namespace toolkit                                                                \
    {                                                                                \
        template <>                                                                  \
        inline bool shouldSerializeCustom<TYPE>(const TYPE &t, const void *otherObj) \
        {                                                                            \
            (void)t;                                                                 \
            (void)otherObj;                                                          \
            __VA_ARGS__;                                                             \
        }                                                                            \
    }
