/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AndroidSystem.hpp"

#if TK_ANDROID

#include "AndroidFile.h"

#include <toolkit/base/Hash.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/debug/Profiler.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/ProcessInitArgs.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/platforms/base/BaseSystem.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/types/Version.hpp>
#include <toolkit/utils/StringUtils.hpp>

#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <cerrno>
#include <climits>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <jni.h>
#include <pwd.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __has_include
#if __has_include(<sys/sysinfo.h>)
#include <sys/sysinfo.h>
#define TK_ANDROID_HAS_SYSINFO 1
#endif
#endif

#ifndef TK_ANDROID_HAS_SYSINFO
#define TK_ANDROID_HAS_SYSINFO 0
#endif

extern char** environ;

namespace toolkit
{
    constexpr i32 kBufferSize = 1024;

    struct AndroidSystemGlobals
    {
        android_app* app = nullptr;
        jobject activityObject = nullptr;
        JavaVM* javaVm = nullptr;

        CPUStats cpuStats{};
        TkUuid deviceId;
        TkString appPackageName;
        TkString deviceManufacturer;
        TkString deviceModel;
        TkString deviceBuildNumber;
        TkString systemVersionText;
        TkString systemLanguage;
        TkString cacheDir;
        TkString executablePath;
        TkString internalDataPath;
        TkString externalDataPath;
        TkString osName{"Android"};
        TkVersion osVersion{0, 0};
        TkString userName{"android"};
        TkString computerName{"Android"};
        TkString clipboardText;
        TkArray<byte> clipboardData;
        TkArray<TkString> clipboardFiles;
        u64 programSizeMem = 0;
        i32 clockSource = CLOCK_MONOTONIC;
        i32 processMutexFd = -1;
        i32 screenWidth = 0;
        i32 screenHeight = 0;
    };

    AndroidSystemGlobals s_global{};

    android_app* getAndroidApp()
    {
        return s_global.app;
    }

    TkString fromNullable(const char* text)
    {
        return text && *text ? TkString(text) : TkString::Empty;
    }

    TkString shellQuote(const TkStringView value)
    {
        TkString result("'");
        const TkString text(value);
        for (i32 i = 0; i < text.length(); ++i)
        {
            if (text[i] == '\'')
                result += "'\\''";
            else
                result += text[i];
        }
        result += "'";
        return result;
    }

