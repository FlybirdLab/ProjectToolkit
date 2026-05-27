/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/shader/format/ShaderAsset.hpp>

namespace toolkit
{
    enum class TksdReadResult : u32
    {
        Ok                   = 0,
        ReadFailed           = 1, // 'TKSHD0001' — file I/O failed
        InvalidHeader        = 2, // 'TKSHD0201' — magic / sizes wrong
        UnsupportedVersion   = 3, // 'TKSHD0202'
        InvalidChunkTable    = 4, // 'TKSHD0203'
        ChunkOutOfRange      = 5, // 'TKSHD0204'
        UnsupportedCompression = 6, // 'TKSHD0205'
        TruncatedChunk       = 7,
    };

    const char *toString(TksdReadResult result);

    // Reads a .tksd container into a ShaderAsset. Unknown chunks are skipped
    // (per the format guarantee), so a v1 reader can ingest a v1.x file even
    // if it grows new chunks.
    //
    // The reader never throws and never asserts on malformed input -- it
    // returns the appropriate `TksdReadResult`. `lastError()` returns a
    // short human-readable description of the most recent failure, useful
    // for CLI diagnostics where DiagnosticSink isn't available.
    class TksdReader
    {
    public:
        TksdReadResult readFromFile(TkStringView path, ShaderAsset &outAsset);
        TksdReadResult readFromMemory(const byte *data, u32 size, ShaderAsset &outAsset);

        const TkString &lastError() const { return m_lastError; }

    private:
        TksdReadResult parseAll(const byte *data, u32 size, ShaderAsset &outAsset);
        TksdReadResult parseChunk(u32 type, const byte *data, u32 size, ShaderAsset &outAsset);

        TkString m_lastError;
    };
}
