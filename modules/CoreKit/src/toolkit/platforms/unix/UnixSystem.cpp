/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/System.hpp>

#if TK_POSIX

#include <toolkit/base/Hash.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/debug/Profiler.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/platforms/ProcessInitArgs.hpp>
#include <toolkit/platforms/base/BaseSystem.hpp>
#include <toolkit/threading/Task.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/types/Version.hpp>

#if TK_USE_RPMALLOC
#include <rpmalloc/rpmalloc.h>
#endif

#include <cerrno>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dlfcn.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#if TK_LINUX
#include <sys/sysinfo.h>
#endif

extern char **environ;

namespace toolkit
{
#ifndef MAP_ANON
#define MAP_ANON MAP_ANONYMOUS
#endif

    void *System::reserveMemory(psize size)
    {
        void *ptr = ::mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
        return ptr == MAP_FAILED ? nullptr : ptr;
    }

    void *System::allocatePages(u64 numPages, u64 pageSize)
    {
        const psize size = static_cast<psize>(numPages * pageSize);
        void *ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        return ptr == MAP_FAILED ? nullptr : ptr;
    }

    void System::commitMemory(void *ptr, psize size)
    {
        ::mprotect(ptr, size, PROT_READ | PROT_WRITE);
    }

    void System::releaseMemory(void *ptr, psize size)
    {
        if (ptr)
            ::munmap(ptr, size);
    }

#if TK_USE_RPMALLOC
    void *System::alloc(psize size)
    {
        return rpaligned_alloc(16, size);
    }

    void *System::allocAligned(psize size, psize alignment)
    {
        return rpaligned_alloc(alignment, size);
    }

    void System::free(void *ptr)
    {
        if (ptr)
            rpfree(ptr);
    }

    void *System::realloc(void *ptr, psize size)
    {
        return rprealloc(ptr, size);
    }

    void *System::reallocAligned(void *ptr, psize size, psize newSize, psize align)
    {
        return rpaligned_realloc(ptr, align, size, newSize, 0);
    }

    static struct RpLifetime
    {
        RpLifetime() { rpmalloc_initialize(nullptr); }
        ~RpLifetime() { rpmalloc_finalize(); }
    } s_rpLifetime;
#else
    void *System::alloc(psize size)
    {
        return allocAligned(size, 16);
    }

    void *System::allocAligned(psize size, psize alignment)
    {
        void *ptr = nullptr;
        if (::posix_memalign(&ptr, alignment, size) != 0)
            return nullptr;
        return ptr;
    }

    void System::free(void *ptr)
    {
        ::free(ptr);
    }

    void *System::realloc(void *ptr, psize size)
    {
        return ::realloc(ptr, size);
    }

    void *System::reallocAligned(void *ptr, psize size, psize newSize, psize align)
    {
        void *newPtr = allocAligned(newSize, align);
        if (newPtr && ptr)
            ::memcpy(newPtr, ptr, size < newSize ? size : newSize);
        free(ptr);
        return newPtr;
    }
#endif

    u64 System::getCurrentProcessId()
    {
        return static_cast<u64>(::getpid());
    }
}

#endif
