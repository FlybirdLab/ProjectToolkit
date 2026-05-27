/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/shader/cache/ShaderHash.hpp>

namespace toolkit
{
    struct ShaderCompileJob;
    class SourceManager;
    class IncludeResolver;
    class DiagnosticSink;

    // One resolved dependency surfaced by the scanner. The pair
    // (resolvedPath, contentHash) is what feeds into the dependency hash
    // used by ShaderJobCache.
    struct ShaderDependency
    {
        TkString resolvedPath;
        ShaderHash128 contentHash;
    };

    // Aggregate output of a recursive include scan. The caller is expected
    // to forward `missingIncludes` / `includeCycles` into the diagnostic sink
    // and treat a non-empty list of either as a compile failure. A successful
    // scan with no deps simply leaves `dependencies` empty and
    // `dependencyHash` set to the hash of an empty payload (still stable
    // across runs).
    struct DependencyScanResult
    {
        bool success = true;

        TkArray<ShaderDependency> dependencies;
        TkArray<TkString> missingIncludes;
        TkArray<TkString> includeCycles;

        ShaderHash128 dependencyHash;
    };

    // Recursive #include scanner.
    //
    // v1 behaviour:
    //   - Only line-start `#include "foo"` / `#include <foo>` is recognised.
    //   - Conditional includes (#if guarding an #include) are over-
    //     approximated -- the directive still gets scanned even if the
    //     surrounding condition would compile away.
    //   - Cycles are reported per-file: the first cycle path that includes
    //     a previously visited file is recorded as a string, scanning of
    //     that branch stops, but other branches still run so the user gets
    //     a complete missing-include list in the same pass.
    class DependencyScanner
    {
    public:
        // Backwards-compatible single-level scan kept so existing callers
        // (DependencyScanner unit tests, ad-hoc tooling) keep working. New
        // code should prefer `scan(...)` below.
        bool scanFlat(
            TkStringView sourceText,
            TkStringView anchorFile,
            const IncludeResolver &resolver,
            TkArray<TkString> &outResolvedPaths,
            TkArray<TkString> &outMissing) const;

        // Recursive entry point. `primaryPath` is used purely as the seed
        // anchor for relative include resolution (so includes in the same
        // dir as the primary source resolve before user -I dirs); pass the
        // virtual file name for embedded sources. `primaryContent` is the
        // text to scan -- the scanner intentionally re-reads each include
        // from disk so that the dependency hash always reflects the
        // on-disk bytes the next compile will see.
        DependencyScanResult scan(
            SourceManager &sourceManager,
            IncludeResolver &includeResolver,
            TkStringView primaryPath,
            TkStringView primaryContent) const;
    };
}
