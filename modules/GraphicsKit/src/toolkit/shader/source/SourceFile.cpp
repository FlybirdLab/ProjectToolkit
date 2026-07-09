/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/source/SourceFile.hpp>

namespace toolkit
{
    SourceFile::SourceFile(u32 id, TkStringView path, TkStringView content, bool isVirtual)
        : m_id(id), m_path(path), m_content(content), m_isVirtual(isVirtual)
    {
        // Pre-compute the byte offset of every line.
        // Newline policy: '\n' terminates the current line. CRLF is handled implicitly
        // because '\r' is left attached to the line above; getLineText() trims it.
        m_lineOffsets.add(0);
        const char *data = m_content.get();
        const i32 length = m_content.length();
        for (i32 i = 0; i < length; i++)
        {
            if (data[i] == '\n')
            {
                m_lineOffsets.add(static_cast<u32>(i + 1));
            }
        }
    }

    SourceLocation SourceFile::getLocation(u32 offset) const
    {
        SourceLocation loc;
        loc.fileId = m_id;
        loc.offset = offset;

        // Binary search for the line whose start offset is <= offset.
        i32 lo = 0;
        i32 hi = m_lineOffsets.count() - 1;
        while (lo < hi)
        {
            const i32 mid = lo + (hi - lo + 1) / 2;
            if (m_lineOffsets[mid] <= offset)
                lo = mid;
            else
                hi = mid - 1;
        }
        loc.line = static_cast<u32>(lo + 1);
        loc.column = static_cast<u32>(offset - m_lineOffsets[lo] + 1);
        return loc;
    }

    TkStringView SourceFile::getLineText(u32 line) const
    {
        if (line == 0 || static_cast<i32>(line) > m_lineOffsets.count())
            return TkStringView();

        const u32 start = m_lineOffsets[static_cast<i32>(line) - 1];
        u32 end;
        if (static_cast<i32>(line) < m_lineOffsets.count())
            end = m_lineOffsets[static_cast<i32>(line)] - 1; // exclude '\n'
        else
            end = static_cast<u32>(m_content.length());

        // Strip trailing '\r' for CRLF lines.
        const char *data = m_content.get();
        if (end > start && data[end - 1] == '\r')
            end--;

        return TkStringView(data + start, static_cast<i32>(end - start));
    }
}
