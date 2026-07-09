/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/serialization/JsonWriter.hpp>

namespace toolkit
{
    class CompactJsonWriter final : public JsonWriter
    {
    public:
        using JsonWriter::key;
        using JsonWriter::rawValue;
        using JsonWriter::string;

        explicit CompactJsonWriter(TkString &output);
        ~CompactJsonWriter() override;

        void key(const char *str, i32 length) override;
        void string(const char *str, i32 length) override;
        void rawValue(const char *json, i32 length) override;
        void boolValue(bool value) override;
        void intValue(i32 value) override;
        void int64Value(i64 value) override;
        void uintValue(u32 value) override;
        void uint64Value(u64 value) override;
        void floatValue(f32 value) override;
        void doubleValue(f64 value) override;
        void nullValue() override;

        void startObject() override;
        void endObject() override;
        void startArray() override;
        void endArray(i32 count = 0) override;

    private:
        void *m_buffer = nullptr;
        void *m_writer = nullptr;
        TkString &m_output;
    };

    class PrettyJsonWriter final : public JsonWriter
    {
    public:
        using JsonWriter::key;
        using JsonWriter::rawValue;
        using JsonWriter::string;

        explicit PrettyJsonWriter(TkString &output);
        ~PrettyJsonWriter() override;

        void key(const char *str, i32 length) override;
        void string(const char *str, i32 length) override;
        void rawValue(const char *json, i32 length) override;
        void boolValue(bool value) override;
        void intValue(i32 value) override;
        void int64Value(i64 value) override;
        void uintValue(u32 value) override;
        void uint64Value(u64 value) override;
        void floatValue(f32 value) override;
        void doubleValue(f64 value) override;
        void nullValue() override;

        void startObject() override;
        void endObject() override;
        void startArray() override;
        void endArray(i32 count = 0) override;

    private:
        void *m_buffer = nullptr;
        void *m_writer = nullptr;
        TkString &m_output;
    };
}
