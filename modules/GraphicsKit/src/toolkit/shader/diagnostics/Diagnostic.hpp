/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/shader/source/SourceRange.hpp>

namespace toolkit
{
    enum class DiagnosticSeverity
    {
        Note,
        Warning,
        Error,
        Fatal,
    };

    const char *toString(DiagnosticSeverity severity);

    // Secondary message attached to a primary Diagnostic.
    // Use for "did you mean...", include stacks, or backend-reported follow-ups.
    struct DiagnosticNote
    {
        TkString message;
        SourceRange range;
    };

    struct Diagnostic
    {
        DiagnosticSeverity severity = DiagnosticSeverity::Error;

        // Stable, namespaced error code. Always non-empty for shipped diagnostics.
        // Convention: TKSHD<NNNN>, see ShaderDiagnosticCodes.hpp.
        TkString code;
        TkString message;

        // Primary source range. May be invalid if the diagnostic has no source
        // origin (e.g. "no backend selected" or a fatal CLI argument error).
        SourceRange range;

        TkArray<DiagnosticNote> notes;

        // Optional backend / stage / variant qualifiers. Used by the printer to
        // emit "[vulkan/pixel]" prefixes and by tools (LSP, CI) to filter.
        TkString backend;
        TkString stage;
        TkString variant;
    };

    //========================================================================
    // Shader compiler error codes.
    //========================================================================
    // Generic IO / setup
    inline constexpr const char *TKSHD_READ_SOURCE_FAILED          = "TKSHD0001";
    inline constexpr const char *TKSHD_WRITE_OUTPUT_FAILED         = "TKSHD0002";
    inline constexpr const char *TKSHD_UNSUPPORTED_BACKEND         = "TKSHD0003";
    inline constexpr const char *TKSHD_UNSUPPORTED_PROFILE         = "TKSHD0004";
    inline constexpr const char *TKSHD_UNSUPPORTED_STAGE           = "TKSHD0005";
    inline constexpr const char *TKSHD_INVALID_CLI                 = "TKSHD0006";
    inline constexpr const char *TKSHD_INVALID_OPTIONS             = "TKSHD0007";
    inline constexpr const char *TKSHD_PARSE_FAILED                = "TKSHD0008";
    inline constexpr const char *TKSHD_INCLUDE_NOT_FOUND           = "TKSHD0009";
    inline constexpr const char *TKSHD_NO_INPUT                    = "TKSHD0010";
    inline constexpr const char *TKSHD_FEATURE_NOT_IMPLEMENTED     = "TKSHD0011";

    // Backend compilation
    inline constexpr const char *TKSHD_BACKEND_COMPILE_FAILED      = "TKSHD0100";
    inline constexpr const char *TKSHD_DXC_COMPILE_FAILED          = "TKSHD0101";
    inline constexpr const char *TKSHD_D3DCOMPILER_COMPILE_FAILED  = "TKSHD0102";
    inline constexpr const char *TKSHD_GLSLANG_COMPILE_FAILED      = "TKSHD0103";
    inline constexpr const char *TKSHD_GLSL_COMPILE_FAILED         = "TKSHD0104";

    // Packaging / cache
    inline constexpr const char *TKSHD_PACKAGING_FAILED            = "TKSHD0200";
    inline constexpr const char *TKSHD_TKSD_INVALID_HEADER         = "TKSHD0201";
    inline constexpr const char *TKSHD_TKSD_UNSUPPORTED_VERSION    = "TKSHD0202";
    inline constexpr const char *TKSHD_TKSD_INVALID_CHUNK_TABLE    = "TKSHD0203";
    inline constexpr const char *TKSHD_TKSD_CHUNK_OUT_OF_RANGE     = "TKSHD0204";
    inline constexpr const char *TKSHD_TKSD_UNSUPPORTED_COMPRESSION = "TKSHD0205";
    inline constexpr const char *TKSHD_TKSD_MISSING_CHUNK          = "TKSHD0206";
    inline constexpr const char *TKSHD_TKSD_HASH_MISMATCH          = "TKSHD0207";

    // Phase 9 stage-bytecode diagnostics.
    inline constexpr const char *TKSHD_INVALID_STAGE_BYTECODE      = "TKSHD0208";
    inline constexpr const char *TKSHD_UNSUPPORTED_STAGE_KIND      = "TKSHD0209";
    inline constexpr const char *TKSHD_MISSING_STAGE_BYTECODE      = "TKSHD0210";
    inline constexpr const char *TKSHD_RUNTIME_BUILD_FAILED        = "TKSHD0211";
    inline constexpr const char *TKSHD_REFLECTION_EXTRACT_FAILED   = "TKSHD0212";

    // Compile graph / job cache (Phase 6+7)
    inline constexpr const char *TKSHD_COMPILE_GRAPH_ERROR         = "TKSHD0300";
    inline constexpr const char *TKSHD_INCLUDE_NOT_FOUND2          = "TKSHD0301";
    inline constexpr const char *TKSHD_INCLUDE_CYCLE               = "TKSHD0302";
    inline constexpr const char *TKSHD_JOB_CACHE_READ_FAILED       = "TKSHD0303";
    inline constexpr const char *TKSHD_JOB_CACHE_WRITE_FAILED      = "TKSHD0304";
    inline constexpr const char *TKSHD_JOB_HASH_FAILED             = "TKSHD0305";
    inline constexpr const char *TKSHD_SCHEDULER_FAILED            = "TKSHD0306";
}
