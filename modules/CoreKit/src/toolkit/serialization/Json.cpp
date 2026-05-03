/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Json.hpp"

#include <toolkit/base/Assert.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/UUID.hpp>

#include <rapidjson/error/en.h>
#include <rapidjson/document.h>

namespace toolkit
{
    struct RapidJsonAllocator
    {
        static const bool kNeedFree = true;

        void *Malloc(size_t size)
        {
            return size ? TkMemory::allocate(TkMemory::getDefault(), static_cast<psize>(size)) : nullptr;
        }

        void *Realloc(void *originalPtr, size_t originalSize, size_t newSize)
        {
            return TkMemory::reallocate(
                TkMemory::getDefault(),
                originalPtr,
                static_cast<psize>(newSize),
                static_cast<psize>(originalSize));
        }

        static void Free(void *ptr)
        {
            TkMemory::free(TkMemory::getDefault(), ptr);
        }
    };

    using RapidDocument = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<RapidJsonAllocator>, RapidJsonAllocator>;
    using RapidValue = RapidDocument::ValueType;

    const RapidValue *asValue(const void *value)
    {
        return static_cast<const RapidValue *>(value);
    }

    RapidValue *asValue(void *value)
    {
        return static_cast<RapidValue *>(value);
    }

    RapidDocument *asDocument(void *document)
    {
        return static_cast<RapidDocument *>(document);
    }

    const RapidDocument *asDocument(const void *document)
    {
        return static_cast<const RapidDocument *>(document);
    }

    void changeIdsRecursive(RapidValue &value, RapidDocument &document, const TkHashMap<TkUuid, TkUuid> &mapping)
    {
        if (value.IsObject())
        {
            for (auto itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr)
                changeIdsRecursive(itr->value, document, mapping);
            return;
        }

        if (value.IsArray())
        {
            for (auto &item : value.GetArray())
                changeIdsRecursive(item, document, mapping);
            return;
        }

        if (!value.IsString() || value.GetStringLength() != 32)
            return;

        TkUuid uuid;
        if (!TkUuid::parse(TkStringView(value.GetString(), static_cast<i32>(value.GetStringLength())), uuid))
            return;

        auto mapped = mapping.find(uuid);
        if (mapped == mapping.End())
            return;

        char buffer[33];
        mapped->value.toString(buffer, UuidFormatType::kN);
        value.SetString(buffer, 32, document.GetAllocator());
    }

    bool JsonValue::isNull() const
    {
        return !m_value || asValue(m_value)->IsNull();
    }

    bool JsonValue::isBool() const
    {
        return m_value && asValue(m_value)->IsBool();
    }

    bool JsonValue::isInt() const
    {
        return m_value && asValue(m_value)->IsInt();
    }

    bool JsonValue::isUint() const
    {
        return m_value && asValue(m_value)->IsUint();
    }

    bool JsonValue::isInt64() const
    {
        return m_value && asValue(m_value)->IsInt64();
    }

    bool JsonValue::isUint64() const
    {
        return m_value && asValue(m_value)->IsUint64();
    }

    bool JsonValue::isNumber() const
    {
        return m_value && asValue(m_value)->IsNumber();
    }

    bool JsonValue::isString() const
    {
        return m_value && asValue(m_value)->IsString();
    }

    bool JsonValue::isArray() const
    {
        return m_value && asValue(m_value)->IsArray();
    }

    bool JsonValue::isObject() const
    {
        return m_value && asValue(m_value)->IsObject();
    }

    JsonValueType JsonValue::getType() const
    {
        if (!m_value || asValue(m_value)->IsNull())
            return JsonValueType::kNull;
        if (asValue(m_value)->IsBool())
            return JsonValueType::kBool;
        if (asValue(m_value)->IsNumber())
            return JsonValueType::kNumber;
        if (asValue(m_value)->IsString())
            return JsonValueType::kString;
        if (asValue(m_value)->IsArray())
            return JsonValueType::kArray;
        return JsonValueType::kObject;
    }

    bool JsonValue::getBool(bool defaultValue) const
    {
        return isBool() ? asValue(m_value)->GetBool() : defaultValue;
    }

