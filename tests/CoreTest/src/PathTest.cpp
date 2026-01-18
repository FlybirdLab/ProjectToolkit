/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/types/Path.hpp"
#include "toolkit/types/Span.hpp"

// ------------------------------------------------------------
// Normalize
// ------------------------------------------------------------
TEST(PathTest, Normalize)
{
    char buf[128];

    TkPath::normalize("a/b/c", TkSpan(buf));
    EXPECT_TRUE(TkStringUtils::equalStrings(buf, "a/b/c"));

    TkPath::normalize("a//b///c", TkSpan(buf));
    EXPECT_TRUE(TkStringUtils::equalStrings(buf, "a/b/c"));

    TkPath::normalize("a\\b\\c", TkSpan(buf));
    EXPECT_TRUE(TkStringUtils::equalStrings(buf, "a/b/c"));

    TkPath::normalize("./a/b", TkSpan(buf));
    EXPECT_TRUE(TkStringUtils::equalStrings(buf, "a/b"));

    TkPath::normalize(".\\a\\b", TkSpan(buf));
    EXPECT_TRUE(TkStringUtils::equalStrings(buf, "a/b"));

    TkPath::normalize("", TkSpan(buf));
    EXPECT_EQ(buf[0], '\0');

    TkPath::normalize("/", TkSpan(buf));
    EXPECT_TRUE(TkStringUtils::equalStrings(buf, "/"));

#if TK_WINDOWS
    TkPath::normalize("C:\\Users\\Test", TkSpan(buf));
    EXPECT_TRUE(TkStringUtils::equalStrings(buf, "C:/Users/Test"));
#endif
}

// ------------------------------------------------------------
// Normalize + buffer boundary
// ------------------------------------------------------------
TEST(PathTest, Normalize_Buffer)
{
    const char* src = "C:/Users/Test";

    // 普通
    {
        char buf[32];
        char* end = TkPath::normalize(src, TkSpan(buf));
        ASSERT_GE(end, buf);
        EXPECT_EQ(buf[end - buf], '\0');
    }

    // 刚好卡边界
    {
        char buf[15];
        char* end = TkPath::normalize(src, TkSpan(buf));

        ASSERT_GE(end, buf);
        ASSERT_LT(end, buf + sizeof(buf));
        EXPECT_EQ(*end, '\0');
        EXPECT_STREQ(buf, "C:/Users/Test");
    }

    // 强制截断
    {
        char buf[8];
        char* end = TkPath::normalize(src, TkSpan(buf));
        EXPECT_EQ(end, buf + 7);
        EXPECT_EQ(buf[7], '\0');
    }
}

// ------------------------------------------------------------
// Normalize in-place
// ------------------------------------------------------------
TEST(PathTest, Normalize_InPlace)
{
    {
        char path[] = "a//b///c";
        TkPath::normalize(path);
        EXPECT_TRUE(TkStringUtils::equalStrings(path, "a/b/c"));
    }

    {
        char path[] = "./a/b";
        TkPath::normalize(path);
        EXPECT_TRUE(TkStringUtils::equalStrings(path, "a/b"));
    }

    {
        char path[] = "a\\b\\c";
        TkPath::normalize(path);
        EXPECT_TRUE(TkStringUtils::equalStrings(path, "a/b/c"));
    }

    {
        char path[] = "";
        TkPath::normalize(path);
        EXPECT_EQ(path[0], '\0');
    }
}

// ------------------------------------------------------------
// getDir
// ------------------------------------------------------------
TEST(PathTest, GetDir)
{
    EXPECT_EQ(TkPath::getDir("a/b/c"), "a/b");
    EXPECT_EQ(TkPath::getDir("a/b"), "a");
    EXPECT_EQ(TkPath::getDir("a"), "");
    EXPECT_EQ(TkPath::getDir("/a"), "/");
    EXPECT_EQ(TkPath::getDir("/"), "");
    EXPECT_EQ(TkPath::getDir("C:/a/b"), "C:/a");
}

// ------------------------------------------------------------
// getFileName
// ------------------------------------------------------------
TEST(PathTest, GetFileName)
{
    EXPECT_EQ(TkPath::getFileName("a/b/c.txt"), "c.txt");
    EXPECT_EQ(TkPath::getFileName("c.txt"), "c.txt");
    EXPECT_EQ(TkPath::getFileName("/a/b"), "b");
    EXPECT_EQ(TkPath::getFileName("C:/a/b"), "b");
}