    TkString readFirstLine(const char* path)
    {
        FILE* file = ::fopen(path, "r");
        if (!file)
            return {};

        char buffer[kBufferSize]{};
        char* result = ::fgets(buffer, sizeof(buffer), file);
        ::fclose(file);
        if (!result)
            return {};

        i32 length = TkStringUtils::length(buffer);
        while (length > 0 && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r' || buffer[length - 1] == ' '))
            buffer[--length] = 0;
        return TkString(buffer, length);
    }

    TkVersion parseVersionLike(const TkString& text)
    {
        i32 values[3] = {0, 0, 0};
        i32 valueIndex = 0;
        i32 current = -1;

        for (i32 i = 0; i < text.length() && valueIndex < 3; ++i)
        {
            const char c = text[i];
            if (TkStringUtils::isDigit(c))
            {
                if (current < 0)
                    current = 0;
                current = current * 10 + (c - '0');
            }
            else if (c == '.' && current >= 0)
            {
                values[valueIndex++] = current;
                current = -1;
            }
            else if (current >= 0)
            {
                break;
            }
        }

        if (current >= 0 && valueIndex < 3)
            values[valueIndex++] = current;
        if (valueIndex >= 3)
            return TkVersion(values[0], values[1], values[2]);
        if (valueIndex >= 2)
            return TkVersion(values[0], values[1]);
        if (valueIndex == 1)
            return TkVersion(values[0], 0);
        return TkVersion(0, 0);
    }

    u64 parseSizeString(const TkString& text)
    {
        if (text.isEmpty())
            return 0;

        u64 value = 0;
        i32 index = 0;
        while (index < text.length() && TkStringUtils::isDigit(text[index]))
        {
            value = value * 10 + static_cast<u64>(text[index] - '0');
            ++index;
        }

        while (index < text.length() && TkStringUtils::isWhitespace(text[index]))
            ++index;

        if (index < text.length())
        {
            switch (TkStringUtils::toUpper(text[index]))
            {
            case 'G':
                value *= 1024;
                [[fallthrough]];
            case 'M':
                value *= 1024;
                [[fallthrough]];
            case 'K':
                value *= 1024;
                break;
            default:
                break;
            }
        }
        return value;
    }

    void initCpuStats()
    {
        CPUStats& stats = s_global.cpuStats;
        stats = {};

        const long processors = ::sysconf(_SC_NPROCESSORS_ONLN);
        stats.logicalCoreCount = processors > 0 ? static_cast<u32>(processors) : 1;
        stats.coreCount = stats.logicalCoreCount;
        stats.packageCount = 1;

        for (i32 index = 0; index < 16; ++index)
        {
            char path[256];
            ::snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu0/cache/index%i/level", index);
            const TkString levelText = readFirstLine(path);
            if (levelText.isEmpty())
                continue;

            i32 level = 0;
            if (!TkStringUtils::fromString(levelText.get(), level))
                continue;

            ::snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu0/cache/index%i/size", index);
            const u64 size = parseSizeString(readFirstLine(path));
            if (level == 1)
                stats.l1Cache += static_cast<u32>(size);
            else if (level == 2)
                stats.l2Cache += static_cast<u32>(size);
            else if (level == 3)
                stats.l3Cache += static_cast<u32>(size);
        }

        const long pageSize = ::sysconf(_SC_PAGESIZE);
        stats.pageSize = pageSize > 0 ? static_cast<u32>(pageSize) : 4096;
        stats.speed = System::getClockFrequency();

#ifdef _SC_LEVEL1_DCACHE_LINESIZE
        const long cacheLineSize = ::sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        stats.cacheLineSize = cacheLineSize > 0 ? static_cast<u32>(cacheLineSize) : TK_CACHE_LINE_SIZE;
#else
        stats.cacheLineSize = TK_CACHE_LINE_SIZE;
#endif
    }

    void initLocale()
    {
        if (s_global.systemLanguage.hasChars())
        {
            s_global.systemLanguage.replace('_', '-');
            return;
        }

        ::setlocale(LC_ALL, "");
        const char* locale = ::setlocale(LC_CTYPE, nullptr);
        s_global.systemLanguage = locale && *locale ? locale : "en";
        const i32 encodingSeparator = s_global.systemLanguage.find('.');
        if (encodingSeparator != -1)
            s_global.systemLanguage = s_global.systemLanguage.left(encodingSeparator);
        s_global.systemLanguage.replace('_', '-');
        if (s_global.systemLanguage == "C" || s_global.systemLanguage == "POSIX")
            s_global.systemLanguage = "en";
    }

    bool readMacAddress(byte mac[6])
    {
        DIR* dir = ::opendir("/sys/class/net");
        if (!dir)
            return false;

        dirent* entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (TkStringUtils::compareStrings(entry->d_name, ".") == 0 ||
                TkStringUtils::compareStrings(entry->d_name, "..") == 0 ||
                TkStringUtils::compareStrings(entry->d_name, "lo") == 0)
            {
                continue;
            }

            char path[PATH_MAX];
            ::snprintf(path, sizeof(path), "/sys/class/net/%s/address", entry->d_name);
            const TkString address = readFirstLine(path);
            unsigned int parts[6]{};
            if (::sscanf(address.getText(), "%x:%x:%x:%x:%x:%x", &parts[0], &parts[1], &parts[2], &parts[3], &parts[4], &parts[5]) == 6)
            {
                for (i32 i = 0; i < 6; ++i)
                    mac[i] = static_cast<byte>(parts[i]);
                ::closedir(dir);
                return true;
            }
        }

        ::closedir(dir);
        return false;
    }

