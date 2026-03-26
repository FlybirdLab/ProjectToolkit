/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#include <toolkit/collection/Array.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/platforms/Network.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/types/Version.hpp>
#include <toolkit/types/Path.hpp>

namespace
{
    inline TkString makePlatformTempTestDir()
    {
        return System::getTempDir() / TkString::format("ToolkitPlatform_%s", TkUuid::create().toString(UuidFormatType::kN).get());
    }

    struct TempDirScope
    {
        TkString path;

        TempDirScope()
            : path(makePlatformTempTestDir())
        {
            EXPECT_TRUE(TkFileSystem::makeDir(path));
        }

        ~TempDirScope()
        {
            TkFileSystem::deleteDir(path, true);
        }
    };

    inline bool writePlatformTestFile(const TkString &path, const TkString &text)
    {
        OpenFileParams params{path, FileMode::kCreateAlways, FileAccess::kReadWrite, FileShare::kReadWrite};
        FileHandle handle = TkFileSystem::openFile(params);
        if (handle == nullptr)
            return false;

        const bool result = TkFileSystem::writeText(handle, text, Encoding::kUTF8);
        TkFileSystem::closeFile(handle);
        return result;
    }
}

class PlatformTestBase : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        System::init();
        System::beforeRun();
    }

    static void TearDownTestSuite()
    {
        System::beforeQuit();
        System::quit();
    }
};
