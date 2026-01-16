/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References:
 *      https://stackoverflow.com/questions/7153935/how-to-convert-utf-8-stdstring-to-utf-16-stdwstring
 *      https://stackoverflow.com/questions/21456926/how-do-i-convert-a-string-in-utf-16-to-utf-8-in-c
 */

#include "Convertor.hpp"
#include "toolkit/memory/Memory.hpp"

#include <toolkit/collection/Array.hpp>
#include <toolkit/debug/Logger.hpp>

#if TK_POSIX

#include <wctype.h>
#include <cctype>
#include <wchar.h>
#include <cstring>
#include <stdlib.h>

#endif

namespace toolkit
{

    TkSpan<const char *> TkConvertor::Private::BytesSizes {};
    TkSpan<const char *> TkConvertor::Private::HertzSizes {};

#if TK_POSIX

    static inline i32 calU8CodePointLen(char ch)
    {
        return ((0xe5000000 >> ((ch >> 3) & 0x1e)) & 3) + 1;
    }

    static inline void u8ShiftAndMask(u32 *codePoint, const char byte)
    {
        *codePoint <<= 6;
        *codePoint |= 0x3F & byte;
    }

    static inline u32 u8ToUtf32CodePoint(const char *src, i32 length)
    {
        u32 unicode;
        switch (length)
        {
        case 1:
            return src[0];
        case 2:
            unicode = src[0] & 0x1f;
            u8ShiftAndMask(&unicode, src[1]);
            return unicode;
        case 3:
            unicode = src[0] & 0x0f;
            u8ShiftAndMask(&unicode, src[1]);
            u8ShiftAndMask(&unicode, src[2]);
            return unicode;
        case 4:
            unicode = src[0] & 0x07;
            u8ShiftAndMask(&unicode, src[1]);
            u8ShiftAndMask(&unicode, src[2]);
            u8ShiftAndMask(&unicode, src[3]);
            return unicode;
        default:
            return 0xffff;
        }
    }

    static const char32_t kByteMask = 0x000000BF;
    static const char32_t kByteMark = 0x00000080;

    // Surrogates aren't valid for UTF-32 characters, so define some constants that will let us screen them out
    static const char32_t kUnicodeSurrogateHighStart = 0x0000D800;
    static const char32_t kUnicodeSurrogateLowEnd = 0x0000DFFF;
    static const char32_t kUnicodeSurrogateStart = kUnicodeSurrogateHighStart;
    static const char32_t kUnicodeSurrogateEnd = kUnicodeSurrogateLowEnd;
    static const char32_t kUnicodeMaxCodepoint = 0x0010FFFF;

    static const char32_t kFirstByteMark[] =
        {
            0x00000000,
            0x00000000,
            0x000000C0,
            0x000000E0,
            0x000000F0};

    // Return number of UTF-8 bytes required for the character, otherwise if the character is invalid, return size of 0
    static inline i32 utf32_codepoint_utf8_length(char32_t srcChar)
    {
        if (srcChar < 0x00000080)
        {
            return 1;
        }
        else if (srcChar < 0x00000800)
        {
            return 2;
        }
        else if (srcChar < 0x00010000)
        {
            if ((srcChar < kUnicodeSurrogateStart) || (srcChar > kUnicodeSurrogateEnd))
            {
                return 3;
            }
            else
            {
                return 0;
            }
        }
        else if (srcChar <= kUnicodeMaxCodepoint)
        {
            return 4;
        }
        else
        {
            return 0;
        }
    }

    // Write out the source character to dstP
    static inline void utf32_codepoint_to_utf8(byte *dstP, char32_t srcChar, i32 bytes)
    {
        dstP += bytes;
        switch (bytes)
        {
        case 4:
            *--dstP = (byte)((srcChar | kByteMark) & kByteMask);
            srcChar >>= 6;
        case 3:
            *--dstP = (byte)((srcChar | kByteMark) & kByteMask);
            srcChar >>= 6;
        case 2:
            *--dstP = (byte)((srcChar | kByteMark) & kByteMask);
            srcChar >>= 6;
        case 1:
            *--dstP = (byte)(srcChar | kFirstByteMark[bytes]);
        }
    }

#endif

    static void emitConvertError(const char *from, u32 fromLength)
    {
        LogError("Not a UTF-8 string. At: %i", fromLength);
        for (u32 i = 0; i < fromLength; i++)
        {
            LogError("str[%i] = %i", i, (u32)from[i]);
        }
    }

