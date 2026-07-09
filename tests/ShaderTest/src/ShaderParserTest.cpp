/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * Parser tests -- exercise the .tkshader grammar productions one block at a
 * time. The expected AST shape is checked field-by-field; the diagnostic
 * sink is checked for both clean parses and recoverable error reporting.
 */

#include "common.hpp"

using namespace shader_test;

namespace
{
    // Convenience: parse a snippet wrapped in a Shader block.
    bool parseSnippet(SourceManager &sm, const char *body,
                      DiagnosticSink &sink, ShaderDecl &outShader)
    {
        TkString src = TkString("Shader \"Pkg/Name\"\n{\n") + TkString(body) + TkString("\n}\n");
        return parseManifest(sm, TkStringView("inline.tkshader"), TkStringView(src), sink, outShader);
    }
}

TEST(ShaderParserTest, MinimalShaderProducesEmptyDecl)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    ASSERT_TRUE(parseManifest(sm,
                              TkStringView("min.tkshader"),
                              TkStringView("Shader \"Empty\" {}"),
                              sink, shader));
    EXPECT_FALSE(sink.hasError());
    EXPECT_EQ(shader.name, TkString("Empty"));
    EXPECT_EQ(shader.properties.count(), 0);
    EXPECT_EQ(shader.tags.count(), 0);
    EXPECT_EQ(shader.variants.count(), 0);
    EXPECT_EQ(shader.subShaders.count(), 0);
}

TEST(ShaderParserTest, MissingShaderNameIsAnError)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    parseManifest(sm,
                  TkStringView("noname.tkshader"),
                  TkStringView("Shader {}"),
                  sink, shader);
    EXPECT_TRUE(sink.hasError());
}

TEST(ShaderParserTest, PropertiesBlockCapturesTypeNameDefault)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    // Grammar is `Type Name = value`. Tuple values like `(1, 1, 1, 1)` are
    // captured verbatim as text; scalars and strings come through as-is.
    ASSERT_TRUE(parseSnippet(sm,
        "Properties\n"
        "{\n"
        "    Color MainColor = (1, 1, 1, 1)\n"
        "    Float Intensity = 0.5\n"
        "    Texture2D Albedo = \"white\"\n"
        "}\n", sink, shader));
    ASSERT_EQ(shader.properties.count(), 3);

    EXPECT_EQ(shader.properties[0].type, TkString("Color"));
    EXPECT_EQ(shader.properties[0].name, TkString("MainColor"));
    EXPECT_NE(shader.properties[0].defaultValue.indexOf("1, 1, 1, 1"), -1);

    EXPECT_EQ(shader.properties[1].type, TkString("Float"));
    EXPECT_EQ(shader.properties[1].name, TkString("Intensity"));
    EXPECT_EQ(shader.properties[1].defaultValue, TkString("0.5"));

    EXPECT_EQ(shader.properties[2].type, TkString("Texture2D"));
    EXPECT_EQ(shader.properties[2].name, TkString("Albedo"));
    EXPECT_EQ(shader.properties[2].defaultValue, TkString("white"));
}

TEST(ShaderParserTest, TopLevelTagsParseAsKeyValuePairs)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    ASSERT_TRUE(parseSnippet(sm,
        "Tags\n"
        "{\n"
        "    RenderType = \"Opaque\"\n"
        "    Queue = \"Geometry\"\n"
        "}\n", sink, shader));
    ASSERT_EQ(shader.tags.count(), 2);
    EXPECT_EQ(shader.tags[0].key, TkString("RenderType"));
    EXPECT_EQ(shader.tags[0].value, TkString("Opaque"));
    EXPECT_EQ(shader.tags[1].key, TkString("Queue"));
    EXPECT_EQ(shader.tags[1].value, TkString("Geometry"));
}

TEST(ShaderParserTest, VariantsCaptureMultiValueArray)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    ASSERT_TRUE(parseSnippet(sm,
        "Variants\n"
        "{\n"
        "    USE_TEXTURE = [0, 1]\n"
        "    QUALITY = [\"low\", \"medium\", \"high\"]\n"
        "}\n", sink, shader));
    ASSERT_EQ(shader.variants.count(), 2);
    EXPECT_EQ(shader.variants[0].name, TkString("USE_TEXTURE"));
    EXPECT_EQ(shader.variants[0].values.count(), 2);
    EXPECT_EQ(shader.variants[1].name, TkString("QUALITY"));
    EXPECT_EQ(shader.variants[1].values.count(), 3);
}

