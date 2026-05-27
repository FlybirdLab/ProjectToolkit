/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit
{
    struct ShaderCompileJob;

    // 128-bit hash slot used by the job cache. v1 only fills the low 64 bits
    // (XXH3 of all job inputs) and reserves `high` for a future stronger hash
    // (e.g. SHA-256-truncated or BLAKE3) without a cache-file format change.
    struct ShaderHash128
    {
        u64 low = 0;
        u64 high = 0;

        bool isZero() const { return low == 0 && high == 0; }
        bool operator==(const ShaderHash128 &o) const { return low == o.low && high == o.high; }
        bool operator!=(const ShaderHash128 &o) const { return !(*this == o); }
    };

    // Deterministic hash helpers used by the shader job cache. Stable across
    // runs (no pointer values, no time, no allocator order).
    class ShaderHash
    {
    public:
        // Hash a single shader compile job. Inputs are mixed in a stable
        // textual form so the cache file name is reproducible across rebuilds.
        static ShaderHash128 hashJob(const ShaderCompileJob &job);

        // Convenience overloads for callers that just want the building blocks.
        static ShaderHash128 hashBytes(const void *data, u32 size);
        static ShaderHash128 hashString(TkStringView text);

        // Format a hash as a stable hex string for use in file names.
        static TkString toHex(const ShaderHash128 &hash);
    };
}
