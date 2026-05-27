/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * ShaderJobCache v2 tests. The cache is the correctness-critical seam
 * between cache-miss-builds-metadata and cache-hit-restores-metadata, so we
 * check:
 *   - round-trip (store -> tryLoad recovers the exact CachedShaderJobResult)
 *   - empty payload corner cases (no bytecode / no bindings / no deps)
 *   - cache-disabled / empty-dir short-circuits
 *   - v1 (legacy blob-only) files trigger MISS, not crash
 *   - bad magic / hash mismatch triggers MISS
 *   - filename layout (<32-hex>.jobcache under the configured dir)
 *   - file-name reproducibility across two stores of the same job
 */

#include "common.hpp"

using namespace shader_test;

namespace
{
    bool sameBytes(const TkArray<byte> &a, const TkArray<byte> &b)
    {
        if (a.count() != b.count()) return false;
        for (i32 i = 0; i < a.count(); i++) if (a[i] != b[i]) return false;
        return true;
    }
}

TEST(ShaderJobCacheTest, StoreThenLoadRecoversBytecode)
{
    const TkString dir = uniqueScratchDir("cache_basic");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/Asset"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// minimal"));
    CachedShaderJobResult written = makeCacheResult(TkStringView("BYTES12345678"),
                                                     /*bindings=*/false,
                                                     /*vertexInputs=*/false,
                                                     /*dependencies=*/false);
    ASSERT_TRUE(cache.store(job, written));

    CachedShaderJobResult loaded;
    ASSERT_TRUE(cache.tryLoad(job, loaded));
    EXPECT_TRUE(sameBytes(loaded.bytecode, written.bytecode));
    EXPECT_EQ(loaded.reflectionBindings.count(), 0);
    EXPECT_EQ(loaded.reflectionVertexInputs.count(), 0);
    EXPECT_EQ(loaded.dependencies.count(), 0);
}

TEST(ShaderJobCacheTest, RoundTripPreservesBindingsVertexInputsDependencies)
{
    const TkString dir = uniqueScratchDir("cache_metadata");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/AssetMeta"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// payload with metadata"));
    CachedShaderJobResult written = makeCacheResult(TkStringView("PAYLOAD"),
                                                     /*bindings=*/true,
                                                     /*vertexInputs=*/true,
                                                     /*dependencies=*/true);
    ASSERT_TRUE(cache.store(job, written));

    CachedShaderJobResult loaded;
    ASSERT_TRUE(cache.tryLoad(job, loaded));
    EXPECT_TRUE(sameBytes(loaded.bytecode, written.bytecode));

    ASSERT_EQ(loaded.reflectionBindings.count(), written.reflectionBindings.count());
    for (i32 i = 0; i < loaded.reflectionBindings.count(); i++)
    {
        EXPECT_EQ(loaded.reflectionBindings[i].name,  written.reflectionBindings[i].name);
        EXPECT_EQ(loaded.reflectionBindings[i].slot,  written.reflectionBindings[i].slot);
        EXPECT_EQ(loaded.reflectionBindings[i].stage, written.reflectionBindings[i].stage);
        EXPECT_EQ(loaded.reflectionBindings[i].type,  written.reflectionBindings[i].type);
    }

    ASSERT_EQ(loaded.reflectionVertexInputs.count(), written.reflectionVertexInputs.count());
    for (i32 i = 0; i < loaded.reflectionVertexInputs.count(); i++)
    {
        EXPECT_EQ(loaded.reflectionVertexInputs[i].semantic, written.reflectionVertexInputs[i].semantic);
        EXPECT_EQ(loaded.reflectionVertexInputs[i].location, written.reflectionVertexInputs[i].location);
        EXPECT_EQ(loaded.reflectionVertexInputs[i].format,   written.reflectionVertexInputs[i].format);
        EXPECT_EQ(loaded.reflectionVertexInputs[i].slot,     written.reflectionVertexInputs[i].slot);
        EXPECT_EQ(loaded.reflectionVertexInputs[i].offset,   written.reflectionVertexInputs[i].offset);
    }

    ASSERT_EQ(loaded.dependencies.count(), written.dependencies.count());
    for (i32 i = 0; i < loaded.dependencies.count(); i++)
        EXPECT_EQ(loaded.dependencies[i], written.dependencies[i]);
}

TEST(ShaderJobCacheTest, MissingFileProducesMissNotError)
{
    const TkString dir = uniqueScratchDir("cache_miss");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    // Never store -- only ask.
    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/Vanish"),
                                          GfxShaderProfile::kDirectX_SM5,
                                          TkStringView("// not stored"));
    CachedShaderJobResult loaded;
    EXPECT_FALSE(cache.tryLoad(job, loaded));
    // outResult must be empty after a miss so the caller doesn't get stale data.
    EXPECT_EQ(loaded.bytecode.count(), 0);
    EXPECT_EQ(loaded.reflectionBindings.count(), 0);
    EXPECT_EQ(loaded.reflectionVertexInputs.count(), 0);
    EXPECT_EQ(loaded.dependencies.count(), 0);
}