// ------------------------------------------------------------
// getBaseName
// ------------------------------------------------------------
TEST(PathTest, GetBaseName)
{
    EXPECT_EQ(TkPath::getBaseName("a/b/c.txt"), "c");
    EXPECT_EQ(TkPath::getBaseName("a/b/archive.tar.gz"), "archive.tar");
    EXPECT_EQ(TkPath::getBaseName("file"), "file");
}

// ------------------------------------------------------------
// getExtension
// ------------------------------------------------------------
TEST(PathTest, GetExtension)
{
    EXPECT_EQ(TkPath::getExtension("a.txt"), "txt");
    EXPECT_EQ(TkPath::getExtension("a.b.c"), "c");
    EXPECT_EQ(TkPath::getExtension("dir/file.txt"), "txt");
    EXPECT_EQ(TkPath::getExtension("dir.with.dot/file"), "");
    EXPECT_EQ(TkPath::getExtension("file."), "");
    EXPECT_EQ(TkPath::getExtension(".gitignore"), "gitignore");
}

// ------------------------------------------------------------
// isRelative
// ------------------------------------------------------------
TEST(PathTest, IsRelative)
{
    EXPECT_TRUE(TkPath::isRelative("a/b"));
    EXPECT_TRUE(TkPath::isRelative("./a/b"));
    EXPECT_FALSE(TkPath::isRelative("/a/b"));

#if TK_WINDOWS
    EXPECT_FALSE(TkPath::isRelative("C:/a/b"));
    EXPECT_FALSE(TkPath::isRelative("\\\\server\\share"));
#endif
}

// ------------------------------------------------------------
// isSame
// ------------------------------------------------------------
TEST(PathTest, IsSame)
{
    EXPECT_TRUE(TkPath::isSame("a/b", "a/b"));
    EXPECT_TRUE(TkPath::isSame("a/b/", "a/b"));
    EXPECT_TRUE(TkPath::isSame("./a/b", "a/b"));
    EXPECT_FALSE(TkPath::isSame("a/b", "a/c"));

#if TK_WINDOWS
    EXPECT_TRUE(TkPath::isSame("C:/a/b", "C:\\a\\b"));
#endif
}

// ------------------------------------------------------------
// replaceExtension
// ------------------------------------------------------------
TEST(PathTest, ReplaceExtension)
{
    {
        char path[64] = "a/b/c.txt";
        bool ok = TkPath::replaceExtension(path, "bin");
        EXPECT_TRUE(ok);
        EXPECT_TRUE(TkStringUtils::equalStrings(path, "a/b/c.bin"));
    }

    {
        char path[64] = "a/b/c";
        bool ok = TkPath::replaceExtension(path, "bin");
        EXPECT_FALSE(ok);
    }

    {
        char path[64] = "a/b/c.";
        bool ok = TkPath::replaceExtension(path, "bin");
        EXPECT_TRUE(ok);
        EXPECT_TRUE(TkStringUtils::equalStrings(path, "a/b/c.bin"));
    }
}

// ------------------------------------------------------------
// pathWithoutRelative
// ------------------------------------------------------------
TEST(PathTest, PathWithoutRelative)
{
    TkString path;

    path = "a/b/../c";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "a/c");

    path = "a/./b";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "a/b");

    path = "./a/b";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "a/b");

    path = "../a/b";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "../a/b");

    path = "../../a";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "../../a");

    path = "a/../../b";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "../b");

    path = "a//b///c";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "a/b/c");

    path = "/a/b/../c";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "/a/c");

    path = "/../a";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "/a");

    path = "/../../a";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "/a");

    path = "/..";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "/");

#if TK_WINDOWS
    path = "C:/a/b/../c";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "C:/a/c");

    path = "C:/a/./b";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "C:/a/b");

    path = "C:/../a";
    TkPath::pathWithoutRelative(path);
    EXPECT_EQ(path, "C:/a");
#endif
}

// ------------------------------------------------------------
// No crash / minimal buffer
// ------------------------------------------------------------
TEST(PathTest, NoCrash)
{
    char buf[4];
    TkPath::normalize("////", TkSpan(buf));
    EXPECT_EQ(buf[0], '/');
}