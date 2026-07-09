/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * ShaderHash tests. The hash is the cache-key foundation, so we cover:
 *   - determinism (same input -> same hash, twice in a row)
 *   - sensitivity (each ShaderCompileJob field that should change the hash)
 *   - insensitivity to fields that should NOT change it (notably
 *     `outputName` which is a logging affordance only)
 *   - toHex stability (32 lowercase hex chars)
 */

#include "common.hpp"

using namespace shader_test;

namespace
{
    ShaderCompileJob baseJob()
    {
        ShaderCompileJob j;
        j.shaderName = TkString("Pkg/A");
        j.passName = TkString("Forward");
        j.stage = ShaderStage::kVertex;
        j.profile = GfxShaderProfile::kGLSL_410;
        j.sourcePath = TkString("C:/abs/shaders/a.hlsl");
        j.virtualSourceName = TkString("Pkg/A/Forward");
        j.sourceText = TkString("float4 VSMain() : SV_Position { return float4(0,0,0,1); }");
        j.entryPoint = TkString("VSMain");
        j.variantKey = TkString();
        j.includeDirs.add(TkString("C:/abs/inc"));

        ShaderDependency dep;
        dep.resolvedPath = TkString("C:/abs/inc/common.hlsl");
        dep.contentHash.low = 0x1234567812345678ull;
        dep.contentHash.high = 0;
        j.dependencies.add(dep);
        j.dependencyHash.low = 0xAABBCCDDEEFF0011ull;
        return j;
    }
}

TEST(ShaderHashTest, HashJobIsDeterministicAcrossCalls)
{
    const ShaderCompileJob j = baseJob();
    const ShaderHash128 h1 = ShaderHash::hashJob(j);
    const ShaderHash128 h2 = ShaderHash::hashJob(j);
    EXPECT_TRUE(h1 == h2);
    EXPECT_FALSE(h1.isZero());
}

TEST(ShaderHashTest, HexEncodingIsThirtyTwoLowercaseChars)
{
    ShaderHash128 h{0x0123456789abcdefull, 0xfedcba9876543210ull};
    const TkString hex = ShaderHash::toHex(h);
    EXPECT_EQ(hex.length(), 32);
    for (i32 i = 0; i < hex.length(); i++)
    {
        const char c = hex[i];
        const bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        EXPECT_TRUE(isHex) << "non-lowercase-hex char at index " << i << " = '" << c << "'";
    }
}

TEST(ShaderHashTest, ChangingSourceTextChangesHash)
{
    ShaderCompileJob a = baseJob();
    ShaderCompileJob b = a;
    b.sourceText = a.sourceText + TkString(" /* tweak */");
    EXPECT_FALSE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, ChangingEntryPointChangesHash)
{
    ShaderCompileJob a = baseJob();
    ShaderCompileJob b = a;
    b.entryPoint = TkString("OtherEntry");
    EXPECT_FALSE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, ChangingProfileChangesHash)
{
    ShaderCompileJob a = baseJob();
    ShaderCompileJob b = a;
    b.profile = GfxShaderProfile::kVulkan_SM5;
    EXPECT_FALSE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, ChangingStageChangesHash)
{
    ShaderCompileJob a = baseJob();
    ShaderCompileJob b = a;
    b.stage = ShaderStage::kPixel;
    EXPECT_FALSE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, ChangingIncludeDirSetChangesHash)
{
    ShaderCompileJob a = baseJob();
    ShaderCompileJob b = a;
    b.includeDirs.add(TkString("C:/abs/extra"));
    EXPECT_FALSE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, ChangingDependencyHashChangesHash)
{
    ShaderCompileJob a = baseJob();
    ShaderCompileJob b = a;
    b.dependencyHash.low = a.dependencyHash.low ^ 0xDEADBEEFull;
    EXPECT_FALSE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, ChangingDefinesChangesHash)
{
    ShaderCompileJob a = baseJob();
    ShaderCompileJob b = a;
    ShaderMacro m;
    m.name = "USE_TEXTURE";
    m.definition = "1";
    b.defines.add(m);
    EXPECT_FALSE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, ChangingVariantKeyChangesHash)
{
    ShaderCompileJob a = baseJob();
    ShaderCompileJob b = a;
    b.variantKey = TkString("USE_TEXTURE=1");
    EXPECT_FALSE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, OutputNameDoesNotChangeHash)
{
    // outputName is a display-only field; reproducible cache keys must
    // ignore it.
    ShaderCompileJob a = baseJob();
    a.outputName = TkString("display-only");
    ShaderCompileJob b = a;
    b.outputName = TkString("something/else");
    EXPECT_TRUE(ShaderHash::hashJob(a) == ShaderHash::hashJob(b));
}

TEST(ShaderHashTest, EmptyJobStillHashesDeterministically)
{
    ShaderCompileJob empty;
    const ShaderHash128 h1 = ShaderHash::hashJob(empty);
    const ShaderHash128 h2 = ShaderHash::hashJob(empty);
    EXPECT_TRUE(h1 == h2);
}

TEST(ShaderHashTest, HashStringAndHashBytesAgreeOnAsciiPayload)
{
    const char text[] = "hello toolkit shader";
    const ShaderHash128 a = ShaderHash::hashString(TkStringView(text));
    const ShaderHash128 b = ShaderHash::hashBytes(text, static_cast<u32>(sizeof(text) - 1));
    EXPECT_TRUE(a == b);
}

TEST(ShaderHashTest, HashBytesSizeZeroIsDeterministic)
{
    const ShaderHash128 a = ShaderHash::hashBytes(nullptr, 0);
    const ShaderHash128 b = ShaderHash::hashBytes(nullptr, 0);
    EXPECT_TRUE(a == b);
}
