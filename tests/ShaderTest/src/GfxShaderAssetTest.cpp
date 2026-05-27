/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * Runtime-side GfxShaderAsset / GfxShaderAssetLoader tests. These do NOT
 * create a real GfxDevice (that belongs in GraphicsTest); they cover the
 * parts that work without a device:
 *   - loadFromMemory accepts a TksdWriter-produced buffer
 *   - findPass / getDefaultPass behave correctly
 *   - lastError() is populated on negative paths
 *   - bindings() / vertexInputs() / dependencies() accessors round-trip
 *   - createShaderForPass returns nullptr on null device (and does NOT leak
 *     a half-initialised shader -- the contract we just fixed)
 *
 * The actual `shader->create(stream)` failure path needs a GfxDevice that
 * intentionally returns failure, which is out of scope here.
 */

#include "common.hpp"

using namespace shader_test;

namespace
{
    TkArray<byte> serializeToBuffer(const ShaderAsset &asset)
    {
        TkArray<byte> buffer;
        TksdWriter w;
        w.writeToMemory(asset, buffer);
        return buffer;
    }
}

TEST(GfxShaderAssetTest, LoadFromMemorySucceedsForMinimalAsset)
{
    const ShaderAsset original = makeMinimalAsset(TkStringView("Test/Runtime"));
    const TkArray<byte> buffer = serializeToBuffer(original);

    GfxShaderAsset asset;
    EXPECT_TRUE(asset.loadFromMemory(buffer.get(), static_cast<u32>(buffer.count())));
    EXPECT_TRUE(asset.lastError().isEmpty());
    EXPECT_EQ(asset.asset().name, TkString("Test/Runtime"));
}

TEST(GfxShaderAssetTest, LoadFromMemoryFailsAndPopulatesLastErrorOnBadHeader)
{
    GfxShaderAsset asset;
    const byte garbage[16] = {};
    EXPECT_FALSE(asset.loadFromMemory(garbage, sizeof(garbage)));
    EXPECT_FALSE(asset.lastError().isEmpty());
}

TEST(GfxShaderAssetTest, LoadFromFileSucceedsForDiskRoundTrip)
{
    const TkString path = tmpPath("runtime_disk.tksd");
    {
        TksdWriter w;
        ASSERT_TRUE(w.writeToFile(makeMinimalAsset(TkStringView("Test/Disk")), TkStringView(path)));
    }

    GfxShaderAsset asset;
    EXPECT_TRUE(asset.loadFromFile(TkStringView(path)));
    EXPECT_EQ(asset.asset().name, TkString("Test/Disk"));
}

TEST(GfxShaderAssetTest, LoadFromFileFailsOnMissingPath)
{
    GfxShaderAsset asset;
    EXPECT_FALSE(asset.loadFromFile(TkStringView("Z:/no/such/file.tksd")));
    EXPECT_FALSE(asset.lastError().isEmpty());
}

TEST(GfxShaderAssetTest, GetDefaultPassReturnsFirstPass)
{
    ShaderAsset src = makeMinimalAsset();
    const TkArray<byte> buffer = serializeToBuffer(src);
    GfxShaderAsset asset;
    ASSERT_TRUE(asset.loadFromMemory(buffer.get(), static_cast<u32>(buffer.count())));
    const ShaderPass *p = asset.getDefaultPass();
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->name, TkString("Forward"));
}

TEST(GfxShaderAssetTest, FindPassReturnsNullForUnknownName)
{
    ShaderAsset src = makeMinimalAsset();
    const TkArray<byte> buffer = serializeToBuffer(src);
    GfxShaderAsset asset;
    ASSERT_TRUE(asset.loadFromMemory(buffer.get(), static_cast<u32>(buffer.count())));
    EXPECT_EQ(asset.findPass(TkStringView("DoesNotExist")), nullptr);
}

TEST(GfxShaderAssetTest, FindPassReturnsCorrectPassWhenPresent)
{
    ShaderAsset src = makeMinimalAsset();
    // Add a second named pass so findPass has something to discriminate.
    ShaderPass extra;
    extra.name = TkString("Detail");
    extra.stageIndices.add(0);
    src.passes.add(extra);

    const TkArray<byte> buffer = serializeToBuffer(src);
    GfxShaderAsset asset;
    ASSERT_TRUE(asset.loadFromMemory(buffer.get(), static_cast<u32>(buffer.count())));
    const ShaderPass *p = asset.findPass(TkStringView("Detail"));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->name, TkString("Detail"));
}

