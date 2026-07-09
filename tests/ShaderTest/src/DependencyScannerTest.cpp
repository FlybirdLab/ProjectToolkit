/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * DependencyScanner tests -- recursive scan, cycle detection, missing
 * includes, hash stability. Each test materializes its own fixtures under
 * the build-tree scratch dir so encoding / line-ending quirks of source-
 * tree files can't influence the result.
 */

#include "common.hpp"

using namespace shader_test;

namespace
{
    // Helper -- write a fixture file and return its normalized absolute path.
    TkString putFixture(const TkString &dir, const char *name, const char *content)
    {
        TkString path = dir / TkString(name);
        TkPath::normalize(path);
        const bool ok = writeTextFile(TkStringView(path), TkStringView(content));
        if (!ok)
        {
            ADD_FAILURE() << "writeTextFile failed for '" << path.get() << "'";
        }
        // Critical: verify the bytes actually landed. EXPECT_TRUE above
        // would have caught a `false` return from writeTextFile, but on
        // some platforms saveToFile reports success even when the file
        // couldn't be created (parent dir missing etc) -- a follow-up
        // existence probe makes the failure mode obvious.
        if (!TkFileSystem::isFileExist(path))
        {
            ADD_FAILURE() << "fixture file not on disk after putFixture: '"
                          << path.get() << "'";
        }
        return path;
    }

    bool depPathContains(const TkArray<ShaderDependency> &deps, const char *needle)
    {
        for (i32 i = 0; i < deps.count(); i++)
        {
            if (deps[i].resolvedPath.indexOf(needle) != -1)
                return true;
        }
        return false;
    }
}

TEST(DependencyScannerTest, NoIncludesYieldsEmptyDependencyListAndStableHash)
{
    SourceManager sm;
    IncludeResolver resolver;
    DependencyScanner scanner;

    const TkString primary = TkString("// nothing to include here\nfloat x = 1;");
    const TkString anchor = tmpPath("dep_noinc/dummy.hlsl");
    DependencyScanResult r1 = scanner.scan(sm, resolver, TkStringView(anchor), TkStringView(primary));
    EXPECT_TRUE(r1.success);
    EXPECT_EQ(r1.dependencies.count(), 0);
    EXPECT_EQ(r1.missingIncludes.count(), 0);
    EXPECT_EQ(r1.includeCycles.count(), 0);

    DependencyScanResult r2 = scanner.scan(sm, resolver, TkStringView(anchor), TkStringView(primary));
    EXPECT_TRUE(r1.dependencyHash == r2.dependencyHash)
        << "hash of an empty dependency closure must be stable across calls";
}

TEST(DependencyScannerTest, RecursiveScanWalksTransitiveIncludes)
{
    const TkString dir = uniqueScratchDir("dep_chain");
    const TkString rootPath = putFixture(dir, "root.hlsl",
                                         "#include \"mid.hlsl\"\n");
    const TkString midPath = putFixture(dir, "mid.hlsl",
                                        "#include \"leaf.hlsl\"\n#define MID 1\n");
    const TkString leafPath = putFixture(dir, "leaf.hlsl",
                                         "#define LEAF 1\n");

    // Sanity probe -- before exercising the scanner, confirm that the
    // resolver can locate the sibling includes from rootPath. This is a
    // direct mirror of step 1 inside DependencyScanner::scan and isolates
    // "fixtures missing on disk" from "scanner walked wrong path".
    IncludeResolver probe;
    probe.addIncludeDirectory(TkStringView(dir));
    {
        TkString hit;
        ASSERT_TRUE(probe.resolve(TkStringView("mid.hlsl"),
                                  TkStringView(rootPath), hit))
            << "resolver failed before scan: rootPath='" << rootPath.get()
            << "' dir='" << dir.get() << "'";
        ASSERT_TRUE(TkFileSystem::isFileExist(hit))
            << "resolver returned non-existent path: '" << hit.get() << "'";
    }

    SourceManager sm;
    IncludeResolver resolver;
    resolver.addIncludeDirectory(TkStringView(dir));
    DependencyScanner scanner;

    const TkString rootContent = readTextFile(TkStringView(rootPath));
    ASSERT_FALSE(rootContent.isEmpty());

    DependencyScanResult r = scanner.scan(sm, resolver,
                                          TkStringView(rootPath),
                                          TkStringView(rootContent));
    EXPECT_TRUE(r.success)
        << "missingIncludes=" << r.missingIncludes.count()
        << " (first missing: "
        << (r.missingIncludes.isEmpty() ? "<none>" : r.missingIncludes[0].get())
        << ")";
    EXPECT_EQ(r.missingIncludes.count(), 0);
    EXPECT_EQ(r.includeCycles.count(), 0);
    EXPECT_GE(r.dependencies.count(), 2)
        << "expected to discover mid + leaf, got " << r.dependencies.count();
    EXPECT_TRUE(depPathContains(r.dependencies, "mid.hlsl"));
    EXPECT_TRUE(depPathContains(r.dependencies, "leaf.hlsl"));
}

