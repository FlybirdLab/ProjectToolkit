/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * Diagnostic sink + printer tests. The sink is the structured collection
 * point that lexer / parser / sema / manifest compiler all feed into; the
 * printers (text + json) are the user-facing formatters. We test counters,
 * filtering by severity, code lookup, clear(), and that the printers run
 * to completion without crashing on representative inputs.
 */

#include "common.hpp"

using namespace shader_test;

namespace
{
    SourceRange makeRange(u32 fileId, u32 line, u32 col, u32 length = 1)
    {
        SourceRange r;
        r.begin.fileId = fileId;
        r.begin.line = line;
        r.begin.column = col;
        r.begin.offset = 0;
        r.end.fileId = fileId;
        r.end.line = line;
        r.end.column = col + length;
        r.end.offset = length;
        return r;
    }
}

// ---------- DiagnosticSink ----------

TEST(DiagnosticSinkTest, FreshSinkHasZeroCounters)
{
    DiagnosticSink s;
    EXPECT_FALSE(s.hasError());
    EXPECT_FALSE(s.hasFatal());
    EXPECT_EQ(s.getNoteCount(),    0u);
    EXPECT_EQ(s.getWarningCount(), 0u);
    EXPECT_EQ(s.getErrorCount(),   0u);
    EXPECT_EQ(s.getFatalCount(),   0u);
    EXPECT_EQ(s.getDiagnostics().count(), 0);
}

TEST(DiagnosticSinkTest, NoteWarningErrorFatalBumpRespectiveCounters)
{
    DiagnosticSink s;
    s.note   (makeRange(1, 1, 1), TkStringView("TKSHD-N"), TkStringView("note"));
    s.warning(makeRange(1, 2, 1), TkStringView("TKSHD-W"), TkStringView("warning"));
    s.error  (makeRange(1, 3, 1), TkStringView("TKSHD-E"), TkStringView("error"));
    s.fatal  (makeRange(1, 4, 1), TkStringView("TKSHD-F"), TkStringView("fatal"));

    EXPECT_EQ(s.getNoteCount(),    1u);
    EXPECT_EQ(s.getWarningCount(), 1u);
    EXPECT_EQ(s.getErrorCount(),   1u);
    EXPECT_EQ(s.getFatalCount(),   1u);
    EXPECT_TRUE(s.hasError());
    EXPECT_TRUE(s.hasFatal());
    EXPECT_EQ(s.getDiagnostics().count(), 4);
}

TEST(DiagnosticSinkTest, ErrorsButNoFatalsDistinguishesHasErrorFromHasFatal)
{
    DiagnosticSink s;
    s.error(makeRange(1, 1, 1), TkStringView("E"), TkStringView("only error"));
    EXPECT_TRUE(s.hasError());
    EXPECT_FALSE(s.hasFatal());
}

TEST(DiagnosticSinkTest, ClearResetsAllCountersAndDiagnostics)
{
    DiagnosticSink s;
    s.error(makeRange(1, 1, 1), TkStringView("E"), TkStringView("e1"));
    s.warning(makeRange(1, 1, 1), TkStringView("W"), TkStringView("w1"));
    s.clear();
    EXPECT_FALSE(s.hasError());
    EXPECT_EQ(s.getErrorCount(), 0u);
    EXPECT_EQ(s.getWarningCount(), 0u);
    EXPECT_EQ(s.getDiagnostics().count(), 0);
}

TEST(DiagnosticSinkTest, ReportPreservesSeverityCodeMessageRangeOnDiagnostic)
{
    DiagnosticSink s;
    Diagnostic d;
    d.severity = DiagnosticSeverity::Error;
    d.code = TkString("TKSHD-CUST");
    d.message = TkString("custom message");
    d.range = makeRange(7, 10, 4, 5);
    d.backend = TkString("opengl");
    d.stage = TkString("vertex");
    s.report(d);

    ASSERT_EQ(s.getDiagnostics().count(), 1);
    const Diagnostic &saved = s.getDiagnostics()[0];
    EXPECT_EQ(saved.severity, DiagnosticSeverity::Error);
    EXPECT_EQ(saved.code, TkString("TKSHD-CUST"));
    EXPECT_EQ(saved.message, TkString("custom message"));
    EXPECT_EQ(saved.range.begin.line, 10u);
    EXPECT_EQ(saved.range.begin.column, 4u);
    EXPECT_EQ(saved.backend, TkString("opengl"));
    EXPECT_EQ(saved.stage, TkString("vertex"));
}

TEST(DiagnosticSinkTest, FindByCodeMatchesPredicateHelper)
{
    DiagnosticSink s;
    s.error(makeRange(1, 1, 1), TkStringView("TKSHD0008"), TkStringView("parse failed"));
    s.warning(makeRange(1, 2, 1), TkStringView("TKSHD0011"), TkStringView("variants ignored"));
    EXPECT_TRUE(hasDiagnosticWithCode(s, TkStringView("TKSHD0008")));
    EXPECT_TRUE(hasDiagnosticWithCode(s, TkStringView("TKSHD0011")));
    EXPECT_FALSE(hasDiagnosticWithCode(s, TkStringView("TKSHD9999")));
}

