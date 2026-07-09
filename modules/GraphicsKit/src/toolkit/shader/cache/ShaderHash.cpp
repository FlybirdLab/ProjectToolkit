/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/cache/ShaderHash.hpp>
#include <toolkit/shader/compilegraph/ShaderCompileJob.hpp>

#include <toolkit/base/Hash.hpp>

namespace toolkit
{
    namespace
    {
        // Single shared salt used to derive the 'high' lane in the future. For
        // now we leave high == 0; the slot is reserved.
        //
        // Bumping this string invalidates every existing on-disk cache entry,
        // which is what you want when the hash mix changes (e.g. when Phase 8
        // started feeding dependency hashes into the job hash).
        constexpr const char *kCompilerVersionTag = "TkShaderCompiler v2 (Phase 8 deps)";

        void hashAppend(RollingStableHasher &h, TkStringView s)
        {
            const u32 len = static_cast<u32>(s.length());
            h.update(&len, sizeof(len));
            if (len > 0)
                h.update(s.get(), len);
        }

        void hashAppendCStr(RollingStableHasher &h, const char *s)
        {
            i32 len = 0;
            while (s[len] != 0) len++;
            const u32 ulen = static_cast<u32>(len);
            h.update(&ulen, sizeof(ulen));
            if (ulen > 0)
                h.update(s, ulen);
        }

        void hashAppendU32(RollingStableHasher &h, u32 v)
        {
            h.update(&v, sizeof(v));
        }

        void hashAppendU64(RollingStableHasher &h, u64 v)
        {
            h.update(&v, sizeof(v));
        }
    }

    ShaderHash128 ShaderHash::hashJob(const ShaderCompileJob &job)
    {
        RollingStableHasher hasher;
        hasher.begin();

        // 1) Compiler identity -- bumping `kCompilerVersionTag` invalidates
        //    every cache entry without us having to chase consumers.
        hashAppendCStr(hasher, kCompilerVersionTag);

        // 2) Shader / pass identity
        hashAppend(hasher, TkStringView(job.shaderName));
        hashAppend(hasher, TkStringView(job.passName));
        hashAppendU32(hasher, static_cast<u32>(job.stage));
        hashAppendU32(hasher, static_cast<u32>(job.profile));
        hashAppend(hasher, TkStringView(job.entryPoint));
        hashAppend(hasher, TkStringView(job.variantKey));

        // 3) Primary source -- both the resolved path (when external) and the
        //    inline / virtual source text. Mixing the text directly here is
        //    what makes the cache invalidate on .hlsl edits.
        hashAppend(hasher, TkStringView(job.sourcePath));
        hashAppend(hasher, TkStringView(job.virtualSourceName));
        hashAppend(hasher, TkStringView(job.sourceText));

        // 4) Defines / variant macros.
        const u32 defineCount = static_cast<u32>(job.defines.count());
        hashAppendU32(hasher, defineCount);
        for (i32 i = 0; i < job.defines.count(); i++)
        {
            const ShaderMacro &m = job.defines[i];
            if (m.name != nullptr)       hashAppendCStr(hasher, m.name);
            else                          hashAppendCStr(hasher, "");
            if (m.definition != nullptr) hashAppendCStr(hasher, m.definition);
            else                          hashAppendCStr(hasher, "");
        }

        // 5) Include search dirs. Path order is significant: -I A -I B differs
        //    from -I B -I A because resolution picks the first match, so we
        //    do NOT sort here. The CLI is expected to feed normalized paths
        //    so the same logical layout produces the same hash on Windows
        //    and POSIX.
        const u32 incCount = static_cast<u32>(job.includeDirs.count());
        hashAppendU32(hasher, incCount);
        for (i32 i = 0; i < job.includeDirs.count(); i++)
            hashAppend(hasher, TkStringView(job.includeDirs[i]));

        // 6) Dependency closure -- this is what makes editing an included
        //    file invalidate the cache. The dependency array is pre-sorted
        //    by DependencyScanner so the hash is order-stable.
        const u32 depCount = static_cast<u32>(job.dependencies.count());
        hashAppendU32(hasher, depCount);
        for (i32 i = 0; i < job.dependencies.count(); i++)
        {
            const ShaderDependency &d = job.dependencies[i];
            hashAppend(hasher, TkStringView(d.resolvedPath));
            hashAppendU64(hasher, d.contentHash.low);
            hashAppendU64(hasher, d.contentHash.high);
        }
        // Mix in the precomputed dependencyHash too. Strictly redundant given
        // the loop above, but it lets future code compare hashes cheaply
        // without rehashing every dependency entry.
        hashAppendU64(hasher, job.dependencyHash.low);
        hashAppendU64(hasher, job.dependencyHash.high);

        ShaderHash128 out;
        out.low = hasher.end64().getHashValue();
        out.high = 0; // reserved for future stronger hash
        return out;
    }

    ShaderHash128 ShaderHash::hashBytes(const void *data, u32 size)
    {
        ShaderHash128 out;
        out.low = StableHash(data, size).getHashValue();
        return out;
    }

    ShaderHash128 ShaderHash::hashString(TkStringView text)
    {
        return hashBytes(text.get(), static_cast<u32>(text.length()));
    }

    TkString ShaderHash::toHex(const ShaderHash128 &hash)
    {
        // 32 hex chars for 128 bits. We emit both lanes even though `high`
        // is zero in v1 so the file name slot keeps the same width once the
        // high lane lights up.
        return TkString::format("%016llx%016llx",
                                static_cast<unsigned long long>(hash.high),
                                static_cast<unsigned long long>(hash.low));
    }
}