    void initDeviceId()
    {
        s_global.deviceId = TkUuid::Empty;

        u32 hash = HashFunc<TkString>::get(System::getComputerName());
        combineHash(hash, HashFunc<TkString>::get(System::getUserName()));
        s_global.deviceId.a = hash;

        byte mac[6]{};
        if (readMacAddress(mac))
        {
            hash = mac[0];
            for (byte value : mac)
                combineHash(hash, value);
            s_global.deviceId.b = hash;
        }
        else
        {
            s_global.deviceId.b = HashFunc<TkString>::get(s_global.appPackageName);
        }

        s_global.deviceId.c = static_cast<u32>(System::getMemoryStats().physicalMemSize);
        const CPUStats cpu = System::getCPUStats();
        s_global.deviceId.d = static_cast<u32>(cpu.speed) * cpu.logicalCoreCount * cpu.coreCount * cpu.cacheLineSize;
    }

    JNIEnv* attachJni(bool& detach)
    {
        detach = false;
        if (s_global.javaVm == nullptr)
            return nullptr;

        JNIEnv* env = nullptr;
        if (s_global.javaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK)
            return env;

        if (s_global.javaVm->AttachCurrentThread(&env, nullptr) == JNI_OK)
        {
            detach = true;
            return env;
        }
        return nullptr;
    }

    void detachJni(bool detach)
    {
        if (detach && s_global.javaVm)
            s_global.javaVm->DetachCurrentThread();
    }

    TkString jstringToTkString(JNIEnv* env, jstring value)
    {
        if (!env || !value)
            return TkString::Empty;

        const char* chars = env->GetStringUTFChars(value, nullptr);
        TkString result(chars ? chars : "");
        if (chars)
            env->ReleaseStringUTFChars(value, chars);
        return result;
    }

    bool clearJniException(JNIEnv* env)
    {
        if (!env || !env->ExceptionCheck())
            return false;
        env->ExceptionClear();
        return true;
    }

    bool callVoidStringMethod(const char* methodName, const TkStringView arg)
    {
        bool detach = false;
        JNIEnv* env = attachJni(detach);
        if (!env || !s_global.activityObject)
        {
            detachJni(detach);
            return false;
        }

        jclass clazz = env->GetObjectClass(s_global.activityObject);
        if (!clazz)
        {
            clearJniException(env);
            detachJni(detach);
            return false;
        }

        jmethodID method = env->GetMethodID(clazz, methodName, "(Ljava/lang/String;)V");
        if (!method)
        {
            clearJniException(env);
            env->DeleteLocalRef(clazz);
            detachJni(detach);
            return false;
        }

        const TkString text(arg);
        jstring jarg = env->NewStringUTF(text.getText());
        env->CallVoidMethod(s_global.activityObject, method, jarg);
        const bool failed = clearJniException(env);
        env->DeleteLocalRef(jarg);
        env->DeleteLocalRef(clazz);
        detachJni(detach);
        return !failed;
    }

    i32 callIntMethod(const char* methodName, i32 fallback)
    {
        bool detach = false;
        JNIEnv* env = attachJni(detach);
        if (!env || !s_global.activityObject)
        {
            detachJni(detach);
            return fallback;
        }

        jclass clazz = env->GetObjectClass(s_global.activityObject);
        if (!clazz)
        {
            clearJniException(env);
            detachJni(detach);
            return fallback;
        }

        jmethodID method = env->GetMethodID(clazz, methodName, "()I");
        if (!method)
        {
            clearJniException(env);
            env->DeleteLocalRef(clazz);
            detachJni(detach);
            return fallback;
        }

        const i32 result = static_cast<i32>(env->CallIntMethod(s_global.activityObject, method));
        if (clearJniException(env))
        {
            env->DeleteLocalRef(clazz);
            detachJni(detach);
            return fallback;
        }
        env->DeleteLocalRef(clazz);
        detachJni(detach);
        return result;
    }

    Pos getScreenSize()
    {
        return Pos(
            static_cast<float>(s_global.screenWidth > 0 ? s_global.screenWidth : 800),
            static_cast<float>(s_global.screenHeight > 0 ? s_global.screenHeight : 600));
    }

