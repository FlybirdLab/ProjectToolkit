/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "UUID.hpp"

#include "String.hpp"
#include "toolkit/base/Assert.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/platforms/System.hpp"

namespace toolkit
{

    TkUuid TkUuid::Empty;

    TkString TkUuid::toString() const
    {
        return toString(UuidFormatType::kN);
    }

    TkString TkUuid::toString(UuidFormatType format) const
    {
        switch (format)
        {
        case UuidFormatType::kN:
            return TkString::format("%08x%08x%08x%08x", a, b, c, d);
        case UuidFormatType::kB:
            return TkString::format("{%08x-%04x-%04x-%04x-%04x%08x}", a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        case UuidFormatType::kP:
            return TkString::format("(%08x-%04x-%04x-%04x-%04x%08x)", a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        default:
            return TkString::format("%08x-%04x-%04x-%04x-%04x%08x", a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        }
    }

    template <typename CharType>
    TK_FORCE_INLINE void uuidToString(const CharType *cachedTkUuidDigits, CharType *buffer, const TkUuid &value, UuidFormatType format)
    {
        switch (format)
        {
        case UuidFormatType::kN:
        {
            for (i32 i = 0; i < 32; i++)
                buffer[i] = '0';
            buffer[32] = 0;
            u32 n = value.a;
            CharType *p = buffer + 7;
            do
            {
                *p-- = cachedTkUuidDigits[(i32)(n & 0xf)];
            } while ((n >>= 4) != 0);
            n = value.b;
            p = buffer + 15;
            do
            {
                *p-- = cachedTkUuidDigits[(i32)(n & 0xf)];
            } while ((n >>= 4) != 0);
            n = value.c;
            p = buffer + 23;
            do
            {
                *p-- = cachedTkUuidDigits[(i32)(n & 0xf)];
            } while ((n >>= 4) != 0);
            n = value.d;
            p = buffer + 31;
            do
            {
                *p-- = cachedTkUuidDigits[(i32)(n & 0xf)];
            } while ((n >>= 4) != 0);
            break;
        }
        case UuidFormatType::kB:
            // TODO: impl TkUuidtoString for UuidFormatType::kB:
        case UuidFormatType::kP:
            // TODO: impl TkUuidtoString for UuidFormatType::kp:
        default:
            // TODO: impl TkUuidtoString for UuidFormatType::kD:
            TK_ASSERT("Missing TkUuidtoString impl.");
        }
    }

    void TkUuid::toString(char *buffer, UuidFormatType format) const
    {
        const static char *CachedTkUuidDigits = "0123456789abcdef";
        return uuidToString<char>(CachedTkUuidDigits, buffer, *this, format);
    }

    template <typename TkStringType, typename TkStringViewType>
    TK_FORCE_INLINE bool uuidParse(const TkStringViewType &text, TkUuid &value)
    {
        switch (text.length())
        {
            // UuidFormatType::kN
        case 32:
        {
            return TkStringUtils::fromHexString(*text + 0, 8, value.a) &&
                   TkStringUtils::fromHexString(*text + 8, 8, value.b) &&
                   TkStringUtils::fromHexString(*text + 16, 8, value.c) &&
                   TkStringUtils::fromHexString(*text + 24, 8, value.d);
        }
        // UuidFormatType::kD
        case 36:
        {
            TkStringType b = TkStringType(text.substring(9, 4)) + text.substring(14, 4);
            TkStringType c = TkStringType(text.substring(19, 4)) + text.substring(24, 4);
            return TkStringUtils::fromHexString(*text + 0, 8, value.a) &&
                   TkStringUtils::fromHexString(*b, value.b) &&
                   TkStringUtils::fromHexString(*c, value.c) &&
                   TkStringUtils::fromHexString(*text + 28, 8, value.d);
        }
        // UuidFormatType::kB
        // UuidFormatType::kp
        case 38:
        {
            TkStringType b = TkStringType(text.substring(10, 4)) + text.substring(15, 4);
            TkStringType c = TkStringType(text.substring(20, 4)) + text.substring(25, 4);
            return text[0] != text[text.length() - 1] &&
                   TkStringUtils::fromHexString(*text + 1, 8, value.a) &&
                   TkStringUtils::fromHexString(*b, value.b) &&
                   TkStringUtils::fromHexString(*c, value.c) &&
                   TkStringUtils::fromHexString(*text + 29, 8, value.d);
        }
        default:
            return false;
        }
    }

    bool TkUuid::parse(const TkStringView &text, TkUuid &value)
    {
        return uuidParse<TkString, TkStringView>(text, value);
    }

    TkUuid TkUuid::create()
    {
        TkUuid result;
        System::createUuid(result);
        return result;
    }
}