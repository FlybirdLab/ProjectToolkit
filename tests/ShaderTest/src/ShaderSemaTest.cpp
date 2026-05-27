/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * Semantic-analysis tests -- verify backend / profile / render-state /
 * stage validation rules. Each test feeds a parsed ShaderDecl through
 * ShaderSema and asserts the resulting diagnostic shape.
 */

#include "common.hpp"

using namespace shader_test;

namespace
{
    // Build a minimally-valid shader programmatically so each test can mutate
    // exactly the field it cares about and not fight the parser.
    ShaderDecl makeValidShader()
    {
        ShaderDecl s;
        s.name = TkString("Test/Valid");

        SubShaderDecl sub;
        sub.hasTarget = true;

        TargetEntryDecl t1; t1.backend = TkString("dx11");   t1.profile = TkString("d3d_sm5");
        TargetEntryDecl t2; t2.backend = TkString("dx12");   t2.profile = TkString("dx_sm6");
        TargetEntryDecl t3; t3.backend = TkString("vulkan"); t3.profile = TkString("vk_sm5");
        TargetEntryDecl t4; t4.backend = TkString("opengl"); t4.profile = TkString("glsl_410");
        sub.target.entries.add(t1);
        sub.target.entries.add(t2);
        sub.target.entries.add(t3);
        sub.target.entries.add(t4);

        PassDecl p;
        p.name = TkString("Forward");
        StageDecl vs; vs.stage = ShaderStage::kVertex; vs.source = TkString("x.hlsl"); vs.entry = TkString("VSMain");
        StageDecl ps; ps.stage = ShaderStage::kPixel;  ps.source = TkString("x.hlsl"); ps.entry = TkString("PSMain");
        p.stages.add(vs);
        p.stages.add(ps);
        sub.passes.add(p);
        s.subShaders.add(sub);
        return s;
    }

    bool runSema(const ShaderDecl &shader, DiagnosticSink &sink)
    {
        ShaderSema sema(&sink);
        return sema.validate(shader);
    }
}

TEST(ShaderSemaTest, AcceptsValidShader)
{
    DiagnosticSink sink;
    EXPECT_TRUE(runSema(makeValidShader(), sink));
    EXPECT_FALSE(sink.hasError());
}

TEST(ShaderSemaTest, EmptyShaderNameIsAnError)
{
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    s.name = TkString();
    EXPECT_FALSE(runSema(s, sink));
    EXPECT_TRUE(sink.hasError());
    EXPECT_TRUE(hasDiagnosticWithCode(sink, TkStringView("TKSHD1001")));
}

TEST(ShaderSemaTest, UnknownBackendInTargetReportsError)
{
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    s.subShaders[0].target.entries[0].backend = TkString("xbox360");
    EXPECT_FALSE(runSema(s, sink));
    EXPECT_TRUE(hasDiagnosticWithCode(sink, TkStringView("TKSHD1006")));
}

TEST(ShaderSemaTest, EmptyProfileStringReportsError)
{
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    s.subShaders[0].target.entries[0].profile = TkString();
    EXPECT_FALSE(runSema(s, sink));
    EXPECT_TRUE(hasDiagnosticWithCode(sink, TkStringView("TKSHD1006")));
}

TEST(ShaderSemaTest, UnknownProfileStringEmitsWarningNotError)
{
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    // dx11 only knows d3d_sm4 / d3d_sm5. "d3d_sm99" should warn but the
    // overall validation must still pass (advisory diagnostic).
    s.subShaders[0].target.entries[0].profile = TkString("d3d_sm99");
    const bool ok = runSema(s, sink);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(sink.hasError());
    EXPECT_GE(countDiagnosticsWithSeverity(sink, DiagnosticSeverity::Warning), 1);
}

TEST(ShaderSemaTest, KnownProfilePairsDoNotWarn)
{
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    // All four (backend, profile) pairs are known-good; sema must produce
    // zero warnings and zero errors.
    EXPECT_TRUE(runSema(s, sink));
    EXPECT_EQ(sink.getWarningCount(), 0u);
    EXPECT_EQ(sink.getErrorCount(),   0u);
}

TEST(ShaderSemaTest, PassWithoutVertexOrPixelIsAnError)
{
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    s.subShaders[0].passes[0].stages.clear();
    EXPECT_FALSE(runSema(s, sink));
    EXPECT_TRUE(sink.hasError());
}

TEST(ShaderSemaTest, UnknownRenderStateNameIsAnError)
{
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    RenderStateDecl rs;
    rs.name = TkString("ZWirte"); // misspelled on purpose
    rs.value = TkString("On");
    s.subShaders[0].passes[0].renderStates.add(rs);
    EXPECT_FALSE(runSema(s, sink));
    EXPECT_TRUE(sink.hasError());
}

TEST(ShaderSemaTest, InvalidRenderStateValueIsAnError)
{
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    RenderStateDecl rs;
    rs.name = TkString("ZTest");
    rs.value = TkString("Banana");
    s.subShaders[0].passes[0].renderStates.add(rs);
    EXPECT_FALSE(runSema(s, sink));
    EXPECT_TRUE(sink.hasError());
}

TEST(ShaderSemaTest, RenderStateAcceptsAllZTestValues)
{
    // ZTest enum has 10 valid values; sema's RenderStateRule values[] used to
    // be too small to hold them (regression we previously fixed). Re-verify.
    const char *ztest_values[] = {
        "Less", "LessEqual", "Equal", "GreaterEqual", "Greater",
        "NotEqual", "Always", "Never", "Off",
    };
    for (const char *v : ztest_values)
    {
        DiagnosticSink sink;
        ShaderDecl s = makeValidShader();
        RenderStateDecl rs;
        rs.name = TkString("ZTest");
        rs.value = TkString(v);
        s.subShaders[0].passes[0].renderStates.add(rs);
        EXPECT_TRUE(runSema(s, sink)) << "ZTest=" << v << " should be accepted";
    }
}

TEST(ShaderSemaTest, EmbeddedHlslIsAcceptedAlongsideExternalStages)
{
    // Passes that *only* embed sources also satisfy the "has vertex/pixel"
    // rule via the embedded fallback. We use a Pass with both an embedded
    // block AND external Vertex / Pixel just to make sure the union of
    // satisfaction paths still validates.
    DiagnosticSink sink;
    ShaderDecl s = makeValidShader();
    EmbeddedSourceDecl emb;
    emb.language = ShaderSourceLanguage::HLSL;
    emb.sourceText = TkString("// inline shader text");
    s.subShaders[0].passes[0].embeddedSources.add(emb);
    EXPECT_TRUE(runSema(s, sink));
}