    bool System::init()
    {
        if (!BaseSystem::init())
            return false;

        timespec ts{};
        s_global.clockSource = ::clock_gettime(CLOCK_MONOTONIC, &ts) == 0 ? CLOCK_MONOTONIC : CLOCK_REALTIME;

        initCpuStats();
        initLocale();
        if (s_global.deviceModel.hasChars())
            s_global.computerName = s_global.deviceModel;
        s_global.osVersion = parseVersionLike(s_global.systemVersionText);
        s_global.programSizeMem = System::getProcessMemoryStats().usedPhysicalSize;
        initDeviceId();

        return true;
    }

    void System::logInfo()
    {
        LogInfo("Android %s %i-bit (%s)", s_global.systemVersionText.getText(), TK_PLATFORM_64BITS ? 64 : 32, s_global.osVersion.toString().getText());
        if (s_global.appPackageName.hasChars())
            LogInfo("App Package: %s", s_global.appPackageName.getText());
        if (s_global.deviceManufacturer.hasChars() || s_global.deviceModel.hasChars() || s_global.deviceBuildNumber.hasChars())
            LogInfo("Device: %s %s, %s", s_global.deviceManufacturer.getText(), s_global.deviceModel.getText(), s_global.deviceBuildNumber.getText());
        BaseSystem::logInfo();
    }

    void System::beforeRun()
    {
    }

    void System::update()
    {
    }

    void System::beforeQuit()
    {
    }

    void System::quit()
    {
        releaseProcessMutex();
        if (s_global.activityObject && s_global.javaVm)
        {
            bool detach = false;
            JNIEnv* env = attachJni(detach);
            if (env)
                env->DeleteGlobalRef(s_global.activityObject);
            detachJni(detach);
        }
        s_global.activityObject = nullptr;
    }

    TkString System::getOpSysName()
    {
        return s_global.osName;
    }

    TkVersion System::getOpSysVersion()
    {
        return s_global.osVersion;
    }

    CPUStats System::getCPUStats()
    {
        return s_global.cpuStats;
    }

    MemoryStats System::getMemoryStats()
    {
        MemoryStats result;
#if TK_ANDROID_HAS_SYSINFO
        sysinfo info{};
        if (::sysinfo(&info) == 0)
        {
            const u64 unit = static_cast<u64>(info.mem_unit);
            result.physicalMemSize = static_cast<u64>(info.totalram) * unit;
            const u64 freePhysical = (static_cast<u64>(info.freeram) + static_cast<u64>(info.bufferram)) * unit;
            result.usedPhysicalSize = result.physicalMemSize > freePhysical ? result.physicalMemSize - freePhysical : 0;
            result.virtualMemSize = result.physicalMemSize + static_cast<u64>(info.totalswap) * unit;
            const u64 freeVirtual = freePhysical + static_cast<u64>(info.freeswap) * unit;
            result.usedVirtualSize = result.virtualMemSize > freeVirtual ? result.virtualMemSize - freeVirtual : result.usedPhysicalSize;
        }
#endif
        if (result.physicalMemSize == 0)
        {
            const long pageSize = ::sysconf(_SC_PAGESIZE);
            const long totalPages = ::sysconf(_SC_PHYS_PAGES);
            const long availablePages = ::sysconf(_SC_AVPHYS_PAGES);
            if (pageSize > 0 && totalPages > 0 && availablePages >= 0)
            {
                result.physicalMemSize = static_cast<u64>(totalPages) * static_cast<u64>(pageSize);
                result.usedPhysicalSize = static_cast<u64>(totalPages - availablePages) * static_cast<u64>(pageSize);
                result.virtualMemSize = result.physicalMemSize;
                result.usedVirtualSize = result.usedPhysicalSize;
            }
        }
        result.programSizeMem = s_global.programSizeMem;
        return result;
    }

    ProcessMemoryStats System::getProcessMemoryStats()
    {
        ProcessMemoryStats result;
        FILE* file = ::fopen("/proc/self/statm", "r");
        if (file)
        {
            unsigned long long virtualPages = 0;
            unsigned long long residentPages = 0;
            if (::fscanf(file, "%llu %llu", &virtualPages, &residentPages) == 2)
            {
                const u64 pageSize = static_cast<u64>(::getpagesize());
                result.usedVirtualSize = virtualPages * pageSize;
                result.usedPhysicalSize = residentPages * pageSize;
            }
            ::fclose(file);
        }

        if (result.usedPhysicalSize == 0)
        {
            rusage usage{};
            ::getrusage(RUSAGE_SELF, &usage);
            result.usedPhysicalSize = static_cast<u64>(usage.ru_maxrss) * 1024;
            result.usedVirtualSize = result.usedPhysicalSize;
        }
        return result;
    }

