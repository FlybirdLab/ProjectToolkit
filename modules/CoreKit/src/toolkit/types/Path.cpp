/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Path.hpp"

#include "Span.hpp"
#include "String.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/collection/StaticArray.hpp"
#include "toolkit/base/Math.hpp"

namespace toolkit
{

    constexpr char DirectorySeparatorCchar = '\\';
    constexpr char AltDirectorySeparatorCchar = '/';
    constexpr char VolumeSeparatorChar = ':';

    TkPath::TkPath()
        : m_path{}
    {
    }
    TkPath::TkPath(TkStringView path)
    {
        m_length = u32(normalize(path, TkSpan(m_path)) - m_path);
        m_hash = StableHash(m_path);
    }

    void TkPath::add(TkStringView value)
    {
        TkSpan span(m_path + m_length, m_path + MAX_PATH);
        m_length = u32(TkStringUtils::copyString(span, value) - m_path);
    }
    void TkPath::add(StableHash hash)
    {
        TkSpan span(m_path + m_length, m_path + MAX_PATH);
        m_length = u32(TkStringUtils::toString(hash.getHashValue(), span) - m_path);
    }
    void TkPath::add(u64 value)
    {
        TkSpan span(m_path + m_length, m_path + MAX_PATH);
        m_length = u32(TkStringUtils::toString(value, span) - m_path);
    }

    TkStringView TkPath::getFileName(const TkStringView &path)
    {
        char chr;
        const i32 length = path.length();
        i32 num = length;
        do
        {
            num--;
            if (num < 0)
                return path;
            chr = path[num];
        } while (chr != DirectorySeparatorCchar && chr != AltDirectorySeparatorCchar && chr != VolumeSeparatorChar);
        return path.substring(num + 1, length - num - 1);
    }

    void TkPath::pathWithoutRelative(TkString &path)
    {
        TkPath::normalize(path);

        TkString tmp = path;
        TkArray<TkString> components;
        tmp.split('/', components);

        const bool isAbsolute = !TkPath::isRelative(path.get());
#if TK_WINDOWS
        bool startWithDisk = false;
#endif
        TkStaticArray<TkString, MAX_PATH> stack;

        auto canPopByDotDot = [](const TkString &s) -> bool
        {
#if TK_WINDOWS
            if (s.length() == 2 && s[1] == ':' && TkStringUtils::isAlpha(s[0]))
                return false;
#endif
            return s != "..";
        };

        if (components.count() > 0)
        {
            auto &c0 = components[0];
#if TK_WINDOWS
            if (c0.length() == 2 && c0.endsWith(':'))
                startWithDisk = true;
#endif
        }
        for (const TkString &bit : components)
        {
            if (bit.isEmpty() || bit == ".")
                continue;

            if (bit == "..")
            {
                if (stack.hasItems() && canPopByDotDot(stack.back()))
                {
                    stack.pop();
                }
                else if (!isAbsolute)
                {
                    stack.push(bit);
                }
                continue;
            }

            stack.push(bit);
        }

        path.clear();

#if TK_WINDOWS
        if (isAbsolute && !startWithDisk)
#else
        if (isAbsolute)
#endif
        {
            path = "/";
        }

        for (const TkString &e : stack)
        {
            path /= e;
        }

        if (isAbsolute && path.isEmpty())
        {
            path = "/";
        }
    }

    TkStringView TkPath::getPathWithoutExt(const TkStringView &path)
    {
        const i32 num = path.findLast('.');
        if (num != -1)
            return path.substring(0, num);
        return path;
    }

    void TkPath::removeLongPathPrefix(const TkString &path, TkString &result)
    {
        if (!path.startsWith(TkStringView{"\\\\?\\"}, true))
        {
            result = path;
        }
        if (!path.startsWith(TkStringView{"\\\\?\\UNC\\"}, false))
        {
            result = path.substring(4);
        }
        result = path;
        result.remove(2, 6);
    }

    TkStringView TkPath::getBaseName(TkStringView path)
    { // TODO
        TkStringView filename = getFileName(path);
        const i32 num = filename.findLast('.');
        if (num != -1)
            return filename.substring(0, num);
        return filename;
    }

    void TkPath::endUpdate()
    {
        m_length = u32(normalize(m_path) - m_path);
        m_hash = StableHash(m_path);
    }

    void TkPath::operator=(TkStringView rhs)
    {
        TK_ASSERT(rhs.length() < lengthOf(m_path));
        m_length = u32(normalize(rhs, TkSpan(m_path)) - m_path);
        m_hash = StableHash(m_path);
    }

    bool TkPath::operator==(const char *rhs) const
    {
        return TkStringUtils::equalStrings(rhs, m_path);
    }

    bool TkPath::operator!=(const char *rhs) const
    {
        return !TkStringUtils::equalStrings(rhs, m_path);
    }

    bool TkPath::operator==(const TkPath &rhs) const
    {
        TK_ASSERT(TkStringUtils::equalStrings(m_path, rhs.m_path) == (m_hash == rhs.m_hash));
        return m_hash == rhs.m_hash;
    }

    bool TkPath::operator!=(const TkPath &rhs) const
    {
        TK_ASSERT(TkStringUtils::equalStrings(m_path, rhs.m_path) == (m_hash == rhs.m_hash));
        return m_hash != rhs.m_hash;
    }