// ---------- Diagnostic code constants ----------

TEST(DiagnosticCodesTest, KnownCodeConstantsAreStableStrings)
{
    EXPECT_EQ(TkString(TKSHD_READ_SOURCE_FAILED),         TkString("TKSHD0001"));
    EXPECT_EQ(TkString(TKSHD_WRITE_OUTPUT_FAILED),        TkString("TKSHD0002"));
    EXPECT_EQ(TkString(TKSHD_UNSUPPORTED_BACKEND),        TkString("TKSHD0003"));
    EXPECT_EQ(TkString(TKSHD_UNSUPPORTED_PROFILE),        TkString("TKSHD0004"));
    EXPECT_EQ(TkString(TKSHD_PARSE_FAILED),               TkString("TKSHD0008"));
    EXPECT_EQ(TkString(TKSHD_INCLUDE_NOT_FOUND),          TkString("TKSHD0009"));
    EXPECT_EQ(TkString(TKSHD_FEATURE_NOT_IMPLEMENTED),    TkString("TKSHD0011"));
    EXPECT_EQ(TkString(TKSHD_BACKEND_COMPILE_FAILED),     TkString("TKSHD0100"));
    EXPECT_EQ(TkString(TKSHD_PACKAGING_FAILED),           TkString("TKSHD0200"));
    EXPECT_EQ(TkString(TKSHD_TKSD_INVALID_HEADER),        TkString("TKSHD0201"));
    EXPECT_EQ(TkString(TKSHD_TKSD_UNSUPPORTED_VERSION),   TkString("TKSHD0202"));
    EXPECT_EQ(TkString(TKSHD_TKSD_INVALID_CHUNK_TABLE),   TkString("TKSHD0203"));
}

// ---------- TextDiagnosticPrinter ----------

TEST(TextDiagnosticPrinterTest, RunsWithoutCrashingOnRichDiagnosticSet)
{
    SourceManager sm;
    SourceFile *file = sm.addVirtualFile(TkStringView("printed.tkshader"),
                                         TkStringView("line one\nline two\nline three"));
    ASSERT_NE(file, nullptr);

    DiagnosticSink sink;
    sink.warning(makeRange(file->getId(), 1, 6), TkStringView("TKSHD-WTEST"), TkStringView("a warning"));
    Diagnostic err;
    err.severity = DiagnosticSeverity::Error;
    err.code = TkString("TKSHD-ETEST");
    err.message = TkString("an error with backend tag");
    err.range = makeRange(file->getId(), 2, 1, 4);
    err.backend = TkString("vulkan");
    DiagnosticNote note;
    note.message = TkString("did you mean 'line two'?");
    note.range = makeRange(file->getId(), 2, 1, 8);
    err.notes.add(note);
    sink.report(err);

    // Pipe the printer output into a temporary file so we don't spam test
    // logs but still exercise the full write path.
    const TkString outPath = tmpPath("text_printer_out.txt");
    std::FILE *stream = std::fopen(outPath.get(), "wb");
    ASSERT_NE(stream, nullptr);
    TextDiagnosticPrinter printer(stream, /*useColor=*/false);
    printer.print(sink, sm);
    std::fclose(stream);

    const TkString contents = readTextFile(TkStringView(outPath));
    EXPECT_FALSE(contents.isEmpty());
    EXPECT_NE(contents.indexOf("TKSHD-ETEST"), -1);
    EXPECT_NE(contents.indexOf("TKSHD-WTEST"), -1);
    EXPECT_NE(contents.indexOf("error"), -1);
    EXPECT_NE(contents.indexOf("warning"), -1);
}

// ---------- JsonDiagnosticPrinter ----------

TEST(JsonDiagnosticPrinterTest, ProducesParseableJsonShape)
{
    SourceManager sm;
    SourceFile *file = sm.addVirtualFile(TkStringView("json.tkshader"),
                                         TkStringView("a\nb\nc"));
    ASSERT_NE(file, nullptr);

    DiagnosticSink sink;
    sink.error(makeRange(file->getId(), 1, 1), TkStringView("TKSHD-J1"), TkStringView("first"));
    sink.warning(makeRange(file->getId(), 2, 1), TkStringView("TKSHD-J2"), TkStringView("second"));

    // JsonDiagnosticPrinter writes into the supplied TkString when one is
    // provided -- nicer for tests than a FILE* round-trip, and lets us
    // assert on the raw JSON without re-reading from disk.
    TkString json;
    JsonDiagnosticPrinter printer(&json);
    printer.print(sink, sm);

    EXPECT_FALSE(json.isEmpty());
    // Look for JSON structural markers + at least the code strings. We don't
    // parse the JSON -- this is a smoke test that the printer ran to
    // completion and emitted the expected keys.
    EXPECT_NE(json.indexOf("TKSHD-J1"), -1);
    EXPECT_NE(json.indexOf("TKSHD-J2"), -1);
    EXPECT_NE(json.indexOf("\"code\""), -1);
    EXPECT_NE(json.indexOf("\"message\""), -1);
}