    void emitConvertError(const wchar *from, u32 fromLength)
    {
        LogError("Not a UTF-16 string. At: %i", fromLength);
        for (u32 i = 0; i < fromLength; i++)
        {
            LogError("str[%i] = %i", i, (u32)from[i]);
        }
    }

    static void utf8ToUtf16Helper(TkArray<u32> &unicode, const char *from, i32 fromLength, i32 &toLength)
    {
        unicode.ensureCapacity(fromLength);
        i32 i = 0, todo;
        u32 uni;
        toLength = 0;
        while (i < fromLength)
        {
            byte ch = from[i++];

            if (ch <= 0x7F)
            {
                uni = ch;
                todo = 0;
            }
            else if (ch <= 0xBF)
            {
                emitConvertError(from, fromLength);
                return;
            }
            else if (ch <= 0xDF)
            {
                uni = ch & 0x1F;
                todo = 1;
            }
            else if (ch <= 0xEF)
            {
                uni = ch & 0x0F;
                todo = 2;
            }
            else if (ch <= 0xF7)
            {
                uni = ch & 0x07;
                todo = 3;
            }
            else
            {
                emitConvertError(from, fromLength);
                return;
            }

            for (i32 j = 0; j < todo; j++)
            {
                if (i == fromLength)
                {
                    emitConvertError(from, fromLength);
                    return;
                }
                ch = from[i++];
                if (ch < 0x80 || ch > 0xBF)
                {
                    emitConvertError(from, fromLength);
                    return;
                }

                uni <<= 6;
                uni += ch & 0x3F;
            }

            if ((uni >= 0xD800 && uni <= 0xDFFF) || uni > 0x10FFFF)
            {
                emitConvertError(from, fromLength);
                return;
            }

            unicode.add(uni);

            toLength++;
            if (uni > 0xFFFF)
            {
                toLength++;
            }
        }
    }

    void utf16ToUtf8Helper(TkArray<u32> &unicode, const wchar *from, i32 fromLength, i32 &toLength)
    {
        unicode.ensureCapacity(fromLength);
        toLength = 0;
        i32 i = 0;
        while (i < fromLength)
        {
            u32 uni = from[i++];
            if (uni < 0xD800U || uni > 0xDFFFU)
            {
            }
            else if (uni >= 0xDC00U)
            {
                emitConvertError(from, fromLength);
                return;
            }
            else if (i + 1 == fromLength)
            {
                emitConvertError(from, fromLength);
                return;
            }
            else if (i < fromLength)
            {
                uni = (uni & 0x3FFU) << 10;
                if ((from[i] < 0xDC00U) || (from[i] > 0xDFFFU))
                {
                    emitConvertError(from, fromLength);
                    return;
                }
                uni |= from[i++] & 0x3FFU;
                uni += 0x10000U;
            }

            unicode.add(uni);

            toLength += uni <= 0x7FU ? 1 : uni <= 0x7FFU    ? 2
                                       : uni <= 0xFFFFU     ? 3
                                       : uni <= 0x1FFFFFU   ? 4
                                       : uni <= 0x3FFFFFFU  ? 5
                                       : uni <= 0x7FFFFFFFU ? 6
                                                            : 7;
        }
    }

    void TkConvertor::ansiToUtf16(const char *from, wchar *to, i32 fromLength, i32 &toLength)
    {
#if TK_WINDOWS
        if (fromLength)
            toLength = (i32)mbstowcs(to, from, fromLength);
        else
            toLength = 0;
#else
        const char *const u8end = from + fromLength;
        const char *u8cur = from;
        char16_t *u16cur = to;
        while (u8cur < u8end)
        {
            i32 len = calU8CodePointLen(*u8cur);
            u32 codepoint = u8ToUtf32CodePoint(u8cur, len);

            // Convert the UTF32 codepoint to one or more UTF16 codepoints
            if (codepoint <= 0xFFFF)
            {
                // Single UTF16 character
                *u16cur++ = (char16_t)codepoint;
            }
            else
            {
                // Multiple UTF16 characters with surrogates
                codepoint = codepoint - 0x10000;
                *u16cur++ = (char16_t)((codepoint >> 10) + 0xD800);
                *u16cur++ = (char16_t)((codepoint & 0x3FF) + 0xDC00);
            }
            u8cur += len;
        }
        toLength = (i32)(u16cur - to);
#endif
    }