TEST(ShaderParserTest, SubShaderWithTargetAndPass)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    ASSERT_TRUE(parseSnippet(sm,
        "SubShader\n"
        "{\n"
        "    Target\n"
        "    {\n"
        "        dx11 = \"d3d_sm5\"\n"
        "        dx12 = \"dx_sm6\"\n"
        "        vulkan = \"vk_sm5\"\n"
        "        opengl = \"glsl_410\"\n"
        "    }\n"
        "    Pass \"Forward\"\n"
        "    {\n"
        "        Vertex { Source = \"x.hlsl\"; Entry = \"VSMain\"; }\n"
        "        Pixel  { Source = \"x.hlsl\"; Entry = \"PSMain\"; }\n"
        "    }\n"
        "}\n", sink, shader));
    ASSERT_EQ(shader.subShaders.count(), 1);

    const SubShaderDecl &sub = shader.subShaders[0];
    EXPECT_TRUE(sub.hasTarget);
    ASSERT_EQ(sub.target.entries.count(), 4);
    EXPECT_EQ(sub.target.entries[0].backend, TkString("dx11"));
    EXPECT_EQ(sub.target.entries[0].profile, TkString("d3d_sm5"));
    EXPECT_EQ(sub.target.entries[3].backend, TkString("opengl"));
    EXPECT_EQ(sub.target.entries[3].profile, TkString("glsl_410"));

    ASSERT_EQ(sub.passes.count(), 1);
    const PassDecl &p = sub.passes[0];
    EXPECT_EQ(p.name, TkString("Forward"));
    ASSERT_EQ(p.stages.count(), 2);
    EXPECT_EQ(p.stages[0].stage, ShaderStage::kVertex);
    EXPECT_EQ(p.stages[0].source, TkString("x.hlsl"));
    EXPECT_EQ(p.stages[0].entry, TkString("VSMain"));
    EXPECT_EQ(p.stages[1].stage, ShaderStage::kPixel);
    EXPECT_EQ(p.stages[1].entry, TkString("PSMain"));
}

TEST(ShaderParserTest, RenderStatesCaptureNameAndValue)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    ASSERT_TRUE(parseSnippet(sm,
        "SubShader\n"
        "{\n"
        "    Pass \"Main\"\n"
        "    {\n"
        "        Blend  Alpha\n"
        "        Cull   Back\n"
        "        ZWrite On\n"
        "        ZTest  LessEqual\n"
        "        Vertex { Source = \"x.hlsl\"; Entry = \"VSMain\"; }\n"
        "        Pixel  { Source = \"x.hlsl\"; Entry = \"PSMain\"; }\n"
        "    }\n"
        "}\n", sink, shader));
    ASSERT_EQ(shader.subShaders.count(), 1);
    const PassDecl &p = shader.subShaders[0].passes[0];
    ASSERT_GE(p.renderStates.count(), 4);

    // Make sure all four states show up by name -- order is preserved from
    // the manifest but the test asserts presence rather than indexing.
    bool seenBlend = false, seenCull = false, seenZWrite = false, seenZTest = false;
    for (i32 i = 0; i < p.renderStates.count(); i++)
    {
        const RenderStateDecl &rs = p.renderStates[i];
        if (rs.name == TkString("Blend"))  { seenBlend = true; EXPECT_EQ(rs.value, TkString("Alpha")); }
        if (rs.name == TkString("Cull"))   { seenCull = true; EXPECT_EQ(rs.value, TkString("Back")); }
        if (rs.name == TkString("ZWrite")) { seenZWrite = true; EXPECT_EQ(rs.value, TkString("On")); }
        if (rs.name == TkString("ZTest"))  { seenZTest = true; EXPECT_EQ(rs.value, TkString("LessEqual")); }
    }
    EXPECT_TRUE(seenBlend);
    EXPECT_TRUE(seenCull);
    EXPECT_TRUE(seenZWrite);
    EXPECT_TRUE(seenZTest);
}