    LibHandle System::loadLib(const char* filename)
    {
        TK_ASSERT(filename);
        PROFILE();
#if TK_TRACY_ENABLE
        ZoneText(filename, TkStringUtils::length(filename));
#endif
        void* handle = ::dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
        if (!handle)
            LogError("[AndroidSystem] Failed to load library %s: %s", filename, ::dlerror());
        return handle;
    }

    void System::freeLib(LibHandle handle)
    {
        if (handle)
            ::dlclose(handle);
    }

    void* System::getLibSymbol(LibHandle handle, const char* symbol)
    {
        return handle ? ::dlsym(handle, symbol) : nullptr;
    }

    i32 System::newProcess(ProcessInitArgs& args)
    {
        LogInfo("Command: %s %s", args.file.getText(), args.args.getText());
        const bool captureOutput = args.logOutput || args.saveOutput;
        i32 pipeHandles[2] = {-1, -1};
        if (captureOutput && ::pipe(pipeHandles) != 0)
        {
            LogWarn("[AndroidSystem] Failed to create pipe errno=%i", errno);
            pipeHandles[0] = pipeHandles[1] = -1;
        }

        const TkString commandLine = shellQuote(args.file) + " " + args.args;
        const pid_t pid = ::fork();
        if (pid < 0)
        {
            LogWarn("[AndroidSystem] Failed to fork process errno=%i", errno);
            if (pipeHandles[0] != -1)
                ::close(pipeHandles[0]);
            if (pipeHandles[1] != -1)
                ::close(pipeHandles[1]);
            return errno;
        }

        if (pid == 0)
        {
            for (auto it = args.envArgs.Begin(); it.isNotEnd(); ++it)
                ::setenv(it->key.getText(), it->value.getText(), 1);

            if (args.workDir.hasChars() && ::chdir(args.workDir.getText()) != 0)
                ::_exit(126);

            if (pipeHandles[1] != -1)
            {
                ::close(pipeHandles[0]);
                ::dup2(pipeHandles[1], STDOUT_FILENO);
                ::dup2(pipeHandles[1], STDERR_FILENO);
                ::close(pipeHandles[1]);
            }

            ::execl("/system/bin/sh", "sh", "-c", commandLine.getText(), static_cast<char*>(nullptr));
            ::_exit(127);
        }

        if (pipeHandles[1] != -1)
            ::close(pipeHandles[1]);

        i32 resultCode = 0;
        if (args.waitForQuit)
        {
            if (pipeHandles[0] != -1)
            {
                char buffer[1024];
                ssize_t readCount = 0;
                while ((readCount = ::read(pipeHandles[0], buffer, sizeof(buffer))) > 0)
                {
                    if (args.saveOutput)
                        args.output.add(buffer, static_cast<i32>(readCount));
                    if (args.logOutput)
                    {
                        TkString text(buffer, static_cast<i32>(readCount));
                        TkLogger::getShared()->log(LogLevel::kInfo, text.getText());
                    }
                }
                ::close(pipeHandles[0]);
            }

            i32 status = 0;
            if (::waitpid(pid, &status, 0) < 0)
                resultCode = errno;
            else if (WIFEXITED(status))
                resultCode = WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
                resultCode = 128 + WTERMSIG(status);
        }
        else if (pipeHandles[0] != -1)
        {
            ::close(pipeHandles[0]);
        }

        return resultCode;
    }

    double System::getTimeSeconds()
    {
        timespec ts{};
        ::clock_gettime(s_global.clockSource, &ts);
        return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 1000000000.0;
    }

    u64 System::getTimeCycles()
    {
        timespec ts{};
        ::clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return static_cast<u64>(ts.tv_sec) * 1000000ULL + static_cast<u64>(ts.tv_nsec) / 1000ULL;
    }

