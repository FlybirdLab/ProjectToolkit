/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "UnixSystem.hpp"

#if TK_POSIX

#include <toolkit/base/Assert.hpp>
#include <toolkit/base/Hash.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/debug/Profiler.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/platforms/ProcessInitArgs.hpp>
#include <toolkit/threading/Task.hpp>
#include <toolkit/types/Version.hpp>
#include <toolkit/utils/StringUtils.hpp>

#if TK_USE_RPMALLOC
#include <rpmalloc/rpmalloc.h>
#endif

#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#if TK_LINUX || TK_ANDROID
#include <sys/sysinfo.h>
#endif

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#if !TK_APPLE
extern char **environ;
#endif

namespace toolkit
{
    namespace apple
    {
#if TK_APPLE
        char **getEnvironment();
        bool getExecutablePath(char *buffer, u32 bufferSize);
#endif
    }

    UnixSystemGlobals s_unixGlobal{};
    static TkString s_clipboardText;
    static TkArray<byte> s_clipboardData;
    static TkArray<TkString> s_clipboardFiles;

    static char **getEnvironment()
    {
#if TK_APPLE
        return apple::getEnvironment();
#else
        return ::environ;
#endif
    }

    static TkString getEnvVarOrDefault(const char *name, const char *defaultValue)
    {
        const char *value = getenv(name);
        return TkString(value && value[0] ? value : defaultValue);
    }

    static bool readCommandOutput(const char *command, TkString &output)
    {
        FILE *pipe = popen(command, "r");
        if (!pipe)
            return false;

        output.clear();
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), pipe))
            output += buffer;
        const i32 result = pclose(pipe);
        while (output.length() > 0 && (output[output.length() - 1] == '\n' || output[output.length() - 1] == '\r'))
            output.resize(output.length() - 1);
        return result == 0;
    }

    static bool writeCommandInput(const char *command, const char *data, i32 length)
    {
        FILE *pipe = popen(command, "w");
        if (!pipe)
            return false;
        if (length > 0)
            fwrite(data, 1, static_cast<size_t>(length), pipe);
        return pclose(pipe) == 0;
    }

#if TK_LINUX
    static bool readOsReleaseValue(const char *key, TkString &value)
    {
        FILE *file = fopen("/etc/os-release", "r");
        if (!file)
            return false;

        char line[512];
        const i32 keyLength = TkStringUtils::length(key);
        while (fgets(line, sizeof(line), file))
        {
            if (strncmp(line, key, keyLength) != 0 || line[keyLength] != '=')
                continue;

            char *start = line + keyLength + 1;
            while (*start == ' ' || *start == '\t')
                ++start;
            char *end = start + strlen(start);
            while (end > start && (end[-1] == '\n' || end[-1] == '\r' || end[-1] == ' ' || end[-1] == '\t'))
                --end;
            if (end > start && start[0] == '"' && end[-1] == '"')
            {
                ++start;
                --end;
            }
            value.set(start, static_cast<i32>(end - start));
            fclose(file);
            return true;
        }

        fclose(file);
        return false;
    }
