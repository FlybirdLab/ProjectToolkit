/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include "toolkit/platforms/System.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/types/Span.hpp"
#include "toolkit/utils/Convertor.hpp"
#include <codecvt>

struct ClipboardBackup
{
    TkString text;
    TkArray<byte> data;
    TkArray<TkString> files;

    ClipboardBackup()
    {
        text = System::getClipBoardAsText();
        data = System::getClipBoardAsData();
        files = System::getClipBoardAsFiles();
    }

    ~ClipboardBackup()
    {
        System::clearClipBoard();
        if (!files.isEmpty())
            System::setClipBoard(files);
        else if (!data.isEmpty())
        {
            System::setClipBoard(data.asSpan());
        }
        else if (!text.isEmpty())
            System::setClipBoard(text);
    }
};

TEST(Clipboard, Text_ASCII)
{
    ClipboardBackup backup;

    System::setClipBoard("Hello Clipboard");
    TkString out = System::getClipBoardAsText();

    EXPECT_EQ(out, "Hello Clipboard");
}

TEST(Clipboard, Text_UTF8)
{
    ClipboardBackup backup;

    TkString text = "中文测试 🚀 Clipboard";
    System::setClipBoard(text);

    TkString out = System::getClipBoardAsText();
    LogInfo("in:%s out:%s", text, out);
    EXPECT_EQ(out, text);
}

TEST(Clipboard, Text_Empty)
{
    ClipboardBackup backup;

    System::setClipBoard("");
    TkString out = System::getClipBoardAsText();

    EXPECT_TRUE(out.isEmpty());
}

TEST(Clipboard, RawData_Basic)
{
    ClipboardBackup backup;

    byte buf[] = {0x00, 0xFF, 0x12, 0x34};
    TkSpan<byte> span{buf, sizeof(buf)};

    System::setClipBoard(span);
    TkArray<byte> out = System::getClipBoardAsData();

    ASSERT_EQ(out.count(), 4);
    EXPECT_EQ(std::memcmp(out.get(), buf, 4), 0);
}

TEST(Clipboard, RawData_Large)
{
    ClipboardBackup backup;

    TkArray<byte> data;
    data.resize(1024 * 1024);
    for (i32 i = 0; i < data.count(); ++i)
        data[i] = static_cast<byte>(i);

    TkSpan<byte> o(data.get(), data.count());
    System::setClipBoard(o);
    auto out = System::getClipBoardAsData();

    EXPECT_EQ(out.count(), data.count());
    EXPECT_EQ(std::memcmp(out.get(), data.get(), data.count()), 0);
}

TEST(Clipboard, Files_Basic)
{
    ClipboardBackup backup;

    TkArray<TkString> files;
    files.add("C:\\Windows\\notepad.exe");
    files.add("C:\\Windows\\System32");

    System::setClipBoard(files);

    auto out = System::getClipBoardAsFiles();
    ASSERT_EQ(out.count(), 2);

    EXPECT_EQ(out[0], files[0]);
    EXPECT_EQ(out[1], files[1]);
}

TEST(Clipboard, Files_UTF16)
{
    ClipboardBackup backup;

    TkArray<TkString> files;
    files.add("C:\\测试的文件夹\\测试的文件.txt");

    StringAsUTF16 rr {files[0].get()};
    auto* ptr = rr.get();

    System::setClipBoard(files);
    auto out = System::getClipBoardAsFiles();

    ASSERT_EQ(out.count(), 1);
    EXPECT_EQ(out[0], files[0]);
}

TEST(Clipboard, Clear)
{
    ClipboardBackup backup;

    System::setClipBoard("Test");
    System::clearClipBoard();

    EXPECT_TRUE(System::getClipBoardAsText().isEmpty());
}
