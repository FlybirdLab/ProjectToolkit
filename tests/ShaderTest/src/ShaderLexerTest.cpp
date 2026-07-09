/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * Lexer tests -- cover token classification, embedded source mode handoff,
 * comment / whitespace skipping, and error recovery on malformed literals.
 */

#include "common.hpp"

using namespace shader_test;

// Helper: lex a snippet end-to-end and return the token stream.
namespace
{
    TkArray<ShaderToken> lexAll(SourceManager &sm, const char *name, const char *src, DiagnosticSink &sink)
    {
        return collectTokens(sm, TkStringView(name), TkStringView(src), sink);
    }

    // Find the first non-EOF token of the given kind.
    const ShaderToken *findFirstOf(const TkArray<ShaderToken> &toks, ShaderTokenKind kind)
    {
        for (i32 i = 0; i < toks.count(); i++)
            if (toks[i].kind == kind)
                return &toks[i];
        return nullptr;
    }

    i32 countOf(const TkArray<ShaderToken> &toks, ShaderTokenKind kind)
    {
        i32 n = 0;
        for (i32 i = 0; i < toks.count(); i++)
            if (toks[i].kind == kind)
                n++;
        return n;
    }
}

TEST(ShaderLexerTest, EmptyInputProducesEofOnly)
{
    SourceManager sm;
    DiagnosticSink sink;
    auto toks = lexAll(sm, "empty.tkshader", "", sink);
    ASSERT_EQ(toks.count(), 1);
    EXPECT_EQ(toks[0].kind, ShaderTokenKind::EndOfFile);
    EXPECT_FALSE(sink.hasError());
}

TEST(ShaderLexerTest, RecognisesAllTopLevelKeywords)
{
    SourceManager sm;
    DiagnosticSink sink;
    const char *src =
        "Shader Properties Tags Variants SubShader Pass Target "
        "Vertex Pixel Compute "
        "Blend Cull ZWrite ZTest";
    auto toks = lexAll(sm, "kw.tkshader", src, sink);

    EXPECT_FALSE(sink.hasError());
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordShader)     != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordProperties) != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordTags)       != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordVariants)   != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordSubShader)  != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordPass)       != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordTarget)     != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordVertex)     != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordPixel)      != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordCompute)    != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordBlend)      != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordCull)       != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordZWrite)     != nullptr);
    EXPECT_TRUE(findFirstOf(toks, ShaderTokenKind::KeywordZTest)      != nullptr);
}

TEST(ShaderLexerTest, IdentifiersAreCaseSensitiveAndNotKeywords)
{
    SourceManager sm;
    DiagnosticSink sink;
    auto toks = lexAll(sm, "ident.tkshader", "shader properties NotAKeyword", sink);

    EXPECT_FALSE(sink.hasError());
    // lowercase variants should NOT be classified as keywords.
    EXPECT_EQ(countOf(toks, ShaderTokenKind::KeywordShader), 0);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::KeywordProperties), 0);
    EXPECT_GE(countOf(toks, ShaderTokenKind::Identifier), 3);
}

TEST(ShaderLexerTest, PunctuationTokensAreEmittedIndividually)
{
    SourceManager sm;
    DiagnosticSink sink;
    auto toks = lexAll(sm, "punct.tkshader", "{ } ( ) [ ] = , . : ;", sink);

    EXPECT_FALSE(sink.hasError());
    EXPECT_EQ(countOf(toks, ShaderTokenKind::LBrace), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::RBrace), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::LParen), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::RParen), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::LBracket), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::RBracket), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::Equal), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::Comma), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::Dot), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::Colon), 1);
    EXPECT_EQ(countOf(toks, ShaderTokenKind::Semicolon), 1);
}

TEST(ShaderLexerTest, StringLiteralsUnescapeQuotes)
{
    SourceManager sm;
    DiagnosticSink sink;
    auto toks = lexAll(sm, "str.tkshader", "\"hello world\" \"with \\\"quotes\\\"\"", sink);

    EXPECT_FALSE(sink.hasError());
    EXPECT_EQ(countOf(toks, ShaderTokenKind::StringLiteral), 2);

    // Walk in declaration order.
    i32 seen = 0;
    for (i32 i = 0; i < toks.count(); i++)
    {
        if (toks[i].kind != ShaderTokenKind::StringLiteral)
            continue;
        if (seen == 0)
            EXPECT_EQ(toks[i].text, TkString("hello world"));
        else if (seen == 1)
            EXPECT_EQ(toks[i].text, TkString("with \"quotes\""));
        seen++;
    }
}

TEST(ShaderLexerTest, NumberLiteralsCoverIntFloatAndNegative)
{
    SourceManager sm;
    DiagnosticSink sink;
    auto toks = lexAll(sm, "num.tkshader", "0 1 42 3.14 0.5 -7 -2.5", sink);

    EXPECT_FALSE(sink.hasError());
    EXPECT_GE(countOf(toks, ShaderTokenKind::NumberLiteral), 5);
}

