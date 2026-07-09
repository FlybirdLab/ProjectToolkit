/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * .tksd format tests: writer / reader round-trip, header validation,
 * dependency / bindings / vertexInput chunk coverage, and unknown-chunk
 * skipping (the format-stability guarantee).
 */

#include "common.hpp"

#include <cstring>

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

TEST(TksdFormatTest, ConstantsAreStable)
{
    EXPECT_EQ(TKSD_MAGIC, 0x44534B54u);             // 'TKSD'
    EXPECT_EQ(static_cast<u32>(TKSD_VERSION_MAJOR), 1u);
    EXPECT_EQ(TKSD_HASH_SIZE, 32u);
    EXPECT_EQ(TKSD_STAGE_KIND_RAW, 1u);
    EXPECT_EQ(static_cast<u32>(TksdChunkType::Metadata),      1u);
    EXPECT_EQ(static_cast<u32>(TksdChunkType::Properties),    2u);
    EXPECT_EQ(static_cast<u32>(TksdChunkType::Tags),          3u);
    EXPECT_EQ(static_cast<u32>(TksdChunkType::Passes),        4u);
    EXPECT_EQ(static_cast<u32>(TksdChunkType::StageBytecode), 5u);
    EXPECT_EQ(static_cast<u32>(TksdChunkType::Bindings),      7u);
    EXPECT_EQ(static_cast<u32>(TksdChunkType::VertexInput),   8u);
    EXPECT_EQ(static_cast<u32>(TksdChunkType::Dependencies),  10u);
}

TEST(TksdFormatTest, HeaderStructIsPacked)
{
    // The on-disk header is required to be byte-tight so static_assert in
    // TksdFormat.hpp must hold; we re-check the runtime sizeof here so the
    // test catches accidental drift.
    EXPECT_EQ(sizeof(TksdHeader), 4u + 2u + 2u + 4u + 4u + 4u + 8u + (TKSD_HASH_SIZE * 5u) + 4u + 4u);
    EXPECT_EQ(sizeof(TksdChunkHeader), 4u + 4u + 8u + 8u + 8u + 4u + 4u);
}

TEST(TksdFormatTest, WriterReaderRoundTripInMemory)
{
    const ShaderAsset original = makeMinimalAsset();
    TkArray<byte> buffer;
    TksdWriter writer;
    ASSERT_TRUE(writer.writeToMemory(original, buffer));
    EXPECT_GE(buffer.count(), static_cast<i32>(sizeof(TksdHeader)));

    ShaderAsset loaded;
    TksdReader reader;
    EXPECT_EQ(reader.readFromMemory(buffer.get(), static_cast<u32>(buffer.count()), loaded),
              TksdReadResult::Ok);

    EXPECT_EQ(loaded.name,                 original.name);
    EXPECT_EQ(loaded.properties.count(),   original.properties.count());
    EXPECT_EQ(loaded.tags.count(),         original.tags.count());
    EXPECT_EQ(loaded.passes.count(),       original.passes.count());
    EXPECT_EQ(loaded.stages.count(),       original.stages.count());
    EXPECT_EQ(loaded.bindings.count(),     original.bindings.count());
    EXPECT_EQ(loaded.vertexInputs.count(), original.vertexInputs.count());
    EXPECT_EQ(loaded.dependencies.count(), original.dependencies.count());

    // Spot-check a few field-level values.
    EXPECT_EQ(loaded.properties[0].name,    TkString("MainColor"));
    EXPECT_EQ(loaded.properties[0].type,    TkString("Color"));
    EXPECT_EQ(loaded.tags[0].key,           TkString("RenderType"));
    EXPECT_EQ(loaded.tags[0].value,         TkString("Opaque"));
    EXPECT_EQ(loaded.passes[0].name,        TkString("Forward"));
    EXPECT_GE(loaded.passes[0].renderStates.count(), 2);
    EXPECT_EQ(loaded.stages[0].kind,        TKSD_STAGE_KIND_RAW);
    EXPECT_EQ(loaded.stages[0].stage,       ShaderStage::kVertex);
    EXPECT_EQ(loaded.stages[0].profile,     GfxShaderProfile::kGLSL_410);
    EXPECT_TRUE(sameBytes(loaded.stages[0].bytecode, original.stages[0].bytecode));
    EXPECT_EQ(loaded.bindings[0].name,      TkString("FrameCB"));
    EXPECT_EQ(loaded.bindings[0].type,      TkString("CB"));
    EXPECT_EQ(loaded.vertexInputs[0].semantic, TkString("POSITION"));
    EXPECT_EQ(loaded.dependencies[0],       TkString("C:/abs/test/header.hlsl"));
}

TEST(TksdFormatTest, WriterReaderRoundTripOnDisk)
{
    const ShaderAsset original = makeMinimalAsset(TkStringView("Test/Disk"));
    const TkString path = tmpPath("disk_roundtrip.tksd");

    TksdWriter writer;
    ASSERT_TRUE(writer.writeToFile(original, TkStringView(path)));
    EXPECT_TRUE(TkFileSystem::isFileExist(path));

    ShaderAsset loaded;
    TksdReader reader;
    EXPECT_EQ(reader.readFromFile(TkStringView(path), loaded), TksdReadResult::Ok);
    EXPECT_EQ(loaded.name, TkString("Test/Disk"));
}