    char *TkPath::normalize(char *path)
    {
        bool is_prev_slash = false;

        char *dst = path;
        const char *src = path;

        // Remove leading "./"
        if (src[0] == '.' && (src[1] == '\\' || src[1] == '/'))
            src += 2;

#if TK_WINDOWS
        // Preserve root slash
        if (*src == '\\' || *src == '/')
        {
            *dst++ = '/';
            ++src;
            is_prev_slash = true;
        }
#endif

        while (*src)
        {
            bool is_current_slash = (*src == '\\' || *src == '/');

            if (is_current_slash && is_prev_slash)
            {
                ++src;
                continue;
            }

            *dst++ = is_current_slash ? '/' : *src;
            is_prev_slash = is_current_slash;
            ++src;
        }

        if (dst - path > 1 && dst[-1] == '/')
        {
            --dst;
        }

        *dst = '\0';
        return dst;
    }

    char *TkPath::normalize(TkStringView path, TkSpan<char> output)
    {
        TK_ASSERT(output.length() > 0);

        char *out = output.begin();
        u32 max_size = output.length();
        u32 i = 0;

        if (path.isEmpty())
        {
            *out = '\0';
            return out;
        }

        const char *c = path.begin();
        bool is_prev_slash = false;

        // Remove leading "./"
        if (path.length() > 1 && c[0] == '.' && (c[1] == '\\' || c[1] == '/'))
            c += 2;

#if TK_WINDOWS
        // Preserve root slash
        if (c != path.end() && (*c == '\\' || *c == '/'))
        {
            if (i + 1 < max_size)
            {
                *out++ = '/';
                ++i;
            }
            ++c;
            is_prev_slash = true;
        }
#endif

        while (c != path.end() && i + 1 < max_size)
        {
            bool is_current_slash = (*c == '\\' || *c == '/');

            if (is_current_slash && is_prev_slash)
            {
                ++c;
                continue;
            }

            *out++ = is_current_slash ? '/' : *c;
            is_prev_slash = is_current_slash;
            ++c;
            ++i;
        }

        if (i > 1 && out[-1] == '/')
        {
            --out;
        }

        *out = '\0';
        return out;
    }

    void TkPath::normalize(TkString &path)
    {
        TkSpan<char> s(path.get(), path.length() + 1);
        char *end = normalize(path.get(), s);
        path.set(s.begin(), end - s.begin());
    }

    TkStringView TkPath::getDir(TkStringView path)
    {
        const i32 lastFrontSlash = path.findLast('\\');
        const i32 lastBackSlash = path.findLast('/');
        const i32 splitIndex = TkMath::max(lastBackSlash, lastFrontSlash);

        if (splitIndex == -1)
            return TkStringView::Empty;

        // "/" -> ""
        if (splitIndex == 0 && path.length() == 1)
            return TkStringView::Empty;

        // "/a" -> "/"
        if (splitIndex == 0)
            return TkStringView(path.begin(), 1);

        return path.left(splitIndex);
    }

    TkStringView TkPath::getExtension(TkStringView src)
    {
        if (src.isEmpty())
            return TkStringView::Empty;

        const char *p = src.end();
        while (p != src.begin())
        {
            --p;
            if (*p == '.')
                return TkStringView(p + 1, src.end() - (p + 1));
            if (*p == '/' || *p == '\\')
                break;
        }
        return TkStringView::Empty;
    }

    bool TkPath::isRelative(const TkStringView &path)
    {
        const bool isRooted =
            (path.length() >= 2 && TkStringUtils::isAlpha(path[0]) && path[1] == ':') ||
            path.startsWith(TkStringView("\\\\", 2), true) ||
            path.startsWith('\\') ||
            path.startsWith('/');
        return !isRooted;
    }

    bool TkPath::isSame(TkStringView a, TkStringView b)
    {
        char bufA[MAX_PATH];
        char bufB[MAX_PATH];

        TkPath::normalize(a, TkSpan(bufA));
        TkPath::normalize(b, TkSpan(bufB));

#if TK_WINDOWS
        return TkStringUtils::equalIStrings(bufA, bufB);
#else
        return TkStringUtils::equalStrings(bufA, bufB);
#endif
    }

    bool TkPath::replaceExtension(char *path, const char *ext)
    {
        TK_ASSERT(path);
        TK_ASSERT(ext);

        char *dot = nullptr;
        char *p = path;

        while (*p)
        {
            if (*p == '.')
                dot = p;
            ++p;
        }

        if (!dot)
            return false;

        ++dot;

        const char *src = ext;
        while (*src)
        {
            *dot++ = *src++;
        }

        *dot = '\0';
        return true;
    }

    bool TkPath::hasExtension(TkStringView filename, TkStringView ext)
    {
        return TkStringUtils::equalIStrings(getExtension(filename), ext);
    }

    TkPath::operator TkStringView() const
    {
        return TkStringView(m_path, m_length);
    }

    PathInfo::PathInfo(TkStringView path)
    {
        extension = TkPath::getExtension(path);
        basename = TkPath::getBaseName(path);
        dir = TkPath::getDir(path);
    }

}