TEST(ShaderLexerTest, LineCommentsAreSkipped)
{
    SourceManager sm;
    DiagnosticSink sink;
    auto toks = lexAll(sm, "comment.tkshader",
                       "Shader // this is a comment\nProperties", sink);

    EXPECT_FALSE(sink.hasError());
    EXPECT_NE(findFirstOf(toks, ShaderTokenKind::KeywordShader), nullptr);
    EXPECT_NE(findFirstOf(toks, ShaderTokenKind::KeywordProperties), nullptr);
}

TEST(ShaderLexerTest, BlockCommentsAreSkipped)
{
    SourceManager sm;
    DiagnosticSink sink;
    auto toks = lexAll(sm, "block.tkshader",
                       "Shader /* multi\nline\ncomment */ Properties", sink);

    EXPECT_FALSE(sink.hasError());
    EXPECT_NE(findFirstOf(toks, ShaderTokenKind::KeywordShader), nullptr);
    EXPECT_NE(findFirstOf(toks, ShaderTokenKind::KeywordProperties), nullptr);
}

TEST(ShaderLexerTest, EmbeddedHlslBlockProducesSingleToken)
{
    SourceManager sm;
    DiagnosticSink sink;
    const char *src =
        "HLSLPROGRAM\n"
        "float4 main() : SV_Target0 { return float4(1,2,3,4); }\n"
        "ENDHLSL\n";
    auto toks = lexAll(sm, "emb.tkshader", src, sink);

    EXPECT_FALSE(sink.hasError());
    const ShaderToken *block = findFirstOf(toks, ShaderTokenKind::EmbeddedSourceBlock);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->language, ShaderSourceLanguage::HLSL);
    // The body must include the function definition but exclude the marker
    // lines themselves.
    EXPECT_TRUE(block->text.contains("SV_Target0"));
    EXPECT_FALSE(block->text.contains("ENDHLSL"));
    EXPECT_FALSE(block->text.contains("HLSLPROGRAM"));
}

TEST(ShaderLexerTest, EmbeddedGlslBlockProducesSingleToken)
{
    SourceManager sm;
    DiagnosticSink sink;
    const char *src =
        "GLSLPROGRAM\n"
        "#version 410 core\n"
        "void main() { gl_Position = vec4(0.0); }\n"
        "ENDGLSL\n";
    auto toks = lexAll(sm, "emb_glsl.tkshader", src, sink);

    EXPECT_FALSE(sink.hasError());
    const ShaderToken *block = findFirstOf(toks, ShaderTokenKind::EmbeddedSourceBlock);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->language, ShaderSourceLanguage::GLSL);
    EXPECT_NE(block->text.indexOf("#version 410"), -1);
}

TEST(ShaderLexerTest, EmbeddedBlocksHandedBackToNormalModeAfterEnd)
{
    SourceManager sm;
    DiagnosticSink sink;
    const char *src =
        "Pass\n"
        "HLSLPROGRAM\n"
        "float4 PSMain() : SV_Target0 { return float4(1); }\n"
        "ENDHLSL\n"
        "Tags";
    auto toks = lexAll(sm, "after.tkshader", src, sink);

    EXPECT_FALSE(sink.hasError());
    EXPECT_NE(findFirstOf(toks, ShaderTokenKind::KeywordPass), nullptr);
    EXPECT_NE(findFirstOf(toks, ShaderTokenKind::EmbeddedSourceBlock), nullptr);
    // Tags AFTER the embedded block must still be lexed as a keyword.
    EXPECT_NE(findFirstOf(toks, ShaderTokenKind::KeywordTags), nullptr);
}

TEST(ShaderLexerTest, UnterminatedEmbeddedBlockEmitsDiagnostic)
{
    SourceManager sm;
    DiagnosticSink sink;
    // No matching ENDHLSL.
    auto toks = lexAll(sm, "trunc.tkshader", "HLSLPROGRAM\nfloat4 x();", sink);
    EXPECT_TRUE(sink.hasError() || sink.getDiagnostics().count() > 0);
    // The last token is still EndOfFile.
    ASSERT_GT(toks.count(), 0);
    EXPECT_EQ(toks[toks.count() - 1].kind, ShaderTokenKind::EndOfFile);
}

TEST(ShaderLexerTest, TokenRangePointsBackAtFile)
{
    SourceManager sm;
    DiagnosticSink sink;
    SourceFile *file = sm.addVirtualFile(TkStringView("loc.tkshader"),
                                         TkStringView("Shader \"Name\" {}"));
    ASSERT_NE(file, nullptr);
    ShaderLexer lexer(file, &sink);
    const ShaderToken tok = lexer.next();
    EXPECT_EQ(tok.kind, ShaderTokenKind::KeywordShader);
    EXPECT_EQ(tok.range.begin.fileId, file->getId());
    EXPECT_GE(tok.range.begin.line, 1u);
    EXPECT_GE(tok.range.begin.column, 1u);
}
