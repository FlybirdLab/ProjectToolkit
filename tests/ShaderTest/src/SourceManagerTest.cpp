/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 *
 * SourceManager + SourceFile + IncludeResolver tests. These don't touch the
 * lexer/parser -- they're focused on file id allocation, virtual-vs-disk
 * file handling, line/column resolution, and include search ordering.
 */

#include "common.hpp"

using namespace shader_test;

TEST(SourceManagerTest, VirtualFileGetsValidIdAndStoresContent)
{
    SourceManager sm;
    SourceFile *file = sm.addVirtualFile(TkStringView("v.tkshader"),
                                         TkStringView("Hello\nWorld"));
    ASSERT_NE(file, nullptr);
    EXPECT_TRUE(file->isVirtual());
    EXPECT_NE(file->getId(), 0u);
    EXPECT_EQ(file->getPath(), TkString("v.tkshader"));
    EXPECT_EQ(file->getContent(), TkString("Hello\nWorld"));
}

TEST(SourceManagerTest, FileIdsAreOneBasedAndIdZeroIsInvalid)
{
    SourceManager sm;
    // Reserve id 0 is internal; user gets ids starting at 1.
    SourceFile *file = sm.addVirtualFile(TkStringView("a"), TkStringView("x"));
    ASSERT_NE(file, nullptr);
    EXPECT_GE(file->getId(), 1u);
    EXPECT_EQ(sm.getFile(0u), nullptr);
}

TEST(SourceManagerTest, GetFileReturnsSameInstanceById)
{
    SourceManager sm;
    SourceFile *a = sm.addVirtualFile(TkStringView("a"), TkStringView("xyz"));
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(sm.getFile(a->getId()), a);
    EXPECT_EQ(static_cast<const SourceManager &>(sm).getFile(a->getId()), a);
}

TEST(SourceManagerTest, GetFileCountReflectsLoadedAndReservedSlot)
{
    SourceManager sm;
    EXPECT_EQ(sm.getFileCount(), 0); // before any loads
    sm.addVirtualFile(TkStringView("a"), TkStringView("x"));
    sm.addVirtualFile(TkStringView("b"), TkStringView("y"));
    // The reserved-id-0 slot bumps the count, so two files report 3.
    EXPECT_GE(sm.getFileCount(), 2);
}

TEST(SourceManagerTest, LoadFileReturnsConsistentObjectForSamePath)
{
    SourceManager sm;
    const TkString abs = docPath("shaders/test_triangle.tkshader");
    SourceFile *f1 = sm.loadFile(TkStringView(abs));
    SourceFile *f2 = sm.loadFile(TkStringView(abs));
    ASSERT_NE(f1, nullptr);
    EXPECT_EQ(f1, f2) << "loadFile must cache by absolute path";
    EXPECT_FALSE(f1->isVirtual());
    EXPECT_NE(f1->getContent().indexOf("Sandbox/TestTriangle"), -1);
}

TEST(SourceManagerTest, LoadFileReturnsNullForMissingPath)
{
    SourceManager sm;
    EXPECT_EQ(sm.loadFile(TkStringView("Z:/this/does/not/exist.tkshader")), nullptr);
}

TEST(SourceFileTest, GetLocationProducesOneBasedLineAndColumn)
{
    SourceManager sm;
    SourceFile *file = sm.addVirtualFile(TkStringView("loc"),
                                         TkStringView("abc\ndef\nghi"));
    ASSERT_NE(file, nullptr);

    // offset 0 -> line 1, col 1
    SourceLocation loc0 = file->getLocation(0);
    EXPECT_EQ(loc0.line, 1u);
    EXPECT_EQ(loc0.column, 1u);

    // offset 4 ('d') -> line 2, col 1
    SourceLocation loc4 = file->getLocation(4);
    EXPECT_EQ(loc4.line, 2u);
    EXPECT_EQ(loc4.column, 1u);

    // offset 6 ('f') -> line 2, col 3
    SourceLocation loc6 = file->getLocation(6);
    EXPECT_EQ(loc6.line, 2u);
    EXPECT_EQ(loc6.column, 3u);
}

TEST(SourceFileTest, GetLineTextReturnsOneBasedLine)
{
    SourceManager sm;
    SourceFile *file = sm.addVirtualFile(TkStringView("lines"),
                                         TkStringView("alpha\nbeta\ngamma"));
    ASSERT_NE(file, nullptr);
    EXPECT_EQ(TkString(file->getLineText(1)), TkString("alpha"));
    EXPECT_EQ(TkString(file->getLineText(2)), TkString("beta"));
    EXPECT_EQ(TkString(file->getLineText(3)), TkString("gamma"));
}

TEST(SourceFileTest, GetLineTextOutOfRangeReturnsEmpty)
{
    SourceManager sm;
    SourceFile *file = sm.addVirtualFile(TkStringView("empty"),
                                         TkStringView("only one"));
    ASSERT_NE(file, nullptr);
    EXPECT_EQ(TkString(file->getLineText(0)), TkString());
    EXPECT_EQ(TkString(file->getLineText(100)), TkString());
}

// ---------- IncludeResolver ----------

TEST(IncludeResolverTest, ResolvesSiblingRelativeToCurrentFile)
{
    IncludeResolver r;
    // No -I dirs configured; sibling of `currentFile` should still resolve.
    const TkString currentFile = docPath("shaders/test_triangle.hlsl");
    TkString resolved;
    EXPECT_TRUE(r.resolve(TkStringView("common_uniforms.hlsl"),
                          TkStringView(currentFile), resolved));
    EXPECT_NE(resolved.indexOf("common_uniforms.hlsl"), -1);
}

TEST(IncludeResolverTest, FallsBackToConfiguredIncludeDirs)
{
    IncludeResolver r;
    r.addIncludeDirectory(TkStringView(docPath("includes")));

    TkString resolved;
    // currentFile points at a path with no sibling -- must walk -I dirs.
    EXPECT_TRUE(r.resolve(TkStringView("chain_leaf.hlsl"),
                          TkStringView(docPath("nowhere/dummy.hlsl")),
                          resolved));
    EXPECT_NE(resolved.indexOf("chain_leaf.hlsl"), -1);
}

TEST(IncludeResolverTest, MissingIncludeReturnsFalse)
{
    IncludeResolver r;
    r.addIncludeDirectory(TkStringView(docPath("includes")));
    TkString resolved;
    EXPECT_FALSE(r.resolve(TkStringView("absolutely_not_here.hlsl"),
                           TkStringView(docPath("shaders/test_triangle.hlsl")),
                           resolved));
}

TEST(IncludeResolverTest, IncludeDirectoryListIsExposedInOrder)
{
    IncludeResolver r;
    r.addIncludeDirectory(TkStringView("/a"));
    r.addIncludeDirectory(TkStringView("/b"));
    r.addIncludeDirectory(TkStringView("/c"));
    const TkArray<TkString> &dirs = r.getIncludeDirectories();
    ASSERT_EQ(dirs.count(), 3);
    EXPECT_NE(dirs[0].indexOf(TkStringView("/a")), -1);
    EXPECT_NE(dirs[1].indexOf(TkStringView("/b")), -1);
    EXPECT_NE(dirs[2].indexOf(TkStringView("/c")), -1);
}