TEST(DependencyScannerTest, MissingIncludeIsReportedNotSilent)
{
    SourceManager sm;
    IncludeResolver resolver;
    DependencyScanner scanner;

    const TkString primary = TkString("#include \"this_file_does_not_exist.hlsl\"\nfloat x;");
    const TkString anchor = tmpPath("dep_missing/anchor.hlsl");
    DependencyScanResult r = scanner.scan(sm, resolver,
                                          TkStringView(anchor),
                                          TkStringView(primary));
    EXPECT_FALSE(r.success);
    EXPECT_GE(r.missingIncludes.count(), 1);
}

TEST(DependencyScannerTest, CycleIsDetectedAndDoesNotInfiniteLoop)
{
    const TkString dir = uniqueScratchDir("dep_cycle");
    const TkString aPath = putFixture(dir, "a.hlsl",
                                      "#include \"b.hlsl\"\nfloat ident_a(float v) { return v; }\n");
    putFixture(dir, "b.hlsl",
               "#include \"a.hlsl\"\nfloat ident_b(float v) { return v; }\n");

    SourceManager sm;
    IncludeResolver resolver;
    resolver.addIncludeDirectory(TkStringView(dir));
    DependencyScanner scanner;

    const TkString aContent = readTextFile(TkStringView(aPath));
    ASSERT_FALSE(aContent.isEmpty());

    DependencyScanResult r = scanner.scan(sm, resolver,
                                          TkStringView(aPath),
                                          TkStringView(aContent));
    EXPECT_GE(r.includeCycles.count(), 1)
        << "cycle a->b->a should be detected";
    // The scanner must terminate -- if it doesn't, the test runner kills us.
    EXPECT_LE(r.dependencies.count(), 8);
}

TEST(DependencyScannerTest, FlatScanFindsDirectIncludesOnly)
{
    const TkString dir = uniqueScratchDir("dep_flat");
    putFixture(dir, "header_a.hlsl", "// a\n");
    putFixture(dir, "header_b.hlsl", "// b\n");

    IncludeResolver resolver;
    resolver.addIncludeDirectory(TkStringView(dir));
    DependencyScanner scanner;

    TkArray<TkString> resolved;
    TkArray<TkString> missing;
    const TkString src = TkString("#include \"header_a.hlsl\"\n#include \"header_b.hlsl\"\n");
    const bool ok = scanner.scanFlat(TkStringView(src),
                                     TkStringView(dir / TkString("anchor.hlsl")),
                                     resolver, resolved, missing);
    EXPECT_TRUE(ok);
    EXPECT_EQ(missing.count(), 0);
    EXPECT_GE(resolved.count(), 2);
}

TEST(DependencyScannerTest, EditingTheIncludeContentChangesDependencyHash)
{
    // Two scans against the same primary text but the included file's bytes
    // change between scans -- the dependency hash must move accordingly so
    // the cache invalidates on header edits.
    const TkString dir = uniqueScratchDir("dep_invalidate");
    const TkString rootPath = putFixture(dir, "root.hlsl",
                                         "#include \"header.hlsl\"\n");

    putFixture(dir, "header.hlsl", "// v1\n");

    SourceManager sm;
    IncludeResolver resolver;
    resolver.addIncludeDirectory(TkStringView(dir));
    DependencyScanner scanner;

    const TkString rootContent = readTextFile(TkStringView(rootPath));
    DependencyScanResult r1 = scanner.scan(sm, resolver,
                                           TkStringView(rootPath),
                                           TkStringView(rootContent));
    EXPECT_TRUE(r1.success);

    // Mutate the header on disk.
    putFixture(dir, "header.hlsl", "// v2 -- changed\n#define HEADER_V 2\n");

    // Fresh SourceManager so the scanner re-reads from disk rather than
    // serving the cached v1 content.
    SourceManager sm2;
    DependencyScanResult r2 = scanner.scan(sm2, resolver,
                                           TkStringView(rootPath),
                                           TkStringView(rootContent));
    EXPECT_TRUE(r2.success);

    EXPECT_FALSE(r1.dependencyHash == r2.dependencyHash)
        << "header content changed -- dep hash must move";
}

TEST(DependencyScannerTest, IdenticalScansProduceIdenticalDependencyHash)
{
    // Two scans against the same primary text and unchanged include closure
    // must produce the same dependency hash (this is what makes the
    // ShaderJobCache key stable across runs).
    const TkString dir = uniqueScratchDir("dep_stable");
    const TkString rootPath = putFixture(dir, "root.hlsl",
                                         "#include \"a.hlsl\"\n#include \"b.hlsl\"\n");
    putFixture(dir, "a.hlsl", "// alpha\n");
    putFixture(dir, "b.hlsl", "// beta\n");

    IncludeResolver resolver;
    resolver.addIncludeDirectory(TkStringView(dir));
    DependencyScanner scanner;

    const TkString rootContent = readTextFile(TkStringView(rootPath));

    SourceManager sm1;
    DependencyScanResult r1 = scanner.scan(sm1, resolver,
                                           TkStringView(rootPath),
                                           TkStringView(rootContent));
    SourceManager sm2;
    DependencyScanResult r2 = scanner.scan(sm2, resolver,
                                           TkStringView(rootPath),
                                           TkStringView(rootContent));
    EXPECT_TRUE(r1.success);
    EXPECT_TRUE(r2.success);
    EXPECT_TRUE(r1.dependencyHash == r2.dependencyHash);
    EXPECT_EQ(r1.dependencies.count(), r2.dependencies.count());
}
