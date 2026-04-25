/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

TEST(DialogMappingTest, Win32DialogResultsMapExplicitly)
{
    EXPECT_EQ(Dialog::fromPlatformDialogResult(IDOK), DialogResult::kOK);
    EXPECT_EQ(Dialog::fromPlatformDialogResult(IDCANCEL), DialogResult::kCancel);
    EXPECT_EQ(Dialog::fromPlatformDialogResult(IDYES), DialogResult::kYes);
    EXPECT_EQ(Dialog::fromPlatformDialogResult(IDNO), DialogResult::kNo);
    EXPECT_EQ(Dialog::fromPlatformDialogResult(IDRETRY), DialogResult::kRetry);
    EXPECT_EQ(Dialog::fromPlatformDialogResult(IDABORT), DialogResult::kAbort);
    EXPECT_EQ(Dialog::fromPlatformDialogResult(IDIGNORE), DialogResult::kIgnore);
    EXPECT_EQ(Dialog::fromPlatformDialogResult(IDCONTINUE), DialogResult::kNone);
    EXPECT_EQ(Dialog::fromPlatformDialogResult(-1), DialogResult::kNone);
}

TEST(DialogMappingTest, EmptyFileExplorerPathFailsSafely)
{
    EXPECT_FALSE(Dialog::showFileExplorer(""));
}
