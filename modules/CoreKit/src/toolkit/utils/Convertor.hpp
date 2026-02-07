/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References:
 *     https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
 * Test target: ConvertorTest.cpp
 */

#pragma once

#include "toolkit/memory/Memory.hpp"
#include <toolkit/common.hpp>
#include <toolkit/types/Span.hpp>
#include <toolkit/types/String.hpp>

#if _MSC_VER && TK_SIMD_SSE4_2
#include <intrin.h>
#endif

namespace toolkit
{

    class TkConvertor
    {
    public:
        struct Private
        {
            static TkSpan<const char *> BytesSizes;
            static TkSpan<const char *> HertzSizes;
        };

        // Round floating point value up to 1 decimal place
        template <typename T>
        TK_FORCE_INLINE static T roundTo1DecimalPlace(T value)
        {
            return (T)round((double)value * 10) / (T)10;
        }

        // Round floating point value up to 2 decimal places
        template <typename T>
        TK_FORCE_INLINE static T roundTo2DecimalPlaces(T value)
        {
            return (T)round((double)value * 100.0) / (T)100;
        }

        // Round floating point value up to 3 decimal places
        template <typename T>
        TK_FORCE_INLINE static T roundTo3DecimalPlaces(T value)
        {
            return (T)round((double)value * 1000.0) / (T)1000;
        }

        // Converts units to the best fitting human-readable denominator.
        template <typename T>
        static TkString unitsToText(T units, i32 divider, const TkSpan<const char *> sizes)
        {
            if (sizes.length() == 0)
                return TkString::format("{0}", units);
            i32 i = 0;
            double dblSUnits = static_cast<double>(units);
            for (; static_cast<u64>(units / static_cast<double>(divider)) > 0; i++, units /= divider)
                dblSUnits = (double)units / (double)divider;
            if (i >= sizes.length())
                i = 0;
            TkString text = TkString::format("{}", roundTo2DecimalPlaces(dblSUnits));
            const i32 dot = text.findLast('.');
            if (dot != -1)
                text = text.left(dot + 3);
            return TkString::format("{0} {1}", text, sizes[i]);
        }

        // Converts size of the data (in bytes) to the best fitting string.
        template <typename T>
        static TkString bytesToText(T bytes)
        {
            return unitsToText(bytes, 1024, Private::BytesSizes);
        }

        // Converts hertz to the best fitting string.
        template <typename T>
        static TkString hertzToText(T hertz)
        {
            return unitsToText(hertz, 1000, Private::HertzSizes);
        }

        // Returns the amount of set bits in 32-bit integer.
        static inline i32 countBits(u32 x)
        {
#ifdef __GNUC_
            return __builtin_popcount(x);
#elif _MSC_VER && PLATFORM_SIMD_SSE4_2
            return __popcnt(x);
#else
            x = x - ((x >> 1) & 0x55555555);
            x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
            x = (x + (x >> 4)) & 0x0F0F0F0F;
            return (x * 0x01010101) >> 24;
#endif
        }

        // Copy memory region but ignoring address sanatizer checks for memory regions.
        static TK_NO_SANITIZE_ADDRESS void unsafeMemoryCopy(void *dst, const void *src, u64 size)
        {
#if !TK_DEBUG
            memcpy(dst, src, static_cast<size_t>(size));
#else
            for (u64 i = 0; i < size; i++)
                ((byte *)dst)[i] = ((byte *)src)[i];
#endif
        }

    public:
        static void ansiToUtf16(const char *from, wchar *to, i32 fromLength, i32 &toLength);
        static void utf16ToAnsi(const wchar *from, char *to, i32 len);

        static void utf8ToUtf16(const char *from, wchar *to, i32 fromLength, i32 &toLength);
        static wchar *utf8ToUtf16(const char *from, i32 fromLength, i32 &toLength);

        static void utf16ToUtf8(const wchar *from, char *to, i32 fromLength, i32 &toLength);
        static char *utf16ToUtf8(const wchar *from, i32 fromLength, i32 &toLength);

    public:
        i32 base64EncodeLength(i32 size);
        i32 base64DecodeLength(const char *encoded, i32 length);

        void base64Encode(const byte *bytes, i32 size, TkArray<char> &encoded);
        void base64Encode(const byte *bytes, i32 size, char *encoded);

        void base64Decode(const char *encoded, i32 length, TkArray<byte> &output);
        void base64Decode(const char *encoded, i32 length, byte *output);

        void encryptBytes(byte *data, u64 size);
        void decryptBytes(byte *data, u64 size);
    };