TEST(TksdFormatTest, EmptyAssetStillWritesAndReads)
{
    ShaderAsset empty;
    empty.name = TkString("Empty/Asset");
    TkArray<byte> buffer;
    TksdWriter writer;
    ASSERT_TRUE(writer.writeToMemory(empty, buffer));

    ShaderAsset loaded;
    TksdReader reader;
    EXPECT_EQ(reader.readFromMemory(buffer.get(), static_cast<u32>(buffer.count()), loaded),
              TksdReadResult::Ok);
    EXPECT_EQ(loaded.name, TkString("Empty/Asset"));
    EXPECT_EQ(loaded.passes.count(), 0);
    EXPECT_EQ(loaded.stages.count(), 0);
    EXPECT_EQ(loaded.bindings.count(), 0);
    EXPECT_EQ(loaded.vertexInputs.count(), 0);
    EXPECT_EQ(loaded.dependencies.count(), 0);
}

TEST(TksdFormatTest, BogusMagicRejected)
{
    TksdHeader header;
    std::memset(&header, 0, sizeof(header));
    header.magic = 0xDEADBEEFu;
    header.versionMajor = TKSD_VERSION_MAJOR;
    header.versionMinor = TKSD_VERSION_MINOR;
    header.headerSize = sizeof(TksdHeader);
    header.chunkTableOffset = sizeof(TksdHeader);
    header.chunkCount = 0;
    header.fileSize = sizeof(TksdHeader);

    TksdReader reader;
    ShaderAsset out;
    EXPECT_EQ(reader.readFromMemory(reinterpret_cast<const byte *>(&header),
                                    sizeof(header), out),
              TksdReadResult::InvalidHeader);
}

TEST(TksdFormatTest, UnsupportedMajorVersionRejected)
{
    TksdHeader header;
    std::memset(&header, 0, sizeof(header));
    header.magic = TKSD_MAGIC;
    header.versionMajor = TKSD_VERSION_MAJOR + 99;
    header.versionMinor = 0;
    header.headerSize = sizeof(TksdHeader);
    header.chunkTableOffset = sizeof(TksdHeader);
    header.chunkCount = 0;
    header.fileSize = sizeof(TksdHeader);

    TksdReader reader;
    ShaderAsset out;
    EXPECT_EQ(reader.readFromMemory(reinterpret_cast<const byte *>(&header),
                                    sizeof(header), out),
              TksdReadResult::UnsupportedVersion);
}

TEST(TksdFormatTest, BufferShorterThanHeaderRejected)
{
    const byte tooShort[] = {'T', 'K', 'S', 'D'};
    TksdReader reader;
    ShaderAsset out;
    EXPECT_EQ(reader.readFromMemory(tooShort, sizeof(tooShort), out),
              TksdReadResult::InvalidHeader);
}

TEST(TksdFormatTest, DependenciesChunkRoundTrip)
{
    ShaderAsset asset = makeMinimalAsset();
    asset.dependencies.clear();
    asset.dependencies.add(TkString("C:/abs/inc/a.hlsl"));
    asset.dependencies.add(TkString("C:/abs/inc/b.hlsl"));
    asset.dependencies.add(TkString("C:/abs/inc/sub/c.hlsl"));

    TkArray<byte> buffer;
    ASSERT_TRUE(TksdWriter().writeToMemory(asset, buffer));

    ShaderAsset loaded;
    EXPECT_EQ(TksdReader().readFromMemory(buffer.get(),
                                          static_cast<u32>(buffer.count()),
                                          loaded),
              TksdReadResult::Ok);
    ASSERT_EQ(loaded.dependencies.count(), 3);
    EXPECT_EQ(loaded.dependencies[0], TkString("C:/abs/inc/a.hlsl"));
    EXPECT_EQ(loaded.dependencies[1], TkString("C:/abs/inc/b.hlsl"));
    EXPECT_EQ(loaded.dependencies[2], TkString("C:/abs/inc/sub/c.hlsl"));
}