    i32 JsonValue::getInt(i32 defaultValue) const
    {
        return m_value && asValue(m_value)->IsInt() ? asValue(m_value)->GetInt() : defaultValue;
    }

    u32 JsonValue::getUint(u32 defaultValue) const
    {
        return m_value && asValue(m_value)->IsUint() ? asValue(m_value)->GetUint() : defaultValue;
    }

    i64 JsonValue::getInt64(i64 defaultValue) const
    {
        return m_value && asValue(m_value)->IsInt64() ? asValue(m_value)->GetInt64() : defaultValue;
    }

    u64 JsonValue::getUint64(u64 defaultValue) const
    {
        return m_value && asValue(m_value)->IsUint64() ? asValue(m_value)->GetUint64() : defaultValue;
    }

    f32 JsonValue::getFloat(f32 defaultValue) const
    {
        return isNumber() ? asValue(m_value)->GetFloat() : defaultValue;
    }

    f64 JsonValue::getDouble(f64 defaultValue) const
    {
        return isNumber() ? asValue(m_value)->GetDouble() : defaultValue;
    }

    TkStringView JsonValue::getString() const
    {
        if (!isString())
            return TkStringView::Empty;
        return TkStringView(asValue(m_value)->GetString(), static_cast<i32>(asValue(m_value)->GetStringLength()));
    }

    i32 JsonValue::getArrayCount() const
    {
        return isArray() ? static_cast<i32>(asValue(m_value)->Size()) : 0;
    }

    JsonValue JsonValue::getArrayItem(i32 index) const
    {
        if (!isArray() || index < 0 || index >= static_cast<i32>(asValue(m_value)->Size()))
            return JsonValue();
        return JsonValue(&(*asValue(m_value))[index]);
    }

    i32 JsonValue::getMemberCount() const
    {
        return isObject() ? static_cast<i32>(asValue(m_value)->MemberCount()) : 0;
    }

    TkStringView JsonValue::getMemberName(i32 index) const
    {
        if (!isObject() || index < 0 || index >= getMemberCount())
            return TkStringView::Empty;
        auto itr = asValue(m_value)->MemberBegin() + index;
        return TkStringView(itr->name.GetString(), static_cast<i32>(itr->name.GetStringLength()));
    }

    JsonValue JsonValue::getMemberValue(i32 index) const
    {
        if (!isObject() || index < 0 || index >= getMemberCount())
            return JsonValue();
        auto itr = asValue(m_value)->MemberBegin() + index;
        return JsonValue(&itr->value);
    }

    JsonValue JsonValue::findMember(TkStringView name) const
    {
        if (!isObject())
            return JsonValue();
        auto itr = asValue(m_value)->FindMember(rapidjson::GenericStringRef<char>(name.getText(), static_cast<rapidjson::SizeType>(name.length())));
        return itr != asValue(m_value)->MemberEnd() ? JsonValue(&itr->value) : JsonValue();
    }

    bool JsonValue::hasMember(TkStringView name) const
    {
        return static_cast<bool>(findMember(name));
    }

    JsonDocument::JsonDocument()
    {
        m_document = TkMemory::newObject<RapidDocument>(TkMemory::getDefault());
    }

    JsonDocument::~JsonDocument()
    {
        clear();
        TkMemory::deletePtr(TkMemory::getDefault(), asDocument(m_document));
        m_document = nullptr;
    }

    bool JsonDocument::parse(TkStringView text)
    {
        clear();
        RapidDocument *document = asDocument(m_document);
        document->Parse(text.getText(), static_cast<size_t>(text.length()));
        if (!document->HasParseError())
            return true;

        m_error = rapidjson::GetParseError_En(document->GetParseError());
        return false;
    }

    void JsonDocument::clear()
    {
        asDocument(m_document)->SetNull();
        m_error.clear();
    }

    JsonValue JsonDocument::getRoot() const
    {
        return JsonValue(asDocument(m_document));
    }

    TkStringView JsonDocument::getError() const
    {
        return TkStringView(m_error);
    }

    void JsonDocument::changeIds(const TkHashMap<TkUuid, TkUuid> &mapping)
    {
        if (!mapping.isEmpty())
            changeIdsRecursive(*asDocument(m_document), *asDocument(m_document), mapping);
    }
}