    void TkConvertor::utf16ToAnsi(const wchar *from, char *to, i32 len)
    {
#if TK_WINDOWS
        if (len)
            wcstombs(to, from, len);
#else
        if (from == nullptr || len == 0 || to == nullptr)
            return;

        const char16_t *curUtf16 = from;
        const char16_t *const endUtf16 = from + len;
        char *cur = to;

        while (curUtf16 < endUtf16)
        {
            char32_t utf32;

            // Surrogate pairs
            if ((*curUtf16 & 0xFC00) == 0xD800)
            {
                utf32 = (*curUtf16++ - 0xD800) << 10;
                utf32 |= *curUtf16++ - 0xDC00;
                utf32 += 0x10000;
            }
            else
            {
                utf32 = (char32_t)*curUtf16++;
            }
            const i32 cLen = utf32_codepoint_utf8_length(utf32);
            utf32_codepoint_to_utf8((byte *)cur, utf32, cLen);
            cur += cLen;
        }

        *cur = '\0';
#endif
    }

    void TkConvertor::utf8ToUtf16(const char *from, wchar *to, i32 fromLength, i32 &toLength)
    {
        TkArray<u32> unicode;
        utf8ToUtf16Helper(unicode, from, fromLength, toLength);
        for (i32 i = 0, j = 0; j < unicode.count(); i++, j++)
        {
            u32 uni = unicode[j];
            if (uni <= 0xFFFF)
            {
                to[i] = (wchar)uni;
            }
            else
            {
                uni -= 0x10000;
                to[i++] += (wchar)((uni >> 10) + 0xD800);
                to[i] += (wchar)((uni & 0x3FF) + 0xDC00);
            }
        }
    }

    wchar *TkConvertor::utf8ToUtf16(const char *from, i32 fromLength, i32 &toLength)
    {
        TkArray<u32> unicode;
        utf8ToUtf16Helper(unicode, from, fromLength, toLength);
        if (toLength == 0)
            return nullptr;
        wchar *to = (wchar *)TkMemory::allocate(TkMemory::getDefault(), (toLength + 1) * sizeof(wchar)); // TODO
        for (i32 i = 0, j = 0; j < unicode.count(); i++, j++)
        {
            u32 uni = unicode[j];
            if (uni <= 0xFFFF)
            {
                to[i] = (wchar)uni;
            }
            else
            {
                uni -= 0x10000;
                to[i++] += (wchar)((uni >> 10) + 0xD800);
                to[i] += (wchar)((uni & 0x3FF) + 0xDC00);
            }
        }
        to[toLength] = 0;
        return to;
    }

    void TkConvertor::utf16ToUtf8(const wchar *from, char *to, i32 fromLength, i32 &toLength)
    {
        TkArray<u32> unicode;
        utf16ToUtf8Helper(unicode, from, fromLength, toLength);
        for (i32 i = 0, j = 0; j < unicode.count(); j++)
        {
            const u32 uni = unicode[j];
            const u32 count = uni <= 0x7FU ? 1 : uni <= 0x7FFU    ? 2
                                             : uni <= 0xFFFFU     ? 3
                                             : uni <= 0x1FFFFFU   ? 4
                                             : uni <= 0x3FFFFFFU  ? 5
                                             : uni <= 0x7FFFFFFFU ? 6
                                                                  : 7;
            to[i++] = (char)(count <= 1 ? (byte)uni : ((byte(0xFFU) << (8 - count)) | byte(uni >> (6 * (count - 1)))));
            for (u32 k = 1; k < count; k++)
                to[i++] = char(byte(0x80U | (byte(0x3FU) & byte(uni >> (6 * (count - 1 - k))))));
        }
    }

    char *TkConvertor::utf16ToUtf8(const wchar *from, i32 fromLength, i32 &toLength)
    {
        TkArray<u32> unicode;
        utf16ToUtf8Helper(unicode, from, fromLength, toLength);
        if (toLength == 0)
            return nullptr;
        char *to = (char *)TkMemory::allocate(TkMemory::getDefault(), toLength + 1); // TODO
        for (i32 i = 0, j = 0; j < unicode.count(); j++)
        {
            const u32 uni = unicode[j];
            const u32 count = uni <= 0x7FU ? 1 : uni <= 0x7FFU    ? 2
                                             : uni <= 0xFFFFU     ? 3
                                             : uni <= 0x1FFFFFU   ? 4
                                             : uni <= 0x3FFFFFFU  ? 5
                                             : uni <= 0x7FFFFFFFU ? 6
                                                                  : 7;
            to[i++] = (char)(count <= 1 ? (byte)uni : ((byte(0xFFU) << (8 - count)) | byte(uni >> (6 * (count - 1)))));
            for (u32 k = 1; k < count; k++)
                to[i++] = char(byte(0x80U | (byte(0x3FU) & byte(uni >> (6 * (count - 1 - k))))));
        }
        to[toLength] = 0;
        return to;
    }