    u64 System::getClockFrequency()
    {
        return 1000000ULL;
    }

    void System::getSystemTime(TimeStats& time)
    {
        timeval tv{};
        ::gettimeofday(&tv, nullptr);

        tm local{};
        ::localtime_r(&tv.tv_sec, &local);
        time.year = local.tm_year + 1900;
        time.month = local.tm_mon + 1;
        time.dayOfWeek = local.tm_wday;
        time.day = local.tm_mday;
        time.hour = local.tm_hour;
        time.minute = local.tm_min;
        time.second = local.tm_sec;
        time.millisecond = static_cast<i32>(tv.tv_usec / 1000);
    }

    void System::getUTCTime(TimeStats& time)
    {
        timeval tv{};
        ::gettimeofday(&tv, nullptr);

        tm utc{};
        ::gmtime_r(&tv.tv_sec, &utc);
        time.year = utc.tm_year + 1900;
        time.month = utc.tm_mon + 1;
        time.dayOfWeek = utc.tm_wday;
        time.day = utc.tm_mday;
        time.hour = utc.tm_hour;
        time.minute = utc.tm_min;
        time.second = utc.tm_sec;
        time.millisecond = static_cast<i32>(tv.tv_usec / 1000);
    }

    TkString System::getExecPath()
    {
        if (s_global.executablePath.hasChars())
            return s_global.executablePath;

        char buffer[PATH_MAX]{};
        const ssize_t length = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (length <= 0)
            return TkString::Empty;
        return TkString(buffer, static_cast<i32>(length));
    }

    TkUuid System::getUniqueDeviceId()
    {
        return s_global.deviceId;
    }

    TkString System::getWorkDir()
    {
        char buffer[PATH_MAX]{};
        if (::getcwd(buffer, sizeof(buffer)) == nullptr)
            return TkString::Empty;
        return TkString(buffer);
    }

    TkString System::getTempDir()
    {
        if (s_global.cacheDir.hasChars())
            return s_global.cacheDir;
        const char* tmp = ::getenv("TMPDIR");
        return tmp && *tmp ? TkString(tmp) : TkString("/data/local/tmp");
    }

    bool System::setWorkDir(const TkString& path)
    {
        return ::chdir(path.getText()) == 0;
    }

    bool System::openUrl(const TkStringView url)
    {
        return callVoidStringMethod("openUrl", url);
    }

    void System::setHighDpiAware(bool)
    {
    }

    i32 System::getDpi()
    {
        const i32 dpi = callIntMethod("getDpi", 0);
        return dpi > 0 ? dpi : 96;
    }

    float System::getDpiScale()
    {
        return static_cast<float>(System::getDpi()) / 96.0f;
    }

    Pos System::getMousePos()
    {
        return Pos(0, 0);
    }

    void System::setMousePos(const Pos pos)
    {
        (void)pos;
    }

    Rect System::getMonitorBounds(const Pos)
    {
        const Pos size = getScreenSize();
        return Rect(0, 0, size.x, size.y);
    }

    Pos System::getDesktopSize()
    {
        return getScreenSize();
    }

    Rect System::getVDesktopBounds()
    {
        return getMonitorBounds(Pos(0, 0));
    }

    Pos System::GetVDesktopSize()
    {
        return getDesktopSize();
    }

    BatteryInfo System::getBatteryStats()
    {
        BatteryInfo result;
        result.percent = 1.0f;
        return result;
    }

    NetConType System::getNetConType()
    {
        const i32 type = callIntMethod("getNetworkConnectionType", static_cast<i32>(NetConType::kUnknown));
        return static_cast<NetConType>(type);
    }

    ScreenOrienType System::getScreenOrienType()
    {
        const i32 rotation = callIntMethod("getRotation", -1);
        switch (rotation)
        {
        case 0:
            return ScreenOrienType::kPortrait;
        case 1:
            return ScreenOrienType::kLandscapeLeft;
        case 2:
            return ScreenOrienType::kPortraitUpsideDown;
        case 3:
            return ScreenOrienType::kLandscapeRight;
        default:
            return s_global.screenWidth >= s_global.screenHeight ? ScreenOrienType::kLandscapeLeft : ScreenOrienType::kPortrait;
        }
    }