#endif

    static void fillTime(TimeStats &time, const tm &value, i32 millisecond)
    {
        time.year = value.tm_year + 1900;
        time.month = value.tm_mon + 1;
        time.dayOfWeek = value.tm_wday;
        time.day = value.tm_mday;
        time.hour = value.tm_hour;
        time.minute = value.tm_min;
        time.second = value.tm_sec;
        time.millisecond = millisecond;
    }

    static TkString sanitizeMutexName(const char *name)
    {
        TkString result(name && name[0] ? name : "toolkit");
        for (i32 i = 0; i < result.length(); ++i)
        {
            if (!isalnum(static_cast<unsigned char>(result[i])) && result[i] != '-' && result[i] != '_')
                result[i] = '_';
        }
        return result;
    }

    static TkString buildCommand(const ProcessInitArgs &args)
    {
        TkString command = args.file;
        if (args.args.hasChars())
        {
            command += " ";
            command += args.args;
        }
        return command;
    }

    static void appendProcessOutput(ProcessInitArgs &args, const char *data, i32 length, LogLevel level)
    {
        if (length <= 0)
            return;

        if (args.saveOutput)
            args.output.add(data, length);

        if (args.logOutput)
        {
            TkString line(data, length);
            if (line.length() > 0 && line[line.length() - 1] == '\n')
                line.resize(line.length() - 1);
            TkLogger::getShared()->log(level, line.getText());
        }
    }

    void *System::reserveMemory(psize size)
    {
        void *result = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return result == MAP_FAILED ? nullptr : result;
    }

    void *System::allocatePages(u64 numPages, u64 pageSize)
    {
        const u64 numBytes = numPages * pageSize;
        void *result = mmap(nullptr, static_cast<psize>(numBytes), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return result == MAP_FAILED ? nullptr : result;
    }

    void System::commitMemory(void *ptr, psize size)
    {
        if (ptr && size > 0)
            mprotect(ptr, size, PROT_READ | PROT_WRITE);
    }

    void System::releaseMemory(void *ptr, psize size)
    {
        if (ptr && size > 0)
            munmap(ptr, size);
    }

#if TK_USE_RPMALLOC

    void *System::alloc(psize size)
    {
        return rpaligned_alloc(TK_SAFE_ALIGNMENT, size);
    }

    void *System::allocAligned(psize size, psize align)
    {
        return rpaligned_alloc(align, size);
    }

    void *System::realloc(void *ptr, psize size)
    {
        return rprealloc(ptr, size);
    }

    void *System::reallocAligned(void *ptr, psize size, psize newSize, psize align)
    {
        return rpaligned_realloc(ptr, align, size, newSize, 0);
    }

    void System::free(void *ptr)
    {
        if (ptr)
            rpfree(ptr);
    }

    static struct RpLifetime
    {
        RpLifetime()
        {
            rpmalloc_initialize(nullptr);
        }
        ~RpLifetime()
        {
            rpmalloc_finalize();
        }
    } s_lifeTime;

#else

    void *System::alloc(psize size)
    {
        return allocAligned(size, TK_SAFE_ALIGNMENT);
    }

    void *System::allocAligned(psize size, psize align)
    {
        void *result = nullptr;
        if (posix_memalign(&result, align, size) != 0)
            return nullptr;
        return result;
    }

    void *System::realloc(void *ptr, psize size)
    {
        return reallocAligned(ptr, size, size, TK_SAFE_ALIGNMENT);
    }

    void *System::reallocAligned(void *ptr, psize size, psize newSize, psize align)
    {
        void *result = allocAligned(newSize, align);
        if (result && ptr)
            memcpy(result, ptr, size < newSize ? size : newSize);
        free(ptr);
        return result;
    }

    void System::free(void *ptr)
    {
        ::free(ptr);
    }

#endif

    bool System::init()
    {
        if (!BaseSystem::init())
            return false;

        s_unixGlobal.clockFreq = 1000000000ull;
        s_unixGlobal.cycles2Seconds = 1.0 / static_cast<f64>(s_unixGlobal.clockFreq);
        s_unixGlobal.programSizeMem = System::getProcessMemoryStats().usedPhysicalSize;

        CPUStats cpu{};
        cpu.packageCount = 1;
        cpu.logicalCoreCount = static_cast<u32>(sysconf(_SC_NPROCESSORS_ONLN));
        cpu.coreCount = cpu.logicalCoreCount;
        cpu.pageSize = static_cast<u32>(sysconf(_SC_PAGESIZE));
        cpu.speed = s_unixGlobal.clockFreq;
        cpu.cacheLineSize = TK_CACHE_LINE_SIZE;
#ifdef _SC_LEVEL1_DCACHE_SIZE
        cpu.l1Cache = static_cast<u32>(sysconf(_SC_LEVEL1_DCACHE_SIZE));
#endif
#ifdef _SC_LEVEL2_CACHE_SIZE
        cpu.l2Cache = static_cast<u32>(sysconf(_SC_LEVEL2_CACHE_SIZE));
#endif
#ifdef _SC_LEVEL3_CACHE_SIZE
        cpu.l3Cache = static_cast<u32>(sysconf(_SC_LEVEL3_CACHE_SIZE));
#endif
        s_unixGlobal.cpuStats = cpu;

        s_unixGlobal.tempDir = getEnvVarOrDefault("TMPDIR", "/tmp");
        s_unixGlobal.deviceId = System::getUniqueDeviceId();
        return true;
    }

    void System::beforeRun()
    {
    }

    void System::update()
    {
    }

    void System::beforeQuit()
    {
        TkTask::disposePendingMainThreadTasks();
#if TK_DEBUG
        TkProfiler::dispose();
#endif
    }

    void System::quit()
    {
        releaseProcessMutex();
    }

    TkString UnixSystem::getLastErrMsg()
    {
        return TkString(strerror(errno));
    }

    TkString System::getOpSysName()
    {
#if TK_LINUX
        TkString name;
        if (readOsReleaseValue("NAME", name))
            return name;
#endif
        utsname info{};
        if (uname(&info) == 0)
            return TkString(info.sysname);
        return TkString(TK_OS_STR);
    }

    TkVersion System::getOpSysVersion()
    {
#if TK_LINUX
        TkString versionId;
        TkVersion parsed;
        if (readOsReleaseValue("VERSION_ID", versionId) && TkVersion::parse(versionId, parsed))
            return parsed;
#endif
        utsname info{};
        if (uname(&info) != 0)
            return TkVersion();

        i32 major = 0;
        i32 minor = 0;
        i32 build = 0;
        sscanf(info.release, "%d.%d.%d", &major, &minor, &build);
        return TkVersion(major, minor, build);
    }

    CPUStats System::getCPUStats()
    {
        return s_unixGlobal.cpuStats;
    }

    MemoryStats System::getMemoryStats()
    {
        MemoryStats result;
#if TK_LINUX || TK_ANDROID
        sysinfo info{};
        if (sysinfo(&info) == 0)
        {
            const u64 unit = info.mem_unit;
            result.physicalMemSize = info.totalram * unit;
            result.usedPhysicalSize = (info.totalram - info.freeram) * unit;
            result.virtualMemSize = (info.totalram + info.totalswap) * unit;
            result.usedVirtualSize = (info.totalram - info.freeram + info.totalswap - info.freeswap) * unit;
        }
#else
        const long pages = sysconf(_SC_PHYS_PAGES);
        const long pageSize = sysconf(_SC_PAGESIZE);
        if (pages > 0 && pageSize > 0)
            result.physicalMemSize = static_cast<u64>(pages) * static_cast<u64>(pageSize);
        result.virtualMemSize = result.physicalMemSize;
#endif
        result.programSizeMem = s_unixGlobal.programSizeMem;
        return result;
    }

    ProcessMemoryStats System::getProcessMemoryStats()
    {
        ProcessMemoryStats result;
#if TK_LINUX || TK_ANDROID
        FILE *file = fopen("/proc/self/statm", "r");
        if (file)
        {
            unsigned long virtualPages = 0;
            unsigned long residentPages = 0;
            if (fscanf(file, "%lu %lu", &virtualPages, &residentPages) == 2)
            {
                const u64 pageSize = static_cast<u64>(sysconf(_SC_PAGESIZE));
                result.usedVirtualSize = virtualPages * pageSize;
                result.usedPhysicalSize = residentPages * pageSize;
            }
            fclose(file);
        }
#else
        rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) == 0)
            result.usedPhysicalSize = static_cast<u64>(usage.ru_maxrss) * 1024ull;
