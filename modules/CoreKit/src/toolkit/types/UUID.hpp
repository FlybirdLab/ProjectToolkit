/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/base/Format.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/base/Hash.hpp>

namespace toolkit
{
    // Accepted format specifiers for the format parameter.
    enum class UuidFormatType : u8
    {
        kN, // 00000000000000000000000000000000
        kD, // 00000000-0000-0000-0000-000000000000
        kB, // {00000000-0000-0000-0000-000000000000}
        kP, // (00000000-0000-0000-0000-000000000000)
    };

    class TkUuid final
    {
    public:
        union
        {
            struct
            {
                u32 a;
                u32 b;
                u32 c;
                u32 d;
            };

            byte raw[16];
            u32 values[4];
        };

    public:
        static TkUuid Empty;

    public:
        TkUuid() = default;
        TkUuid(u32 a, u32 b, u32 c, u32 d)
            : a(a), b(b), c(c), d(d) {}

    public:
        bool operator==(const TkUuid &other) const
        {
            return ((a ^ other.a) | (b ^ other.b) | (c ^ other.c) | (d ^ other.d)) == 0;
        }
        bool operator!=(const TkUuid &other) const
        {
            return ((a ^ other.a) | (b ^ other.b) | (c ^ other.c) | (d ^ other.d)) != 0;
        }

        u32 &operator[](i32 index)
        {
            TK_ASSERT(index >= 0 && index < 4);
            return values[index];
        }
        const u32 &operator[](int index) const
        {
            TK_ASSERT(index >= 0 && index < 4);
            return values[index];
        }

        TK_FORCE_INLINE bool isValid() const
        {
            return (a | b | c | d) != 0;
        }
        explicit operator bool() const
        {
            return (a | b | c | d) != 0;
        }

    public:
        TkString toString() const;
        TkString toString(UuidFormatType format) const;
        void toString(char *buffer, UuidFormatType format) const;

    public:
        static bool parse(const TkStringView &text, TkUuid &value);

        static TkUuid create();
    };

    template <>
    struct TIsPODType<TkUuid>
    {
        enum
        {
            Value = true
        };
    };

    template <>
    struct HashFunc<TkUuid>
    {
        static u32 get(const TkUuid &key)
        {
            return static_cast<u32>(
                key.a ^ key.b ^ key.c ^ key.d);
        }
    };

}

FORMAT_CUSTOM(TkUuid, "%08x%08x%08x%08x", t.a, t.b, t.c, t.d);