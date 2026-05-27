/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit
{
    // Minimal include search-path resolver.
    //
    // Phase 2 scope: try `currentFile`'s directory first, then the user-added
    // include directories. Returns a normalized absolute path on success.
    // Phase 6 will layer dependency-graph hashing on top of this.
    class IncludeResolver
    {
    public:
        void addIncludeDirectory(TkStringView path);

        bool resolve(
            TkStringView includeName,
            TkStringView currentFile,
            TkString &outResolvedPath
        ) const;

        const TkArray<TkString> &getIncludeDirectories() const { return m_includeDirs; }

    private:
        TkArray<TkString> m_includeDirs;
    };
}
