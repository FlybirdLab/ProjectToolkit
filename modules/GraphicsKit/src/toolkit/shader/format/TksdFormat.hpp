/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{
    // 'TKSD' little-endian.
    constexpr u32 TKSD_MAGIC = 0x44534B54;

    // Format version. Bumping `major` is a breaking change; `minor` should
    // remain backward-compatible (new chunks may be added but readers must
    // be able to skip unknown chunks regardless).
    constexpr u16 TKSD_VERSION_MAJOR = 1;
    constexpr u16 TKSD_VERSION_MINOR = 0;

    enum class TksdChunkType : u32
    {
        Metadata      = 1,
        Properties    = 2,
        Tags          = 3,
        Passes        = 4,
        StageBytecode = 5,
        Reflection    = 6,
        Bindings      = 7,
        VertexInput   = 8,
        Variants      = 9,
        Dependencies  = 10,
        DebugInfo     = 11,
    };

    enum class TksdCompression : u32
    {
        None = 0,
        // Reserved values for future compressors (LZ4, ZSTD, ...).
    };

    enum class TksdFlags : u32
    {
        None = 0,
    };

    // Number of bytes in every fixed-size hash slot in the header.
    constexpr u32 TKSD_HASH_SIZE = 32;

    // Single canonical stage-data kind. The payload is a self-contained
    // `.shadercache`-wire-format blob covering every stage in the pass, ready
    // to be fed into `GfxShader::create(MemoryIStream&)` by way of
    // `LegacyShaderCacheBuilder`. The kind tag is kept on disk as a u32 so a
    // future split into true per-stage bytecode can reserve a second value
    // without a format-major bump.
    constexpr u32 TKSD_STAGE_KIND_RAW = 1;

#pragma pack(push, 1)

    // Fixed-size on-disk header. All multi-byte integers are little-endian
    // (matches every target platform we ship).
    struct TksdHeader
    {
        u32 magic;
        u16 versionMajor;
        u16 versionMinor;

        u32 headerSize;        // sizeof(TksdHeader)
        u32 chunkTableOffset;  // absolute byte offset of the chunk-header table
        u32 chunkCount;

        u64 fileSize;

        u8 sourceHash[TKSD_HASH_SIZE];
        u8 dependencyHash[TKSD_HASH_SIZE];
        u8 compilerHash[TKSD_HASH_SIZE];
        u8 reflectionHash[TKSD_HASH_SIZE];
        u8 payloadHash[TKSD_HASH_SIZE];

        u32 flags;
        u32 reserved;
    };

    // One entry per chunk. The on-disk layout is:
    //   [TksdHeader] [TksdChunkHeader] x chunkCount [chunk payloads...]
    // Each chunk payload is at `offset`, has `size` bytes after optional
    // decompression to `uncompressedSize`. Payload alignment is 8 bytes from
    // the start of the file.
    struct TksdChunkHeader
    {
        u32 type;             // TksdChunkType
        u32 version;          // per-chunk schema version
        u64 offset;           // absolute byte offset
        u64 size;             // bytes on disk (compressed if compression != None)
        u64 uncompressedSize; // bytes after decompression
        u32 compression;      // TksdCompression
        u32 checksum;         // optional per-chunk checksum, 0 = unchecked
    };

#pragma pack(pop)

    static_assert(sizeof(TksdHeader) == 4 + 2 + 2 + 4 + 4 + 4 + 8 + (TKSD_HASH_SIZE * 5) + 4 + 4,
                  "TksdHeader layout drifted -- update the on-disk version!");
    static_assert(sizeof(TksdChunkHeader) == 4 + 4 + 8 + 8 + 8 + 4 + 4,
                  "TksdChunkHeader layout drifted -- update the on-disk version!");

    // Round `value` up to the next multiple of `alignment` (alignment must be
    // a power of two). Used by writer/reader to keep chunk payloads 8-aligned.
    constexpr u64 tksdAlignUp(u64 value, u64 alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }
}