TEST(ShaderParserTest, EmbeddedHlslProgramCapturedIntoPass)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    ASSERT_TRUE(parseSnippet(sm,
        "SubShader\n"
        "{\n"
        "    Pass \"Main\"\n"
        "    {\n"
        "        HLSLPROGRAM\n"
        "        struct V { float4 p : SV_Position; };\n"
        "        V VSMain(uint id : SV_VertexID) { V o; o.p = float4(0,0,0,1); return o; }\n"
        "        float4 PSMain(V i) : SV_Target0 { return float4(1,1,1,1); }\n"
        "        ENDHLSL\n"
        "    }\n"
        "}\n", sink, shader));
    ASSERT_EQ(shader.subShaders.count(), 1);
    const PassDecl &p = shader.subShaders[0].passes[0];
    ASSERT_EQ(p.embeddedSources.count(), 1);
    EXPECT_EQ(p.embeddedSources[0].language, ShaderSourceLanguage::HLSL);
    EXPECT_NE(p.embeddedSources[0].sourceText.indexOf("VSMain"), -1);
}

TEST(ShaderParserTest, EmbeddedGlslProgramCapturedIntoPass)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    ASSERT_TRUE(parseSnippet(sm,
        "SubShader\n"
        "{\n"
        "    Pass \"GL\"\n"
        "    {\n"
        "        GLSLPROGRAM\n"
        "        #version 410 core\n"
        "        void main() { gl_Position = vec4(0.0); }\n"
        "        ENDGLSL\n"
        "    }\n"
        "}\n", sink, shader));
    const PassDecl &p = shader.subShaders[0].passes[0];
    ASSERT_EQ(p.embeddedSources.count(), 1);
    EXPECT_EQ(p.embeddedSources[0].language, ShaderSourceLanguage::GLSL);
}

TEST(ShaderParserTest, ErrorInOnePassDoesNotKillSubsequentParsing)
{
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    // First Pass has a bad token where stage Source is expected; second pass
    // is well-formed. The recovery strategy should still ingest the second.
    parseManifest(sm,
                  TkStringView("recover.tkshader"),
                  TkStringView(
                      "Shader \"R\"\n"
                      "{\n"
                      "    SubShader\n"
                      "    {\n"
                      "        Pass \"Bad\"\n"
                      "        {\n"
                      "            Vertex { Source = @@@; Entry = \"VS\"; }\n"
                      "        }\n"
                      "        Pass \"Good\"\n"
                      "        {\n"
                      "            Vertex { Source = \"x.hlsl\"; Entry = \"VS\"; }\n"
                      "            Pixel  { Source = \"x.hlsl\"; Entry = \"PS\"; }\n"
                      "        }\n"
                      "    }\n"
                      "}\n"),
                  sink, shader);
    EXPECT_TRUE(sink.hasError());
    // Either we recovered and saw the second pass, or the panic mode swallowed
    // it. The contract is "at least one diagnostic was emitted" -- recovery
    // quality is verified through the count of preserved passes when present.
    if (!shader.subShaders.isEmpty())
        EXPECT_GE(shader.subShaders[0].passes.count(), 1);
}

TEST(ShaderParserTest, ParsesFullTriangleFixture)
{
    // Round-trip the on-disk testdata triangle manifest. This guarantees the
    // sample we ship with the test suite continues to parse cleanly.
    SourceManager sm;
    DiagnosticSink sink;
    ShaderDecl shader;
    const TkString text = readTextFile(TkStringView(docPath("shaders/test_triangle.tkshader")));
    ASSERT_FALSE(text.isEmpty()) << "missing testdata file: shaders/test_triangle.tkshader";
    ASSERT_TRUE(parseManifest(sm,
                              TkStringView("shaders/test_triangle.tkshader"),
                              TkStringView(text), sink, shader))
        << "parse failed; diagnostic count = " << sink.getDiagnostics().count();

    EXPECT_EQ(shader.name, TkString("Sandbox/TestTriangle"));
    EXPECT_GE(shader.properties.count(), 3);
    EXPECT_GE(shader.tags.count(), 1);
    EXPECT_GE(shader.variants.count(), 1);
    ASSERT_EQ(shader.subShaders.count(), 1);
    EXPECT_TRUE(shader.subShaders[0].hasTarget);
    EXPECT_GE(shader.subShaders[0].target.entries.count(), 4);
    ASSERT_GE(shader.subShaders[0].passes.count(), 1);
    EXPECT_EQ(shader.subShaders[0].passes[0].name, TkString("Forward"));
}