#endif
        return result;
    }

    u64 System::getCurrentProcessId()
    {
        return static_cast<u64>(getpid());
    }

    LibHandle System::loadLib(const char *filename)
    {
        void *handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
        if (!handle)
        {
            const char *error = dlerror();
            LogWarn("Failed to load '%s' (Msg:%s)", filename ? filename : "", error ? error : "");
        }
        return handle;
    }

    void System::freeLib(LibHandle handle)
    {
        if (handle)
            dlclose(handle);
    }

    void *System::getLibSymbol(LibHandle handle, const char *symbol)
    {
        return handle ? dlsym(handle, symbol) : nullptr;
    }

    i32 System::newProcess(ProcessInitArgs &args)
    {
        LogInfo("Command: %s %s", args.file.getText(), args.args.getText());
        if (args.workDir.hasChars())
            LogInfo("Working directory: %s", args.workDir.getText());

        const bool captureOutput = args.logOutput || args.saveOutput;
        i32 pipeFd[2] = {-1, -1};
        if (captureOutput && pipe(pipeFd) != 0)
        {
            LogWarn("Cannot create process pipe. Error: %s", UnixSystem::getLastErrMsg().getText());
            return 1;
        }

        const TkString command = buildCommand(args);
        const pid_t pid = fork();
        if (pid < 0)
        {
            LogWarn("Cannot start process. Error: %s", UnixSystem::getLastErrMsg().getText());
            if (pipeFd[0] != -1)
            {
                close(pipeFd[0]);
                close(pipeFd[1]);
            }
            return 1;
        }

        if (pid == 0)
        {
            if (args.workDir.hasChars())
                chdir(args.workDir.getText());
            for (auto &entry : args.envArgs)
                setenv(entry.key.getText(), entry.value.getText(), 1);
            if (captureOutput)
            {
                close(pipeFd[0]);
                dup2(pipeFd[1], STDOUT_FILENO);
                dup2(pipeFd[1], STDERR_FILENO);
                close(pipeFd[1]);
            }
            execl("/bin/sh", "sh", "-c", command.getText(), static_cast<char *>(nullptr));
            _exit(127);
        }

        if (!args.waitForQuit && !captureOutput)
            return 0;

        if (captureOutput)
        {
            close(pipeFd[1]);
            char buffer[4096];
            ssize_t bytesRead = 0;
            while ((bytesRead = read(pipeFd[0], buffer, sizeof(buffer))) > 0)
                appendProcessOutput(args, buffer, static_cast<i32>(bytesRead), LogLevel::kInfo);
            close(pipeFd[0]);
        }

        i32 status = 0;
        if (waitpid(pid, &status, 0) < 0)
            return 1;
        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        if (WIFSIGNALED(status))
            return 128 + WTERMSIG(status);
        return 1;
    }

    double System::getTimeSeconds()
    {
        timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 1000000000.0;
    }

    u64 System::getTimeCycles()
    {
        timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return static_cast<u64>(ts.tv_sec) * 1000000000ull + static_cast<u64>(ts.tv_nsec);
    }

    u64 System::getClockFrequency()
    {
        return s_unixGlobal.clockFreq;
    }

    void System::getSystemTime(TimeStats &time)
    {
        timeval tv{};
        gettimeofday(&tv, nullptr);
        tm localTime{};
        localtime_r(&tv.tv_sec, &localTime);
        fillTime(time, localTime, static_cast<i32>(tv.tv_usec / 1000));
    }

    void System::getUTCTime(TimeStats &time)
    {
        timeval tv{};
        gettimeofday(&tv, nullptr);
        tm utcTime{};
        gmtime_r(&tv.tv_sec, &utcTime);
        fillTime(time, utcTime, static_cast<i32>(tv.tv_usec / 1000));
    }

    TkString System::getExecPath()
    {
        char buffer[PATH_MAX];
#if TK_APPLE
        if (apple::getExecutablePath(buffer, sizeof(buffer)))
            return TkString(buffer);
#else
        const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (length > 0)
        {
            buffer[length] = '\0';
            return TkString(buffer);
        }
#endif
        return TkString::Empty;
    }

    TkUuid System::getUniqueDeviceId()
    {
        if (!s_unixGlobal.deviceId.isValid())
        {
            u32 hash = HashFunc<TkString>::get(System::getComputerName());
            combineHash(hash, HashFunc<TkString>::get(System::getUserName()));
            combineHash(hash, static_cast<u32>(System::getMemoryStats().physicalMemSize));
            const CPUStats cpu = System::getCPUStats();
            s_unixGlobal.deviceId = TkUuid(hash, cpu.coreCount, cpu.logicalCoreCount, cpu.cacheLineSize);
        }
        return s_unixGlobal.deviceId;
    }

    TkString System::getWorkDir()
    {
        char buffer[PATH_MAX];
        if (getcwd(buffer, sizeof(buffer)))
            return TkString(buffer);
        return TkString::Empty;
    }

    TkString System::getTempDir()
    {
        return s_unixGlobal.tempDir.hasChars() ? s_unixGlobal.tempDir : getEnvVarOrDefault("TMPDIR", "/tmp");
    }

    bool System::setWorkDir(const TkString &path)
    {
        return chdir(path.getText()) == 0;
    }

    bool System::openUrl(const TkStringView url)
    {
        const TkString command =
#if TK_APPLE
            TkString("open \"") + TkString(url) + "\"";
#else
            TkString("xdg-open \"") + TkString(url) + "\"";
#endif
        return system(command.getText()) == 0;
    }

    void System::setHighDpiAware(bool)
    {
    }

    i32 System::getDpi()
    {
        return 96;
    }

    float System::getDpiScale()
    {
        return 1.0f;
    }

    Pos System::getMousePos()
    {
        return Pos(0, 0);
    }

    void System::setMousePos(const Pos)
    {
    }

    Rect System::getMonitorBounds(const Pos)
    {
        return Rect(0, 0, 0, 0);
    }

    Pos System::getDesktopSize()
    {
        return Pos(0, 0);
    }

    Rect System::getVDesktopBounds()
    {
        return Rect(0, 0, 0, 0);
    }

    Pos System::GetVDesktopSize()
    {
        return Pos(0, 0);
    }

    BatteryInfo System::getBatteryStats()
    {
        return BatteryInfo();
    }

    NetConType System::getNetConType()
    {
#if TK_LINUX
        DIR *dir = opendir("/sys/class/net");
        if (!dir)
            return NetConType::kUnknown;

        NetConType result = NetConType::kNone;
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (entry->d_name[0] == '.')
                continue;
            TkString operstatePath = TkString("/sys/class/net") / entry->d_name / "operstate";
            FILE *file = fopen(operstatePath.getText(), "r");
            if (!file)
                continue;
            char state[32] {};
            const bool isUp = fgets(state, sizeof(state), file) && strncmp(state, "up", 2) == 0;
            fclose(file);
            if (!isUp)
                continue;

            if (TkStringUtils::compareStrings(entry->d_name, "lo") == 0)
                result = NetConType::kNone;
            else if (strncmp(entry->d_name, "wl", 2) == 0 || strncmp(entry->d_name, "wifi", 4) == 0)
            {
                closedir(dir);
                return NetConType::kWiFi;
            }
            else if (strncmp(entry->d_name, "ww", 2) == 0 || strncmp(entry->d_name, "ppp", 3) == 0)
            {
                closedir(dir);
                return NetConType::kCell;
            }
            else
                result = NetConType::kEthernet;
        }
        closedir(dir);
        return result;