    TkString System::getLocale()
    {
        return s_global.systemLanguage;
    }

    TkString System::getComputerName()
    {
        return s_global.computerName;
    }

    TkString System::getUserName()
    {
        return s_global.userName;
    }

    void System::createUuid(TkUuid& result)
    {
        result = TkUuid::Empty;
        const i32 fd = ::open("/dev/urandom", O_RDONLY | O_CLOEXEC);
        if (fd != -1)
        {
            const ssize_t readBytes = ::read(fd, result.raw, sizeof(result.raw));
            ::close(fd);
            if (readBytes == static_cast<ssize_t>(sizeof(result.raw)))
                return;
        }

        result.a = static_cast<u32>(System::getTimeCycles());
        result.b = static_cast<u32>(System::getCurrentProcessId());
        result.c = HashFunc<TkString>::get(System::getExecPath());
        result.d = HashFunc<TkString>::get(System::getWorkDir());
    }

    void System::getEnvVars(TkHashMap<TkString, TkString>& result)
    {
        for (char** entry = environ; entry && *entry; ++entry)
        {
            char* separator = ::strchr(*entry, '=');
            if (!separator)
                result[TkString(*entry)] = TkString::Empty;
            else
                result[TkString(*entry, static_cast<i32>(separator - *entry))] = TkString(separator + 1);
        }
    }

    bool System::getEnvVar(const TkStringView name, TkString& value)
    {
        const TkString key(name);
        const char* env = ::getenv(key.getText());
        if (!env)
            return false;
        value = env;
        return true;
    }

    bool System::setEnvVar(const TkStringView name, const TkString& value)
    {
        const TkString key(name);
        return ::setenv(key.getText(), value.getText(), 1) == 0;
    }

