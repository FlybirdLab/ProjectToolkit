/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/source/IncludeResolver.hpp>

#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/types/Path.hpp>

namespace toolkit
{
    void IncludeResolver::addIncludeDirectory(TkStringView path)
    {
        TkString normalized(path);
        TkPath::normalize(normalized);
        for (i32 i = 0; i < m_includeDirs.count(); i++)
        {
            if (m_includeDirs[i] == normalized)
                return;
        }
        m_includeDirs.add(normalized);
    }

    bool IncludeResolver::resolve(
        TkStringView includeName,
        TkStringView currentFile,
        TkString &outResolvedPath
    ) const
    {
        // 1) Path relative to the directory of the file doing the #include.
        if (!currentFile.isEmpty())
        {
            const TkStringView dir = TkPath::getDir(currentFile);
            if (!dir.isEmpty())
            {
                TkString candidate = TkString(dir) / TkString(includeName);
                TkPath::normalize(candidate);
                if (TkFileSystem::isFileExist(candidate))
                {
                    outResolvedPath = candidate;
                    return true;
                }
            }
        }

        // 2) User-added include search dirs in order.
        for (i32 i = 0; i < m_includeDirs.count(); i++)
        {
            TkString candidate = m_includeDirs[i] / TkString(includeName);
            TkPath::normalize(candidate);
            if (TkFileSystem::isFileExist(candidate))
            {
                outResolvedPath = candidate;
                return true;
            }
        }

        return false;
    }
}