#else
        return NetConType::kUnknown;
#endif
    }

    ScreenOrienType System::getScreenOrienType()
    {
        return ScreenOrienType::kUnknown;
    }

    TkString System::getLocale()
    {
        return getEnvVarOrDefault("LANG", "C");
    }

    TkString System::getComputerName()
    {
        char buffer[256];
        if (gethostname(buffer, sizeof(buffer)) == 0)
        {
            buffer[sizeof(buffer) - 1] = '\0';
            return TkString(buffer);
        }
        return TkString::Empty;
    }

    TkString System::getUserName()
    {
        passwd *pwd = getpwuid(getuid());
        if (pwd && pwd->pw_name)
            return TkString(pwd->pw_name);
        return getEnvVarOrDefault("USER", "");
    }

    void System::createUuid(TkUuid &result)
    {
        i32 fd = open("/dev/urandom", O_RDONLY);
        if (fd >= 0)
        {
            const ssize_t readBytes = read(fd, result.raw, sizeof(result.raw));
            close(fd);
            if (readBytes == static_cast<ssize_t>(sizeof(result.raw)))
                return;
        }

        const u64 time = System::getTimeCycles();
        result.a = static_cast<u32>(time);
        result.b = static_cast<u32>(time >> 32);
        result.c = static_cast<u32>(getpid());
        result.d = HashFunc<TkString>::get(System::getComputerName());
    }

    void System::getEnvVars(TkHashMap<TkString, TkString> &result)
    {
        char **env = getEnvironment();
        if (!env)
            return;

        for (; *env; ++env)
        {
            char *separator = strchr(*env, '=');
            if (!separator || separator == *env)
                continue;
            const i32 keyLength = static_cast<i32>(separator - *env);
            result[TkString(*env, keyLength)] = TkString(separator + 1);
        }
    }

    bool System::getEnvVar(const TkStringView name, TkString &value)
    {
        const TkString key(name);
        const char *env = getenv(key.getText());
        if (!env)
            return false;
        value = env;
        return true;
    }

    bool System::setEnvVar(const TkStringView name, const TkString &value)
    {
        const TkString key(name);
        return setenv(key.getText(), value.getText(), 1) == 0;
    }

    bool System::createProcessMutex(const char *name)
    {
        if (s_unixGlobal.processMutex != -1)
            return true;

        const TkString mutexPath = System::getTempDir() + "/" + sanitizeMutexName(name) + ".lock";
        const i32 fd = open(mutexPath.getText(), O_CREAT | O_RDWR, 0600);
        if (fd < 0)
            return false;
        if (flock(fd, LOCK_EX | LOCK_NB) != 0)
        {
            close(fd);
            return false;
        }
        s_unixGlobal.processMutex = fd;
        return true;
    }

    void System::releaseProcessMutex()
    {
        if (s_unixGlobal.processMutex != -1)
        {
            flock(s_unixGlobal.processMutex, LOCK_UN);
            close(s_unixGlobal.processMutex);
            s_unixGlobal.processMutex = -1;
        }
    }

    void System::clearClipBoard()
    {
        s_clipboardText.clear();
        s_clipboardData.clear();
        s_clipboardFiles.clear();
#if TK_LINUX
        writeCommandInput("wl-copy 2>/dev/null || xclip -selection clipboard 2>/dev/null || xsel --clipboard --input 2>/dev/null", "", 0);
#endif
    }

    void System::setClipBoard(const TkStringView text)
    {
        s_clipboardText = TkString(text);
        s_clipboardData.clear();
        s_clipboardFiles.clear();
#if TK_LINUX
        writeCommandInput("wl-copy 2>/dev/null || xclip -selection clipboard 2>/dev/null || xsel --clipboard --input 2>/dev/null", s_clipboardText.getText(), s_clipboardText.length());
#endif
    }

    void System::setClipBoard(const TkSpan<byte> data)
    {
        s_clipboardText.clear();
        s_clipboardFiles.clear();
        s_clipboardData.resize(data.length(), false);
        if (data.length() > 0)
            TkMemory::memoryCopy(s_clipboardData.get(), data.get(), data.length());
    }

    void System::setClipBoard(const TkArray<TkString> &files)
    {
        s_clipboardText.clear();
        s_clipboardData.clear();
        s_clipboardFiles = files;
    }

    TkString System::getClipBoardAsText()
    {
#if TK_LINUX
        TkString text;
        if (readCommandOutput("wl-paste --no-newline 2>/dev/null || xclip -selection clipboard -o 2>/dev/null || xsel --clipboard --output 2>/dev/null", text))
            return text;
#endif
        return s_clipboardText;
    }

    TkArray<byte> System::getClipBoardAsData()
    {
        return s_clipboardData;
    }

    TkArray<TkString> System::getClipBoardAsFiles()
    {
        return s_clipboardFiles;
    }

    void System::logInfo()
    {
        utsname info{};
        if (uname(&info) == 0)
            LogInfo("%s %s %s", info.sysname, info.release, info.machine);
    }
}

#endif