TEST(ShaderJobCacheTest, DisabledCacheNeitherLoadsNorStores)
{
    const TkString dir = uniqueScratchDir("cache_disabled");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));
    cache.setEnabled(false);

    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/Disabled"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// ignored"));
    CachedShaderJobResult written = makeCacheResult(TkStringView("XXXX"), false, false, false);
    EXPECT_FALSE(cache.store(job, written));
    CachedShaderJobResult loaded;
    EXPECT_FALSE(cache.tryLoad(job, loaded));
}

TEST(ShaderJobCacheTest, EmptyDirectoryShortCircuitsBothEnds)
{
    ShaderJobCache cache; // m_dir defaults to empty
    EXPECT_TRUE(cache.isEnabled());
    EXPECT_EQ(cache.getCacheDirectory(), TkString());

    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/NoDir"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// nowhere to write"));
    CachedShaderJobResult written = makeCacheResult(TkStringView("XX"), false, false, false);
    EXPECT_FALSE(cache.store(job, written));
    CachedShaderJobResult loaded;
    EXPECT_FALSE(cache.tryLoad(job, loaded));
}

TEST(ShaderJobCacheTest, JobsThatHashSameProduceTheSameFile)
{
    const TkString dir = uniqueScratchDir("cache_same_hash");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    // Two structurally identical jobs hash the same -> same file -- store
    // once, load with the second job.
    ShaderCompileJob a = makeMinimalJob(TkStringView("Test/Same"),
                                        GfxShaderProfile::kGLSL_410,
                                        TkStringView("// fixed"));
    ShaderCompileJob b = a;

    CachedShaderJobResult written = makeCacheResult(TkStringView("SHARED"), true, false, true);
    ASSERT_TRUE(cache.store(a, written));
    CachedShaderJobResult loaded;
    EXPECT_TRUE(cache.tryLoad(b, loaded));
    EXPECT_TRUE(sameBytes(loaded.bytecode, written.bytecode));
}

TEST(ShaderJobCacheTest, ChangingSourceTextChangesCacheFileAndForcesMiss)
{
    const TkString dir = uniqueScratchDir("cache_invalidate");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    ShaderCompileJob a = makeMinimalJob(TkStringView("Test/Inval"),
                                        GfxShaderProfile::kGLSL_410,
                                        TkStringView("// version A"));
    CachedShaderJobResult writtenA = makeCacheResult(TkStringView("OLD"), false, false, false);
    ASSERT_TRUE(cache.store(a, writtenA));

    // Change source -- hash recomputes -> different file -> miss.
    ShaderCompileJob b = a;
    b.sourceText = TkString("// version B");
    b.jobHashLow = 0; b.jobHashHigh = 0; // force re-hash
    CachedShaderJobResult loaded;
    EXPECT_FALSE(cache.tryLoad(b, loaded));
}

TEST(ShaderJobCacheTest, CacheFileLandsUnderConfiguredDirectoryWith32HexName)
{
    const TkString dir = uniqueScratchDir("cache_layout");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/Layout"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// any payload"));
    CachedShaderJobResult written = makeCacheResult(TkStringView("HELLO"), false, false, false);
    ASSERT_TRUE(cache.store(job, written));

    // Manually compute the expected file path.
    const ShaderHash128 h = ShaderHash::hashJob(job);
    const TkString hex = ShaderHash::toHex(h);
    const TkString expected = TkString(dir) / TkString::format("%s.jobcache", hex.get());
    EXPECT_TRUE(TkFileSystem::isFileExist(expected))
        << "expected cache file " << expected.get() << " was not created";
}

TEST(ShaderJobCacheTest, V1BlobOnlyFileIsTreatedAsMissNotCrash)
{
    const TkString dir = uniqueScratchDir("cache_v1_legacy");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    // Build a v1-style file body: magic + version=1 + hashLow + hashHigh +
    // blobSize + blob. The current reader recognises this layout but rejects
    // it because version != 2.
    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/V1"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// v1 payload"));
    const ShaderHash128 h = ShaderHash::hashJob(job);
    job.jobHashLow = h.low;
    job.jobHashHigh = h.high;
    const TkString hex = ShaderHash::toHex(h);
    const TkString path = TkString(dir) / TkString::format("%s.jobcache", hex.get());

    MemoryOStream o(64);
    constexpr u32 kSjcbMagic   = 0x42434A53; // 'SJCB' LE
    constexpr u32 kV1          = 1;
    const byte blob[]          = {0x10, 0x20, 0x30, 0x40};
    o.writeU32(kSjcbMagic);
    o.writeU32(kV1);
    o.writeU64(h.low);
    o.writeU64(h.high);
    o.writeU32(static_cast<u32>(sizeof(blob)));
    o.writeBytes(blob, sizeof(blob));
    ASSERT_TRUE(o.saveToFile(path));

    CachedShaderJobResult loaded;
    EXPECT_FALSE(cache.tryLoad(job, loaded))
        << "v1 entries must be rejected so the next miss can rewrite them";
    EXPECT_EQ(loaded.bytecode.count(), 0);
}

