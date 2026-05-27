/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/shader/source/SourceFile.hpp>

namespace toolkit
{
    // Owns the set of SourceFiles seen by the compiler.
    // Lifetime spans one CLI invocation (or one editor session for LSP);
    // file ids returned here are unique within a single SourceManager only.
    class SourceManager
    {
    public:
        SourceManager() = default;
        ~SourceManager();

        SourceManager(const SourceManager &) = delete;
        SourceManager &operator=(const SourceManager &) = delete;

        // Load a real file from disk. Returns nullptr on read failure.
        // The same absolute path is mapped to the same SourceFile (cached).
        SourceFile *loadFile(TkStringView path);

        // Register an in-memory virtual file (e.g. inline HLSLPROGRAM block).
        // `name` is what the diagnostic printer will display.
        SourceFile *addVirtualFile(TkStringView name, TkStringView content);

        SourceFile *getFile(u32 fileId);
        const SourceFile *getFile(u32 fileId) const;

        // Convenience: look up a file by id and compute the location for an offset.
        // Returns SourceLocation::invalid() if fileId is unknown.
        SourceLocation getLocation(u32 fileId, u32 offset) const;
        TkStringView getFilePath(u32 fileId) const;

        i32 getFileCount() const { return m_files.count(); }

    private:
        // m_files[0] is intentionally nullptr so that valid file ids are 1-based.
        TkArray<SourceFile *> m_files;
    };
}
