/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Path.hpp"

#include "Span.hpp"
#include "String.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/collection/StaticArray.hpp"
#include "toolkit/base/Math.hpp"

namespace toolkit {
    
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


    TkStringView TkPath::getFileName(const TkStringView& path)
    {
        char chr;
        const i32 length = path.length();
        i32 num = length;
        do {
            num--;
            if (num < 0)
                return path;
            chr = path[num];
        } while (chr != DirectorySeparatorCchar && chr != AltDirectorySeparatorCchar && chr != VolumeSeparatorChar);
        return path.substring(num + 1, length - num - 1);
    }


    void TkPath::pathWithoutRelative(TkString& path)
    {
        TkPath::normalize(path);

        TkArray<TkString> components;
        path.split('/', components);

        TkStaticArray<TkString, 16> stack;
        for (TkString& bit : components) {
            if (bit == "..") {
                if (stack.hasItems()) {
                    TkString popped = stack.pop();
                    const i32 windowsDriveStart = popped.find('\\');
                    if (popped == "..") {
                        stack.push(popped);
                        stack.push(bit);
                    }
                    else if (windowsDriveStart != -1) {
                        stack.add(moveTemp(popped.left(windowsDriveStart + 1)));
                    }
                }
                else {
                    stack.push(bit);
                }
            }
            else if (bit == ".") {
                // Skip /./
            }
            else {
                stack.add(moveTemp(bit));
            }
        }

        const bool isRooted = path.startsWith('/') || (path.length() >= 2 && path[0] == '.' && path[1] == '/');
        path.clear();
        for (const TkString& e : stack)
            path /= e;
        if (isRooted && path.hasChars() && path[0] != '/')
            path.insert(0, "/");
    }


    TkStringView TkPath::getPathWithoutExt(const TkStringView& path) {
        const i32 num = path.findLast('.');
        if (num != -1)
            return path.substring(0, num);
        return path;
    }


    void TkPath::removeLongPathPrefix(const TkString& path, TkString& result)
    {
        if (!path.startsWith(TkStringView{ "\\\\?\\" }, true)) {
            result = path;
        }
        if (!path.startsWith(TkStringView{ "\\\\?\\UNC\\" }, false)) {
            result = path.substring(4);
        }
        result = path;
        result.remove(2, 6);
    }


    TkStringView TkPath::getBaseName(TkStringView path)
    { //TODO
        TkStringView filename = getFileName(path);
        const i32 num = filename.findLast('.');
        if (num != -1)
            return filename.substring(0, num);
        return filename;
    }


    void TkPath::endUpdate() {
        m_length = u32(normalize(m_path) - m_path);
        m_hash = StableHash(m_path);
    }


    void TkPath::operator=(TkStringView rhs) {
        TK_ASSERT(rhs.length() < lengthOf(m_path));
        m_length = u32(normalize(rhs, TkSpan(m_path)) - m_path);
        m_hash = StableHash(m_path);
    }


    bool TkPath::operator==(const char* rhs) const {
        return TkStringUtils::equalStrings(rhs, m_path);
    }

    bool TkPath::operator!=(const char* rhs) const {
        return !TkStringUtils::equalStrings(rhs, m_path);
    }

    bool TkPath::operator==(const TkPath& rhs) const {
        TK_ASSERT(TkStringUtils::equalStrings(m_path, rhs.m_path) == (m_hash == rhs.m_hash));
        return m_hash == rhs.m_hash;
    }

    bool TkPath::operator!=(const TkPath& rhs) const {
        TK_ASSERT(TkStringUtils::equalStrings(m_path, rhs.m_path) == (m_hash == rhs.m_hash));
        return m_hash != rhs.m_hash;
    }