TEST(GfxShaderAssetTest, AccessorsExposeBindingsVertexInputsDependencies)
{
    ShaderAsset src = makeMinimalAsset();
    const TkArray<byte> buffer = serializeToBuffer(src);
    GfxShaderAsset asset;
    ASSERT_TRUE(asset.loadFromMemory(buffer.get(), static_cast<u32>(buffer.count())));

    EXPECT_EQ(asset.bindings().count(),     src.bindings.count());
    EXPECT_EQ(asset.vertexInputs().count(), src.vertexInputs.count());
    EXPECT_EQ(asset.dependencies().count(), src.dependencies.count());

    EXPECT_EQ(asset.bindings()[0].name,        TkString("FrameCB"));
    EXPECT_EQ(asset.vertexInputs()[0].semantic, TkString("POSITION"));
    EXPECT_EQ(asset.dependencies()[0],         TkString("C:/abs/test/header.hlsl"));
}

TEST(GfxShaderAssetTest, CreateShaderForPassRejectsNullDevice)
{
    ShaderAsset src = makeMinimalAsset();
    const TkArray<byte> buffer = serializeToBuffer(src);
    GfxShaderAsset asset;
    ASSERT_TRUE(asset.loadFromMemory(buffer.get(), static_cast<u32>(buffer.count())));
    EXPECT_EQ(asset.createShaderForPass(nullptr, TkStringView()), nullptr);
    EXPECT_FALSE(asset.lastError().isEmpty());
}

TEST(GfxShaderAssetTest, CreateShaderForPassFailsWhenNoPasses)
{
    ShaderAsset src;
    src.name = TkString("Empty");
    const TkArray<byte> buffer = serializeToBuffer(src);
    GfxShaderAsset asset;
    ASSERT_TRUE(asset.loadFromMemory(buffer.get(), static_cast<u32>(buffer.count())));
    // device pointer goes unused once we hit the empty-pass check.
    EXPECT_EQ(asset.createShaderForPass(nullptr, TkStringView()), nullptr);
    EXPECT_FALSE(asset.lastError().isEmpty());
}

TEST(GfxShaderAssetTest, CreateShaderForPassFailsForUnknownPassName)
{
    ShaderAsset src = makeMinimalAsset();
    const TkArray<byte> buffer = serializeToBuffer(src);
    GfxShaderAsset asset;
    ASSERT_TRUE(asset.loadFromMemory(buffer.get(), static_cast<u32>(buffer.count())));
    EXPECT_EQ(asset.createShaderForPass(nullptr, TkStringView("Nope")), nullptr);
    EXPECT_FALSE(asset.lastError().isEmpty());
}

TEST(GfxShaderAssetLoaderTest, LoadAssetFromTksdParsesIntoOutAsset)
{
    const TkString path = tmpPath("loader_round.tksd");
    {
        TksdWriter w;
        ASSERT_TRUE(w.writeToFile(makeMinimalAsset(TkStringView("Loader/Asset")),
                                   TkStringView(path)));
    }

    ShaderAsset out;
    TkString err;
    EXPECT_TRUE(GfxShaderAssetLoader::loadAssetFromTksd(TkStringView(path), out, &err));
    EXPECT_TRUE(err.isEmpty());
    EXPECT_EQ(out.name, TkString("Loader/Asset"));
}

TEST(GfxShaderAssetLoaderTest, LoadAssetFromTksdReportsErrorOnMissingFile)
{
    ShaderAsset out;
    TkString err;
    EXPECT_FALSE(GfxShaderAssetLoader::loadAssetFromTksd(TkStringView("Z:/nope.tksd"), out, &err));
    EXPECT_FALSE(err.isEmpty());
}

TEST(GfxShaderAssetLoaderTest, LoadShaderFromTksdReturnsNullForNullDevice)
{
    // Even with a valid .tksd on disk, nullptr device must short-circuit
    // without creating any GPU object.
    const TkString path = tmpPath("loader_null_device.tksd");
    {
        TksdWriter w;
        ASSERT_TRUE(w.writeToFile(makeMinimalAsset(TkStringView("Loader/NullDev")),
                                   TkStringView(path)));
    }
    TkString err;
    GfxShader *s = GfxShaderAssetLoader::loadShaderFromTksd(
        nullptr, TkStringView(path), TkStringView(), &err);
    EXPECT_EQ(s, nullptr);
    EXPECT_FALSE(err.isEmpty());
}
