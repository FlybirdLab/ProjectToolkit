/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "JsonWriters.hpp"

#include <toolkit/memory/Memory.hpp>

#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>

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

    using RapidBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, RapidJsonAllocator>;
    using RapidWriter = rapidjson::Writer<RapidBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, RapidJsonAllocator>;
    using RapidPrettyWriter = rapidjson::PrettyWriter<RapidBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, RapidJsonAllocator>;

    template <typename WriterType>
    void finishWriter(TkString &output, void *buffer, void *writer)
    {
        (void)writer;
        auto *rapidBuffer = static_cast<RapidBuffer *>(buffer);
        output.set(rapidBuffer->GetString(), static_cast<i32>(rapidBuffer->GetSize()));
    }

    CompactJsonWriter::CompactJsonWriter(TkString &output)
        : m_output(output)
    {
        m_buffer = TkMemory::newObject<RapidBuffer>(TkMemory::getDefault());
        m_writer = TkMemory::newObject<RapidWriter>(TkMemory::getDefault(), *static_cast<RapidBuffer *>(m_buffer));
    }

    CompactJsonWriter::~CompactJsonWriter()
    {
        finishWriter<RapidWriter>(m_output, m_buffer, m_writer);
        TkMemory::deletePtr(TkMemory::getDefault(), static_cast<RapidWriter *>(m_writer));
        TkMemory::deletePtr(TkMemory::getDefault(), static_cast<RapidBuffer *>(m_buffer));
    }

    void CompactJsonWriter::key(const char *str, i32 length)
    {
        static_cast<RapidWriter *>(m_writer)->Key(str, static_cast<rapidjson::SizeType>(length));
    }
    void CompactJsonWriter::string(const char *str, i32 length)
    {
        static_cast<RapidWriter *>(m_writer)->String(str, static_cast<rapidjson::SizeType>(length));
    }
    void CompactJsonWriter::rawValue(const char *json, i32 length)
    {
        static_cast<RapidWriter *>(m_writer)->RawValue(json, static_cast<rapidjson::SizeType>(length), rapidjson::kObjectType);
    }
    void CompactJsonWriter::boolValue(bool value)
    {
        static_cast<RapidWriter *>(m_writer)->Bool(value);
    }
    void CompactJsonWriter::intValue(i32 value)
    {
        static_cast<RapidWriter *>(m_writer)->Int(value);
    }
    void CompactJsonWriter::int64Value(i64 value)
    {
        static_cast<RapidWriter *>(m_writer)->Int64(value);
    }
    void CompactJsonWriter::uintValue(u32 value)
    {
        static_cast<RapidWriter *>(m_writer)->Uint(value);
    }
    void CompactJsonWriter::uint64Value(u64 value)
    {
        static_cast<RapidWriter *>(m_writer)->Uint64(value);
    }
    void CompactJsonWriter::floatValue(f32 value)
    {
        static_cast<RapidWriter *>(m_writer)->Double(value);
    }
    void CompactJsonWriter::doubleValue(f64 value)
    {
        static_cast<RapidWriter *>(m_writer)->Double(value);
    }
    void CompactJsonWriter::nullValue()
    {
        static_cast<RapidWriter *>(m_writer)->Null();
    }
    void CompactJsonWriter::startObject()
    {
        static_cast<RapidWriter *>(m_writer)->StartObject();
    }
    void CompactJsonWriter::endObject()
    {
        static_cast<RapidWriter *>(m_writer)->EndObject();
    }
    void CompactJsonWriter::startArray()
    {
        static_cast<RapidWriter *>(m_writer)->StartArray();
    }
    void CompactJsonWriter::endArray(i32 count)
    {
        static_cast<RapidWriter *>(m_writer)->EndArray(static_cast<rapidjson::SizeType>(count));
    }

    PrettyJsonWriter::PrettyJsonWriter(TkString &output)
        : m_output(output)
    {
        m_buffer = TkMemory::newObject<RapidBuffer>(TkMemory::getDefault());
        m_writer = TkMemory::newObject<RapidPrettyWriter>(TkMemory::getDefault(), *static_cast<RapidBuffer *>(m_buffer));
        static_cast<RapidPrettyWriter *>(m_writer)->SetIndent('\t', 1);
    }

    PrettyJsonWriter::~PrettyJsonWriter()
    {
        finishWriter<RapidPrettyWriter>(m_output, m_buffer, m_writer);
        TkMemory::deletePtr(TkMemory::getDefault(), static_cast<RapidPrettyWriter *>(m_writer));
        TkMemory::deletePtr(TkMemory::getDefault(), static_cast<RapidBuffer *>(m_buffer));
    }

    void PrettyJsonWriter::key(const char *str, i32 length)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Key(str, static_cast<rapidjson::SizeType>(length));
    }
    void PrettyJsonWriter::string(const char *str, i32 length)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->String(str, static_cast<rapidjson::SizeType>(length));
    }
    void PrettyJsonWriter::rawValue(const char *json, i32 length)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->RawValue(json, static_cast<rapidjson::SizeType>(length), rapidjson::kObjectType);
    }
    void PrettyJsonWriter::boolValue(bool value)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Bool(value);
    }
    void PrettyJsonWriter::intValue(i32 value)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Int(value);
    }
    void PrettyJsonWriter::int64Value(i64 value)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Int64(value);
    }
    void PrettyJsonWriter::uintValue(u32 value)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Uint(value);
    }
    void PrettyJsonWriter::uint64Value(u64 value)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Uint64(value);
    }
    void PrettyJsonWriter::floatValue(f32 value)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Double(value);
    }
    void PrettyJsonWriter::doubleValue(f64 value)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Double(value);
    }
    void PrettyJsonWriter::nullValue()
    {
        static_cast<RapidPrettyWriter *>(m_writer)->Null();
    }
    void PrettyJsonWriter::startObject()
    {
        static_cast<RapidPrettyWriter *>(m_writer)->StartObject();
    }
    void PrettyJsonWriter::endObject()
    {
        static_cast<RapidPrettyWriter *>(m_writer)->EndObject();
    }
    void PrettyJsonWriter::startArray()
    {
        static_cast<RapidPrettyWriter *>(m_writer)->StartArray();
    }
    void PrettyJsonWriter::endArray(i32 count)
    {
        static_cast<RapidPrettyWriter *>(m_writer)->EndArray(static_cast<rapidjson::SizeType>(count));
    }
}