    bool System::createProcessMutex(const char* name)
    {
        releaseProcessMutex();

        char path[PATH_MAX];
        ::snprintf(path, sizeof(path), "%s/%s.pid", System::getTempDir().getText(), name);
        s_global.processMutexFd = ::open(path, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
        if (s_global.processMutexFd == -1)
            return false;
        if (::flock(s_global.processMutexFd, LOCK_EX | LOCK_NB) != 0)
        {
            ::close(s_global.processMutexFd);
            s_global.processMutexFd = -1;
            return false;
        }
        return true;
    }

    void System::releaseProcessMutex()
    {
        if (s_global.processMutexFd != -1)
        {
            ::flock(s_global.processMutexFd, LOCK_UN);
            ::close(s_global.processMutexFd);
            s_global.processMutexFd = -1;
        }
    }

    void System::clearClipBoard()
    {
        s_global.clipboardText.clear();
        s_global.clipboardData.clear();
        s_global.clipboardFiles.clear();
    }

    void System::setClipBoard(const TkStringView text)
    {
        s_global.clipboardText = text;
        s_global.clipboardData.clear();
        s_global.clipboardFiles.clear();
    }

    void System::setClipBoard(const TkSpan<byte> data)
    {
        s_global.clipboardText.clear();
        s_global.clipboardFiles.clear();
        s_global.clipboardData.set(data.get(), data.length());
    }

    void System::setClipBoard(const TkArray<TkString>& files)
    {
        s_global.clipboardText.clear();
        s_global.clipboardData.clear();
        s_global.clipboardFiles = files;
    }

    TkString System::getClipBoardAsText()
    {
        return s_global.clipboardText;
    }

    TkArray<byte> System::getClipBoardAsData()
    {
        return s_global.clipboardData;
    }

    TkArray<TkString> System::getClipBoardAsFiles()
    {
        return s_global.clipboardFiles;
    }
}

extern "C" TK_API void TkAndroidSetApp(android_app* app)
{
    toolkit::s_global.app = app;
}

extern "C" TK_API void TkAndroidSetPlatformInfo(
    const char* appPackageName,
    const char* deviceManufacturer,
    const char* deviceModel,
    const char* deviceBuildNumber,
    const char* systemVersion,
    const char* systemLanguage,
    int32_t screenWidth,
    int32_t screenHeight,
    const char* cacheDir,
    const char* executablePath,
    const char* internalDataPath,
    const char* externalDataPath)
{
    toolkit::s_global.appPackageName = toolkit::fromNullable(appPackageName);
    toolkit::s_global.deviceManufacturer = toolkit::fromNullable(deviceManufacturer);
    toolkit::s_global.deviceModel = toolkit::fromNullable(deviceModel);
    toolkit::s_global.deviceBuildNumber = toolkit::fromNullable(deviceBuildNumber);
    toolkit::s_global.systemVersionText = toolkit::fromNullable(systemVersion);
    toolkit::s_global.systemLanguage = toolkit::fromNullable(systemLanguage);
    toolkit::s_global.cacheDir = toolkit::fromNullable(cacheDir);
    toolkit::s_global.executablePath = toolkit::fromNullable(executablePath);
    toolkit::s_global.internalDataPath = toolkit::fromNullable(internalDataPath);
    toolkit::s_global.externalDataPath = toolkit::fromNullable(externalDataPath);
    toolkit::s_global.systemLanguage.replace('_', '-');
    toolkit::s_global.osVersion = toolkit::parseVersionLike(toolkit::s_global.systemVersionText);
    toolkit::s_global.userName = toolkit::s_global.appPackageName.hasChars() ? toolkit::s_global.appPackageName : "android";
    toolkit::s_global.computerName = toolkit::s_global.deviceModel.hasChars() ? toolkit::s_global.deviceModel : "Android";
    toolkit::s_global.screenWidth = static_cast<toolkit::i32>(screenWidth);
    toolkit::s_global.screenHeight = static_cast<toolkit::i32>(screenHeight);
}

extern "C" JNIEXPORT void JNICALL Java_com_flaxengine_GameActivity_nativeSetPlatformInfo(
    JNIEnv* env,
    jobject thiz,
    jstring appPackageName,
    jstring deviceManufacturer,
    jstring deviceModel,
    jstring deviceBuildNumber,
    jstring systemVersion,
    jstring systemLanguage,
    jint screenWidth,
    jint screenHeight,
    jstring cacheDir,
    jstring executablePath)
{
    if (toolkit::s_global.activityObject != nullptr)
        env->DeleteGlobalRef(toolkit::s_global.activityObject);
    toolkit::s_global.activityObject = nullptr;
    if (thiz != nullptr)
        toolkit::s_global.activityObject = env->NewGlobalRef(thiz);
    env->GetJavaVM(&toolkit::s_global.javaVm);

    toolkit::s_global.appPackageName = toolkit::jstringToTkString(env, appPackageName);
    toolkit::s_global.deviceManufacturer = toolkit::jstringToTkString(env, deviceManufacturer);
    toolkit::s_global.deviceModel = toolkit::jstringToTkString(env, deviceModel);
    toolkit::s_global.deviceBuildNumber = toolkit::jstringToTkString(env, deviceBuildNumber);
    toolkit::s_global.systemVersionText = toolkit::jstringToTkString(env, systemVersion);
    toolkit::s_global.systemLanguage = toolkit::jstringToTkString(env, systemLanguage);
    toolkit::s_global.systemLanguage.replace('_', '-');
    toolkit::s_global.cacheDir = toolkit::jstringToTkString(env, cacheDir);
    toolkit::s_global.executablePath = toolkit::jstringToTkString(env, executablePath);
    toolkit::s_global.osVersion = toolkit::parseVersionLike(toolkit::s_global.systemVersionText);
    toolkit::s_global.userName = toolkit::s_global.appPackageName.hasChars() ? toolkit::s_global.appPackageName : "android";
    toolkit::s_global.computerName = toolkit::s_global.deviceModel.hasChars() ? toolkit::s_global.deviceModel : "Android";
    toolkit::s_global.screenWidth = static_cast<toolkit::i32>(screenWidth);
    toolkit::s_global.screenHeight = static_cast<toolkit::i32>(screenHeight);
}

#endif
