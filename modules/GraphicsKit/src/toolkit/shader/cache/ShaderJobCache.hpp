/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>

#include <toolkit/shader/cache/ShaderHash.hpp>
#include <toolkit/shader/format/ShaderAsset.hpp> // ShaderBindingInfo / ShaderVertexInputInfo

namespace toolkit
{
    struct ShaderCompileJob;

    // Per-job cache payload. Stored on disk via ShaderJobCache so a cache hit
    // produces the same downstream .tksd metadata as a fresh compile:
    // bytecode alone is not enough -- if we lose the SPIRV-Cross-derived
    // bindings / vertex inputs and the dependency closure on hit, the .tksd
    // ends up with empty Bindings / VertexInput / Dependencies chunks even
    // though the matching cache-miss run produced full ones. Keeping the
    // metadata inside the cache entry makes hit/miss byte-equivalent at the
    // asset level.
    struct CachedShaderJobResult
    {
        TkArray<byte> bytecode;
        TkArray<ShaderBindingInfo> reflectionBindings;
        TkArray<ShaderVertexInputInfo> reflectionVertexInputs;
        TkArray<TkString> dependencies; // resolved absolute paths
    };

    // Trivial on-disk cache for compiled shader blobs + their reflection.
    //
    // Layout under `setCacheDirectory(dir)`:
    //   <dir>/<32-hex-hash>.jobcache
    //
    // File body (version 2):
    //   u32 magic  ('SJCB' little endian, 0x42434A53)
    //   u32 version (2)
    //   u64 hashLow
    //   u64 hashHigh
    //   u32 blobSize
    //   bytes (blobSize)
    //   u32 bindingCount
    //     per binding: str name, u32 slot, u32 stage, str type
    //   u32 vertexInputCount
    //     per vinput: str semantic, u32 location, u32 format, u32 slot, u32 offset
    //   u32 dependencyCount
    //     per dep: str path
    //
    // Lookup hashes the job in-memory and probes the matching path. v1 (blob-
    // only) cache files are silently treated as miss -- the user can delete
    // the directory or just let new misses overwrite. There is no in-memory
    // index, no eviction policy and no concurrent-write guard; two processes
    // hitting the same job race the file system but the last writer wins
    // with the same content.
    //
    // Diagnostic mode (setVerbose(true), wired to the CLI's --show-stats):
    // every tryLoad / store call prints the computed job hash, the on-disk
    // path, and a one-line "hit/miss reason". Lets the caller pinpoint
    // whether a perceived cache miss is store-write failure, tryLoad
    // header-rejection, or genuine job-hash drift between runs.
    class ShaderJobCache
    {
    public:
        void setCacheDirectory(TkStringView dir);
        const TkString &getCacheDirectory() const { return m_dir; }

        bool isEnabled() const { return m_enabled; }
        void setEnabled(bool enabled) { m_enabled = enabled; }

        // Enable [ShaderJobCache] log lines for every lookup / store.
        void setVerbose(bool verbose) { m_verbose = verbose; }
        bool isVerbose() const { return m_verbose; }

        // Compute the job hash if not already set on the job, then look up.
        // Writes the cached bytecode + reflection + dependencies into
        // `outResult` on hit and returns true.
        bool tryLoad(ShaderCompileJob &job, CachedShaderJobResult &outResult);

        // Persist a freshly-compiled result. Computes the job hash if needed.
        bool store(ShaderCompileJob &job, const CachedShaderJobResult &result);

    private:
        TkString makeCachePath(const ShaderHash128 &hash) const;
        static void ensureHash(ShaderCompileJob &job);

        TkString m_dir;
        bool m_enabled = true;
        bool m_verbose = false;
    };
}
