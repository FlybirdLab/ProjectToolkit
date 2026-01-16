/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/types/StringView.hpp>

namespace toolkit
{

    struct PathInfo
    {
        explicit PathInfo(TkStringView path);

        TkStringView extension;
        TkStringView basename;
        TkStringView dir;
    };


    class TkPath final
    {
    public:
        static void normalize(TkString &path);
        static char *normalize(char *in_out_path);
        static char *normalize(TkStringView path, TkSpan<char> output);

        static TkStringView getDir(TkStringView src);
        static TkStringView getFileName(const TkStringView &path);
        static TkStringView getBaseName(TkStringView path);

        static TkStringView getExtension(TkStringView src);
        static bool hasExtension(TkStringView filename, TkStringView ext);
        static bool replaceExtension(char *path, const char *ext);
        static bool isSame(TkStringView a, TkStringView b);
        static bool isRelative(const TkStringView &path);

        // miscs
        static void pathWithoutRelative(TkString &path);
        static void removeLongPathPrefix(const TkString &path, TkString &result);
        static TkStringView getPathWithoutExt(const TkStringView &path);

    public:
        TkPath();
        explicit TkPath(TkStringView path);
        template <typename... Args>
        explicit TkPath(Args... args);

        void operator=(TkStringView rhs);
        bool operator==(const char *rhs) const;
        bool operator==(const TkPath &rhs) const;
        bool operator!=(const char *rhs) const;
        bool operator!=(const TkPath &rhs) const;

        u32 length() const { return m_length; }
        FilePathHash getHash() const { return m_hash; }
        template <typename... Args>
        void append(Args... args);
        char *beginUpdate() { return m_path; }
        void endUpdate();
        const char *c_str() const { return m_path; }
        bool isEmpty() const { return m_path[0] == '\0'; }
        static u32 capacity() { return MAX_PATH; }
        operator TkStringView() const;

    private:
        void add(TkStringView);
        void add(StableHash hash);
        void add(u64 value);

        char m_path[MAX_PATH];
        u32 m_length = 0;
        FilePathHash m_hash;
    };

    template <typename... Args>
    TkPath::TkPath(Args... args)
    {
        m_path[0] = '\0';
        int tmp[] = {(add(args), 0)...};
        (void)tmp;
        endUpdate();
    }

    template <typename... Args>
    void TkPath::append(Args... args)
    {
        int tmp[] = {(add(args), 0)...};
        (void)tmp;
        endUpdate();
    }

}