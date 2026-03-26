/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/FileSystem.hpp>

#if TK_WINDOWS

#include "toolkit/threading/Sync.hpp"
#include "toolkit/base/Function.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/collection/StaticArray.hpp"

#define NOMINMAX
#include <Windows.h>

namespace toolkit
{

    class Win32FileSystem : public TkFileSystem
    {
    public:
        // Move a file to the recycle bin with possible undo instead of removing it permanently.
        static bool moveToTrashBin(const TkStringView path);
    };

    struct Win32FileWatcher
    {
    public:
        static TkMutex s_mtx;
        static TkStaticArray<Win32FileWatcher *, 16> s_watchers;
        static TkThread *s_thread;
        static ::HANDLE s_ioHandle;
        static bool s_threadActive;

        static i32 run();

    public:
        TkString path;
        bool withSubDirs;
        TkAction<const TkString &, FileAction> event;

        ::OVERLAPPED overlapped;
        ::HANDLE directoryHandle;
        TkThread *thread = nullptr;
        TkMutex mtx;
        volatile i64 activeCallbacks = 0;
        bool stopNow = false;
        static const i32 bufferSize = 32 * 1024;
        alignas(::DWORD) byte Buffer[bufferSize];

        void readDirectoryChanges();
        void notificationCompletion();
    };
}

#endif