    char* TkPath::normalize(char* path)
    {
        bool is_prev_slash = false;

        char* dst = path;
        const char* src = dst;
        if (src[0] == '.' && (src[1] == '\\' || src[1] == '/')) src += 2;

#if TK_WINDOWS
        if (src[0] == '\\' || src[0] == '/') ++src;
#endif

        while (*src) {
            bool is_current_slash = *src == '\\' || *src == '/';

            if (is_current_slash && is_prev_slash) {
                ++src;
                continue;
            }

            *dst = *src == '\\' ? '/' : *src;

            src++;
            dst++;

            is_prev_slash = is_current_slash;
        }
        *dst = 0;
        return dst;

    }


    char* TkPath::normalize(TkStringView path, TkSpan<char> output)
    {
        TK_ASSERT(path.begin() <= output.begin() || path.begin() > output.end());
        u32 max_size = output.length();
        TK_ASSERT(max_size > 0);
        char* out = output.begin();

        if (path.length() == 0) {
            *out = '\0';
            return out;
        }

        u32 i = 0;
        bool is_prev_slash = false;

        const char* c = path.begin();
        if (c[0] == '.' && path.length() > 1 && (c[1] == '\\' || c[1] == '/')) c += 2;

#if TK_WINDOWS
        if (c != path.end() && (c[0] == '\\' || c[0] == '/')) ++c;
#endif

        while (c != path.end() && i < max_size) {
            bool is_current_slash = *c == '\\' || *c == '/';

            if (is_current_slash && is_prev_slash) {
                ++c;
                continue;
            }

            *out = *c == '\\' ? '/' : *c;

            c++;
            out++;
            i++;

            is_prev_slash = is_current_slash;
        }
        TK_ASSERT(i < max_size);
        if (i < max_size) {
            *out = '\0';
            return out;
        }
        *(out - 1) = '\0';
        return out - 1;
    }


    void TkPath::normalize(TkString& path)
    {
        TkSpan<char> s(path.get(), path.length());
        auto* end = normalize(path.get(), s);
        path.set(s.begin(), end - s.begin());
    }


    TkStringView TkPath::getDir(TkStringView path)
    {
        const i32 lastFrontSlash = path.findLast('\\');
        const i32 lastBackSlash = path.findLast('/');
        const i32 splitIndex = TkMath::max(lastBackSlash, lastFrontSlash);
        return splitIndex != -1 ? path.left(splitIndex) : TkStringView::Empty;
    }


    TkStringView TkPath::getExtension(TkStringView src)
    {
        if (src.isEmpty())
            return src;

        const char* end = src.end();
        const char* begin = src.begin() - 1;

        while (begin != src.begin() && *begin != '.') {
            --begin;
        }
        if (*begin != '.')
            return TkStringView::Empty;
        ++begin;
        return TkStringView{ begin, static_cast<i32>(end - begin) };
    }


    bool TkPath::isRelative(const TkStringView& path) 
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
        auto aSize = a.length();
        auto bSize = b.length();
        auto aEnd = a.end();
        auto bEnd = b.end();

        if (aSize > 0 && (*aEnd == '\\' || *aEnd == '/')) {
            --aEnd;
            aSize--;
        }
        if (bSize > 0 && (*bEnd == '\\' || *bEnd == '/')) {
            --bEnd;
            bSize--;
        }
        if (aSize == 0 && bSize == 1 && b[0] == '.') return true;
        if (bSize == 0 && aSize == 1 && a[0] == '.') return true;

        a = {aEnd - sizeof(char) * aSize, aSize};
        b = {bEnd - sizeof(char) * bSize, bSize};

        return TkStringUtils::equalStrings(a, b);
    }


    bool TkPath::replaceExtension(char* path, const char* ext)
    {
        char* end = path + TkStringUtils::length(path);
        while (end > path && *end != '.')
        {
            --end;
        }
        if (*end != '.') return false;

        ++end;
        const char* src = ext;
        while (*src != '\0' && *end != '\0')
        {
            *end = *src;
            ++end;
            ++src;
        }
        bool copied_whole_ext = *src == '\0';
        if (!copied_whole_ext) return false;

        *end = '\0';
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