    static const char *s_b64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static const i32 s_b64Idx[256] =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 63, 62, 62, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 63,
            0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    void TkConvertor::encryptBytes(byte *data, u64 size)
    {
        byte offset = 71;
        for (u64 i = 0; i < size; i++)
        {
            data[i] = (data[i] ^ (byte)i) + offset;
            offset += 13;
        }
    }

    void TkConvertor::decryptBytes(byte *data, u64 size)
    {
        byte offset = 71;
        for (u64 i = 0; i < size; i++)
        {
            data[i] = data[i] - offset ^ (byte)i;
            offset += 13;
        }
    }

    i32 TkConvertor::base64EncodeLength(i32 size)
    {
        return (size + 2) / 3 * 4;
    }

    i32 TkConvertor::base64DecodeLength(const char *encoded, i32 length)
    {
        if (length == 0)
            return 0;
        const i32 pad1 = length % 4 || encoded[length - 1] == '=';
        const i32 pad2 = pad1 && (length % 4 > 2 || encoded[length - 2] != '=');
        const i32 last = (length - pad1) / 4 << 2;
        return last / 4 * 3 + pad1 + pad2;
    }

    void TkConvertor::base64Encode(const byte *bytes, i32 size, TkArray<char> &encoded)
    {
        encoded.resize(base64EncodeLength(size));
        base64Encode(bytes, size, encoded.get());
    }

    void TkConvertor::base64Encode(const byte *bytes, i32 size, char *encoded)
    {
        i32 i;
        for (i = 0; i < size - 2; i += 3)
        {
            *encoded++ = s_b64Chars[bytes[i] >> 2 & 0x3F];
            *encoded++ = s_b64Chars[(bytes[i] & 0x3) << 4 | (i32)(bytes[i + 1] & 0xF0) >> 4];
            *encoded++ = s_b64Chars[(bytes[i + 1] & 0xF) << 2 | (i32)(bytes[i + 2] & 0xC0) >> 6];
            *encoded++ = s_b64Chars[bytes[i + 2] & 0x3F];
        }
        if (i < size)
        {
            *encoded++ = s_b64Chars[bytes[i] >> 2 & 0x3F];
            if (i == size - 1)
            {
                *encoded++ = s_b64Chars[((bytes[i] & 0x3) << 4)];
                *encoded++ = '=';
            }
            else
            {
                *encoded++ = s_b64Chars[(bytes[i] & 0x3) << 4 | (i32)(bytes[i + 1] & 0xF0) >> 4];
                *encoded++ = s_b64Chars[((bytes[i + 1] & 0xF) << 2)];
            }
            *encoded = '=';
        }
    }

    void TkConvertor::base64Decode(const char *encoded, i32 length, TkArray<byte> &output)
    {
        output.clear();
        if (length == 0)
            return;
        output.resize(base64DecodeLength(encoded, length));
        TkConvertor::base64Decode(encoded, length, output.get());
    }

    void TkConvertor::base64Decode(const char *encoded, i32 length, byte *output)
    {
        if (length == 0)
            return;
        i32 j = 0;
        const i32 pad1 = length % 4 || encoded[length - 1] == '=', pad2 = pad1 && (length % 4 > 2 || encoded[length - 2] != '=');
        const i32 last = (length - pad1) / 4 << 2;
        byte *str = output;
        for (i32 i = 0; i < last; i += 4)
        {
            i32 n = s_b64Idx[encoded[i]] << 18 | s_b64Idx[encoded[i + 1]] << 12 | s_b64Idx[encoded[i + 2]] << 6 | s_b64Idx[encoded[i + 3]];
            str[j++] = n >> 16;
            str[j++] = n >> 8 & 0xFF;
            str[j++] = n & 0xFF;
        }
        if (pad1)
        {
            i32 n = s_b64Idx[encoded[last]] << 18 | s_b64Idx[encoded[last + 1]] << 12;
            str[j++] = n >> 16;
            if (pad2)
            {
                n |= s_b64Idx[encoded[last + 2]] << 6;
                str[j] = n >> 8 & 0xFF;
            }
        }
    }

}