TEST(ShaderJobCacheTest, BadMagicProducesMiss)
{
    const TkString dir = uniqueScratchDir("cache_bad_magic");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/BadMagic"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// payload"));
    const ShaderHash128 h = ShaderHash::hashJob(job);
    job.jobHashLow = h.low;
    job.jobHashHigh = h.high;
    const TkString hex = ShaderHash::toHex(h);
    const TkString path = TkString(dir) / TkString::format("%s.jobcache", hex.get());

    MemoryOStream o(64);
    o.writeU32(0xDEADBEEFu);  // not 'SJCB'
    o.writeU32(2u);
    o.writeU64(h.low);
    o.writeU64(h.high);
    o.writeU32(0u);
    ASSERT_TRUE(o.saveToFile(path));

    CachedShaderJobResult loaded;
    EXPECT_FALSE(cache.tryLoad(job, loaded));
}

TEST(ShaderJobCacheTest, HashMismatchProducesMiss)
{
    const TkString dir = uniqueScratchDir("cache_hash_mismatch");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    // Compute the expected file path for `job` but write a file whose
    // header carries the *wrong* hash inside. This simulates a name
    // collision via filesystem -- the reader must MISS instead of trusting
    // the body.
    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/Collide"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// payload"));
    const ShaderHash128 h = ShaderHash::hashJob(job);
    job.jobHashLow = h.low;
    job.jobHashHigh = h.high;
    const TkString hex = ShaderHash::toHex(h);
    const TkString path = TkString(dir) / TkString::format("%s.jobcache", hex.get());

    MemoryOStream o(64);
    constexpr u32 kSjcbMagic = 0x42434A53;
    o.writeU32(kSjcbMagic);
    o.writeU32(2u);
    // Wrong hash inside the body.
    o.writeU64(h.low ^ 0xAAAAAAAAAAAAAAAAull);
    o.writeU64(h.high);
    o.writeU32(0u); // no blob
    o.writeU32(0u); // no bindings
    o.writeU32(0u); // no vertex inputs
    o.writeU32(0u); // no deps
    ASSERT_TRUE(o.saveToFile(path));

    CachedShaderJobResult loaded;
    EXPECT_FALSE(cache.tryLoad(job, loaded));
}

TEST(ShaderJobCacheTest, ShortTruncatedFileProducesMiss)
{
    const TkString dir = uniqueScratchDir("cache_truncated");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));

    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/Trunc"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// payload"));
    const ShaderHash128 h = ShaderHash::hashJob(job);
    job.jobHashLow = h.low;
    job.jobHashHigh = h.high;
    const TkString hex = ShaderHash::toHex(h);
    const TkString path = TkString(dir) / TkString::format("%s.jobcache", hex.get());

    // Only 8 bytes -- well under the 28-byte header minimum.
    MemoryOStream o(8);
    o.writeU32(0x42434A53u);
    o.writeU32(2u);
    ASSERT_TRUE(o.saveToFile(path));

    CachedShaderJobResult loaded;
    EXPECT_FALSE(cache.tryLoad(job, loaded));
}

TEST(ShaderJobCacheTest, VerboseModeIsToggleableWithoutAffectingResults)
{
    const TkString dir = uniqueScratchDir("cache_verbose");
    ShaderJobCache cache;
    cache.setCacheDirectory(TkStringView(dir));
    cache.setVerbose(true);
    EXPECT_TRUE(cache.isVerbose());
    cache.setVerbose(false);
    EXPECT_FALSE(cache.isVerbose());

    ShaderCompileJob job = makeMinimalJob(TkStringView("Test/Verbose"),
                                          GfxShaderProfile::kGLSL_410,
                                          TkStringView("// vb payload"));
    CachedShaderJobResult written = makeCacheResult(TkStringView("ABC"), false, false, false);
    EXPECT_TRUE(cache.store(job, written));
    CachedShaderJobResult loaded;
    EXPECT_TRUE(cache.tryLoad(job, loaded));
}
