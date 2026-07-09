/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/shader/source/SourceLocation.hpp>

namespace toolkit
{
    // A single loaded source file -- either backed by disk (path is the absolute
    // file path) or by an in-memory virtual buffer (path is a synthetic name).
    // Owned by SourceManager; do not construct directly.
    class SourceFile
    {
    public:
        SourceFile(u32 id, TkStringView path, TkStringView content, bool isVirtual);

        u32 getId() const { return m_id; }
        const TkString &getPath() const { return m_path; }
        const TkString &getContent() const { return m_content; }
        bool isVirtual() const { return m_isVirtual; }

        // Convert a byte offset to a full SourceLocation (line/column 1-based).
        SourceLocation getLocation(u32 offset) const;

        // Return the text of the given 1-based line (without trailing newline).
        // Returns empty view if line is out of range.
        TkStringView getLineText(u32 line) const;

        i32 getLineCount() const { return m_lineOffsets.count(); }

    private:
        u32 m_id = 0;
        TkString m_path;
        TkString m_content;
        bool m_isVirtual = false;
        // Byte offset of the start of each line (line 1 -> m_lineOffsets[0]).
        TkArray<u32> m_lineOffsets;
    };
}