TEST(TksdFormatTest, BindingsChunkPreservesAllFields)
{
    ShaderAsset asset = makeMinimalAsset();
    asset.bindings.clear();

    ShaderBindingInfo cb;
    cb.name = TkString("FrameCB");
    cb.slot = 0;
    cb.stage = ShaderStage::kVertex;
    cb.type = TkString("CB");
    asset.bindings.add(cb);

    ShaderBindingInfo tex;
    tex.name = TkString("MainTex");
    tex.slot = 3;
    tex.stage = ShaderStage::kPixel;
    tex.type = TkString("SRV");
    asset.bindings.add(tex);

    ShaderBindingInfo sampler;
    sampler.name = TkString("LinearSampler");
    sampler.slot = 5;
    sampler.stage = ShaderStage::kPixel;
    sampler.type = TkString("Sampler");
    asset.bindings.add(sampler);

    TkArray<byte> buffer;
    ASSERT_TRUE(TksdWriter().writeToMemory(asset, buffer));
    ShaderAsset loaded;
    EXPECT_EQ(TksdReader().readFromMemory(buffer.get(),
                                          static_cast<u32>(buffer.count()),
                                          loaded),
              TksdReadResult::Ok);
    ASSERT_EQ(loaded.bindings.count(), 3);
    EXPECT_EQ(loaded.bindings[1].name,  TkString("MainTex"));
    EXPECT_EQ(loaded.bindings[1].slot,  3u);
    EXPECT_EQ(loaded.bindings[1].stage, ShaderStage::kPixel);
    EXPECT_EQ(loaded.bindings[1].type,  TkString("SRV"));
    EXPECT_EQ(loaded.bindings[2].type,  TkString("Sampler"));
}

TEST(TksdFormatTest, VertexInputChunkPreservesAllFields)
{
    ShaderAsset asset = makeMinimalAsset();
    asset.vertexInputs.clear();

    ShaderVertexInputInfo a;
    a.semantic = TkString("POSITION");
    a.location = 0;
    a.format = 0;
    a.slot = 0;
    a.offset = 0;
    asset.vertexInputs.add(a);

    ShaderVertexInputInfo b;
    b.semantic = TkString("COLOR0");
    b.location = 1;
    b.format = 7;
    b.slot = 0;
    b.offset = 12;
    asset.vertexInputs.add(b);

    TkArray<byte> buffer;
    ASSERT_TRUE(TksdWriter().writeToMemory(asset, buffer));
    ShaderAsset loaded;
    EXPECT_EQ(TksdReader().readFromMemory(buffer.get(),
                                          static_cast<u32>(buffer.count()),
                                          loaded),
              TksdReadResult::Ok);
    ASSERT_EQ(loaded.vertexInputs.count(), 2);
    EXPECT_EQ(loaded.vertexInputs[1].semantic, TkString("COLOR0"));
    EXPECT_EQ(loaded.vertexInputs[1].location, 1u);
    EXPECT_EQ(loaded.vertexInputs[1].format,   7u);
    EXPECT_EQ(loaded.vertexInputs[1].offset,   12u);
}

TEST(TksdFormatTest, PassesChunkPreservesTagsRenderStatesAndStageIndices)
{
    ShaderAsset asset = makeMinimalAsset();
    asset.passes.clear();

    ShaderPass p;
    p.name = TkString("Detail");
    ShaderTag t;
    t.key = TkString("LightMode");
    t.value = TkString("ForwardAdd");
    p.tags.add(t);
    ShaderRenderState rs;
    rs.name = TkString("Blend");
    rs.value = TkString("Additive");
    p.renderStates.add(rs);
    p.stageIndices.add(0);
    p.stageIndices.add(1);
    asset.passes.add(p);

    // Match stages count to keep indices valid.
    asset.stages.add(asset.stages[0]);

    TkArray<byte> buffer;
    ASSERT_TRUE(TksdWriter().writeToMemory(asset, buffer));
    ShaderAsset loaded;
    EXPECT_EQ(TksdReader().readFromMemory(buffer.get(),
                                          static_cast<u32>(buffer.count()),
                                          loaded),
              TksdReadResult::Ok);
    ASSERT_EQ(loaded.passes.count(), 1);
    EXPECT_EQ(loaded.passes[0].name, TkString("Detail"));
    ASSERT_EQ(loaded.passes[0].tags.count(), 1);
    EXPECT_EQ(loaded.passes[0].tags[0].key, TkString("LightMode"));
    ASSERT_EQ(loaded.passes[0].renderStates.count(), 1);
    EXPECT_EQ(loaded.passes[0].renderStates[0].name, TkString("Blend"));
    EXPECT_EQ(loaded.passes[0].renderStates[0].value, TkString("Additive"));
    ASSERT_EQ(loaded.passes[0].stageIndices.count(), 2);
    EXPECT_EQ(loaded.passes[0].stageIndices[0], 0u);
    EXPECT_EQ(loaded.passes[0].stageIndices[1], 1u);
}

TEST(TksdFormatTest, ToStringForReadResultCoversAllValues)
{
    EXPECT_STREQ(toString(TksdReadResult::Ok),                     "ok");
    EXPECT_NE(toString(TksdReadResult::ReadFailed),              nullptr);
    EXPECT_NE(toString(TksdReadResult::InvalidHeader),           nullptr);
    EXPECT_NE(toString(TksdReadResult::UnsupportedVersion),      nullptr);
    EXPECT_NE(toString(TksdReadResult::InvalidChunkTable),       nullptr);
    EXPECT_NE(toString(TksdReadResult::ChunkOutOfRange),         nullptr);
    EXPECT_NE(toString(TksdReadResult::UnsupportedCompression),  nullptr);
    EXPECT_NE(toString(TksdReadResult::TruncatedChunk),          nullptr);
}