    constexpr i32 kUtf8MaxPerUtf16 = 4;
    constexpr i32 kUtf16MaxPerUtf8 = 1;

    template <typename CharType, int InlinedSize = 128>
    class StringAsBase
    {
    public:
        ~StringAsBase()
        {
            if (m_dynamic)
                TkMemory::free(TkMemory::getDefault(), m_dynamic);
        }

    public:
        const CharType *get() const
        {
            return m_static ? m_static : (m_dynamic ? m_dynamic : m_inlined);
        }

        i32 length() const
        {
            return TkStringUtils::length(get());
        }

    protected:
        const CharType *m_static = nullptr;
        CharType *m_dynamic = nullptr;
        CharType m_inlined[InlinedSize];
    };

    template <int InlinedSize = 128>
    class StringAsANSI : public StringAsBase<char, InlinedSize>
    {
    public:
        StringAsANSI(const char *text)
        {
            this->m_static = text;
        }

        StringAsANSI(const wchar *text)
            : StringAsANSI(text, TkStringUtils::length(text))
        {
        }

        StringAsANSI(const wchar *text, i32 length)
        {
            // i32 requiredBytes = (length * 4) + 1;
            if (length * kUtf8MaxPerUtf16 + 1 <= InlinedSize)
            {
                TkConvertor::utf16ToAnsi(text, this->m_inlined, InlinedSize);
            }
            else
            {
                this->m_dynamic = (CharType *)TkMemory::allocate(TkMemory::getDefault(), length * kUtf8MaxPerUtf16 + 1);
                TkConvertor::utf16ToAnsi(text, this->m_dynamic, length + 1);
            }
        }

    public:
        typedef char CharType;
        typedef StringAsBase<CharType, InlinedSize> Base;
    };

    template <int InlinedSize = 128>
    class StringAsUTF8 : public StringAsBase<char, InlinedSize>
    {
    public:
        StringAsUTF8(const char *text)
        {
            this->m_static = text;
        }

        StringAsUTF8(const wchar *text)
            : StringAsUTF8(text, TkStringUtils::length(text))
        {
        }

        StringAsUTF8(const wchar *text, i32 length)
        {
            i32 lengthUtf8;
            if (length + 1 < InlinedSize)
            {
                TkConvertor::utf16ToUtf8(text, this->m_inlined, length, lengthUtf8);
                this->m_inlined[lengthUtf8] = 0;
            }
            else
            {
                this->m_dynamic = TkConvertor::utf16ToUtf8(text, length, lengthUtf8);
                this->m_dynamic[lengthUtf8] = 0;
            }
        }

    public:
        typedef char CharType;
        typedef StringAsBase<CharType, InlinedSize> Base;
    };

    template <int InlinedSize = 128>
    class StringAsUTF16 : public StringAsBase<wchar, InlinedSize>
    {
    public:
        StringAsUTF16(const char *text)
            : StringAsUTF16(text, TkStringUtils::length(text))
        {
        }

        StringAsUTF16(const char *text, i32 length)
        {
            if (length + 1 < InlinedSize)
            {
                i32 outLen;
                TkConvertor::utf8ToUtf16(text, this->m_inlined, length, outLen);
                this->m_inlined[outLen] = 0;
            }
            else
            {
                i32 outLen;
                this->m_dynamic = TkConvertor::utf8ToUtf16(text, length, outLen);
                this->m_dynamic[outLen] = 0;
            }
        }

        StringAsUTF16(const wchar *text)
        {
            this->m_static = text;
        }

    public:
        typedef wchar CharType;
        typedef StringAsBase<CharType, InlinedSize> Base;
    };

    template <typename CharType = wchar>
    class StringAsTerminated
    {
    public:
        StringAsTerminated(const CharType *str, i32 length)
        {
            if (length != 0 && str[length] == 0) // Unsafe to access out of bounds...
            {
                m_static = str;
            }
            else
            {
                m_dynamic = (CharType *)TkMemory::allocate(TkMemory::getDefault(), (length + 1) * sizeof(CharType));
                TkMemory::memoryCopy(m_dynamic, str, length * sizeof(CharType));
                m_dynamic[length] = 0;
            }
        }

        ~StringAsTerminated()
        {
            TkMemory::free(TkMemory::getDefault(), m_dynamic);
        }

        operator const CharType *() const
        {
            return m_static ? m_static : m_dynamic;
        }

    protected:
        const CharType *m_static = nullptr;
        CharType *m_dynamic = nullptr;
    };
}