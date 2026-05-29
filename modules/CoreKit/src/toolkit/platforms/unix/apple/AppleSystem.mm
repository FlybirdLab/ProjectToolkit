/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AppleSystem.hpp"

#if TK_APPLE

#include "AppleUtils.h"

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

#include <cerrno>
#include <climits>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dlfcn.h>
#include <fcntl.h>
#include <pwd.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <dispatch/dispatch.h>
#include <Foundation/Foundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach-o/dyld.h>
#include <uuid/uuid.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#if TK_MACOS
#include "macos/MacSystem.hpp"
#include <AppKit/AppKit.h>
#include <IOKit/IOKitLib.h>
#endif

#if TK_IOS
#include <UIKit/UIKit.h>
#include <os/proc.h>
#endif

extern char** environ;

namespace toolkit
{
    constexpr i32 kBufferSize = 1024;

    struct AppleSystemGlobals
    {
        CPUStats cpuStats{};
        TkUuid deviceId;
        TkString userLocale{"en"};
        TkString computerName{"Apple"};
        TkString userName{"user"};
        TkString homeDir{"/"};
        TkString tempDir{"/tmp"};
        TkVersion osVersion{0, 0};
        TkString osName;
        TkString clipboardText;
        TkArray<byte> clipboardData;
        TkArray<TkString> clipboardFiles;
        u64 programSizeMem = 0;
        double secondsPerCycle = 1e-9;
        float screenScale = 1.0f;
        i32 processMutexFd = -1;
        NSAutoreleasePool* autoreleasePool = nullptr;
        i32 autoreleasePoolInterval = 0;
    };

    AppleSystemGlobals s_global{};

    void setAppleScreenScale(float scale)
    {
        s_global.screenScale = scale > 0.0f ? scale : 1.0f;
    }

    float getAppleScreenScale()
    {
        return s_global.screenScale;
    }

    namespace AppleUtils
    {
        TkString toString(CFStringRef value)
        {
            if (!value)
                return TkString::Empty;

            const CFIndex length = CFStringGetLength(value);
            const CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
            TkArray<char> buffer(static_cast<i32>(maxSize));
            buffer.resize(static_cast<i32>(maxSize));
            if (!CFStringGetCString(value, buffer.get(), maxSize, kCFStringEncodingUTF8))
                return TkString::Empty;
            return TkString(buffer.get());
        }

        CFStringRef toCFString(TkStringView value)
        {
            return CFStringCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(value.getText()), value.length(), kCFStringEncodingUTF8, false);
        }

        NSString* toNSString(TkStringView value)
        {
            return value.hasChars() ? [[[NSString alloc] initWithBytes:value.getText() length:value.length() encoding:NSUTF8StringEncoding] autorelease] : @"";
        }

        NSString* toNSString(const char* value)
        {
            return value && *value ? [NSString stringWithUTF8String:value] : @"";
        }

        NSArray* parseArguments(NSString* arguments)
        {
            NSMutableArray* result = [NSMutableArray array];
            NSMutableString* current = [NSMutableString string];
            BOOL quoted = NO;
            for (NSUInteger i = 0; i < [arguments length]; ++i)
            {
                const unichar c = [arguments characterAtIndex:i];
                if (c == '"')
                {
                    quoted = !quoted;
                }
                else if (c == ' ' && !quoted)
                {
                    if ([current length] > 0)
                    {
                        [result addObject:[[current copy] autorelease]];
                        [current setString:@""];
                    }
                }
                else
                {
                    [current appendFormat:@"%C", c];
                }
            }
            if ([current length] > 0)
                [result addObject:[[current copy] autorelease]];
            return result;
        }
    }

    TkVersion versionFromNSVersion(NSOperatingSystemVersion version)
    {
        return TkVersion(static_cast<i32>(version.majorVersion), static_cast<i32>(version.minorVersion), static_cast<i32>(version.patchVersion));
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

    template<typename T>
    bool sysctlValue(const char* name, T& value)
    {
        size_t size = sizeof(T);
        return ::sysctlbyname(name, &value, &size, nullptr, 0) == 0;
    }

    void initCpuStats()
    {
        CPUStats& stats = s_global.cpuStats;
        stats = {};

        i32 value32 = 0;
        u64 value64 = 0;
        if (!sysctlValue("hw.packages", value32) || value32 <= 0)
            value32 = 1;
        stats.packageCount = static_cast<u32>(value32);
        if (!sysctlValue("hw.physicalcpu", value32) || value32 <= 0)
            value32 = 1;
        stats.coreCount = static_cast<u32>(value32);
        if (!sysctlValue("hw.logicalcpu", value32) || value32 <= 0)
            value32 = static_cast<i32>(stats.coreCount);
        stats.logicalCoreCount = static_cast<u32>(value32);
        if (sysctlValue("hw.l1dcachesize", value32))
            stats.l1Cache = static_cast<u32>(value32);
        if (sysctlValue("hw.l2cachesize", value32))
            stats.l2Cache = static_cast<u32>(value32);
        if (sysctlValue("hw.l3cachesize", value32))
            stats.l3Cache = static_cast<u32>(value32);
        if (!sysctlValue("hw.pagesize", value32) || value32 <= 0)
            value32 = static_cast<i32>(::getpagesize());
        stats.pageSize = static_cast<u32>(value32);
        if (!sysctlValue("hw.cpufrequency_max", value64) || value64 == 0)
            value64 = System::getClockFrequency();
        stats.speed = value64;
        if (!sysctlValue("hw.cachelinesize", value32) || value32 <= 0)
            value32 = TK_CACHE_LINE_SIZE;
        stats.cacheLineSize = static_cast<u32>(value32);
    }

    void initLocale()
    {
        @autoreleasepool
        {
            NSLocale* locale = [NSLocale currentLocale];
            NSString* identifier = [locale localeIdentifier];
            s_global.userLocale = identifier ? [identifier UTF8String] : "en";
            s_global.userLocale.replace('_', '-');
        }
    }

    void initUserInfo()
    {
        char buffer[kBufferSize]{};
        if (::gethostname(buffer, sizeof(buffer)) == 0)
            s_global.computerName = buffer;

        passwd pwd{};
        passwd* pwdResult = nullptr;
        if (::getpwuid_r(::getuid(), &pwd, buffer, sizeof(buffer), &pwdResult) == 0 && pwdResult)
        {
            s_global.userName = pwd.pw_name;
            s_global.homeDir = pwd.pw_dir;
        }
        const char* tmp = ::getenv("TMPDIR");
        s_global.tempDir = tmp && *tmp ? tmp : "/tmp";
    }

    void initOsInfo()
    {
        s_global.osVersion = versionFromNSVersion([[NSProcessInfo processInfo] operatingSystemVersion]);
#if TK_IOS
        s_global.osName = "iOS";
#else
        s_global.osName = "macOS";
#endif
    }

    void initDeviceId()
    {
        s_global.deviceId = TkUuid::Empty;
#if TK_MACOS
        io_registry_entry_t root = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
        CFStringRef uuidString = root ? static_cast<CFStringRef>(IORegistryEntryCreateCFProperty(root, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0)) : nullptr;
        if (root)
            IOObjectRelease(root);
        if (uuidString)
        {
            TkUuid::parse(AppleUtils::toString(uuidString), s_global.deviceId);
            CFRelease(uuidString);
        }
#elif TK_IOS
        NSString* uuidString = [[[UIDevice currentDevice] identifierForVendor] UUIDString];
        if (uuidString)
            TkUuid::parse(TkString([uuidString UTF8String]), s_global.deviceId);
#endif
        if (!s_global.deviceId)
        {
            u32 hash = HashFunc<TkString>::get(s_global.computerName);
            combineHash(hash, HashFunc<TkString>::get(s_global.userName));
            s_global.deviceId.a = hash;
            s_global.deviceId.b = HashFunc<TkString>::get(System::getExecPath());
            s_global.deviceId.c = static_cast<u32>(System::getMemoryStats().physicalMemSize);
            const CPUStats cpu = System::getCPUStats();
            s_global.deviceId.d = static_cast<u32>(cpu.speed) * cpu.logicalCoreCount * cpu.coreCount * cpu.cacheLineSize;
        }
    }

    Rect mainDisplayBounds()
    {
#if TK_IOS
        CGRect frame = [[UIScreen mainScreen] bounds];
        const CGFloat scale = [[UIScreen mainScreen] scale];
        return Rect(0, 0, static_cast<float>(frame.size.width * scale), static_cast<float>(frame.size.height * scale));
#else
        CGDirectDisplayID display = CGMainDisplayID();
        CGRect rect = CGDisplayBounds(display);
        return Rect(static_cast<float>(rect.origin.x), static_cast<float>(rect.origin.y), static_cast<float>(rect.size.width), static_cast<float>(rect.size.height));
#endif
    }

    bool System::init()
    {
        if (!BaseSystem::init())
            return false;

        mach_timebase_info_data_t timebase{};
        mach_timebase_info(&timebase);
        s_global.secondsPerCycle = 1e-9 * static_cast<double>(timebase.numer) / static_cast<double>(timebase.denom);

#if TK_IOS
        setAppleScreenScale(static_cast<float>([[UIScreen mainScreen] scale]));
#elif TK_MACOS
        NSArray* screens = [NSScreen screens];
        for (NSScreen* screen in screens)
        {
            if ([screen respondsToSelector:@selector(backingScaleFactor)])
                setAppleScreenScale(TkMath::max(getAppleScreenScale(), static_cast<float>([screen backingScaleFactor])));
        }
#endif

        initOsInfo();
        initLocale();
        initUserInfo();
        initCpuStats();
        s_global.programSizeMem = System::getProcessMemoryStats().usedPhysicalSize;
        initDeviceId();
        s_global.autoreleasePool = [[NSAutoreleasePool alloc] init];

        return true;
    }

    void System::logInfo()
    {
        LogInfo("%s %i-bit (%s)", s_global.osName.getText(), TK_PLATFORM_64BITS ? 64 : 32, s_global.osVersion.toString().getText());
        BaseSystem::logInfo();
    }

    void System::beforeRun()
    {
#if TK_MACOS
        initMacApplication();
#endif
    }

    void System::update()
    {
#if TK_MACOS
        updateMacApplicationEvents();
#endif
        if (++s_global.autoreleasePoolInterval >= 60)
        {
            s_global.autoreleasePoolInterval = 0;
            [s_global.autoreleasePool drain];
            s_global.autoreleasePool = [[NSAutoreleasePool alloc] init];
        }
    }

    void System::beforeQuit()
    {
    }

    void System::quit()
    {
        releaseProcessMutex();
        if (s_global.autoreleasePool)
        {
            [s_global.autoreleasePool drain];
            s_global.autoreleasePool = nullptr;
        }
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
        u64 physical = 0;
        if (!sysctlValue("hw.memsize", physical) || physical == 0)
            physical = 1024 * 1024;
        result.physicalMemSize = physical;

        mach_port_t host = mach_host_self();
        vm_statistics64_data_t vmstat{};
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        if (host_statistics64(host, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vmstat), &count) == KERN_SUCCESS)
        {
            const u64 pageSize = static_cast<u64>(::getpagesize());
            const u64 freeMem = (static_cast<u64>(vmstat.free_count) + static_cast<u64>(vmstat.inactive_count)) * pageSize;
            result.usedPhysicalSize = physical > freeMem ? physical - freeMem : 0;
        }
        else
        {
            result.usedPhysicalSize = 0;
        }

        result.virtualMemSize = result.physicalMemSize;
        result.usedVirtualSize = result.usedPhysicalSize;
#if TK_MACOS
        xsw_usage swap{};
        size_t swapSize = sizeof(swap);
        if (::sysctlbyname("vm.swapusage", &swap, &swapSize, nullptr, 0) == 0)
        {
            result.virtualMemSize += swap.xsu_total;
            result.usedVirtualSize += swap.xsu_used;
        }
#endif
        result.programSizeMem = s_global.programSizeMem;
        return result;
    }

    ProcessMemoryStats System::getProcessMemoryStats()
    {
        ProcessMemoryStats result;
        task_vm_info_data_t info{};
        mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
        if (task_info(mach_task_self(), TASK_VM_INFO, reinterpret_cast<task_info_t>(&info), &count) == KERN_SUCCESS)
        {
            result.usedPhysicalSize = info.phys_footprint;
            result.usedVirtualSize = info.virtual_size;
        }
        return result;
    }

    LibHandle System::loadLib(const char* filename)
    {
        TK_ASSERT(filename);
        PROFILE();
        void* handle = ::dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
        if (!handle)
            LogError("[AppleSystem] Failed to load library %s: %s", filename, ::dlerror());
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
            pipeHandles[0] = pipeHandles[1] = -1;

        const TkString commandLine = shellQuote(args.file) + " " + args.args;
        const pid_t pid = ::fork();
        if (pid < 0)
            return errno;

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
            ::execl("/bin/sh", "sh", "-c", commandLine.getText(), static_cast<char*>(nullptr));
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
        return s_global.secondsPerCycle * static_cast<double>(mach_absolute_time());
    }

    u64 System::getTimeCycles()
    {
        return mach_absolute_time();
    }

    u64 System::getClockFrequency()
    {
        return static_cast<u64>(1.0 / s_global.secondsPerCycle);
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
        char buffer[PATH_MAX]{};
        u32 size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0)
            return TkString(buffer);
        return TkString::Empty;
    }

    TkUuid System::getUniqueDeviceId()
    {
        return s_global.deviceId;
    }

    TkString System::getWorkDir()
    {
        char buffer[PATH_MAX]{};
        return ::getcwd(buffer, sizeof(buffer)) ? TkString(buffer) : TkString::Empty;
    }

    TkString System::getTempDir()
    {
        return s_global.tempDir;
    }

    bool System::setWorkDir(const TkString& path)
    {
        return ::chdir(path.getText()) == 0;
    }

    bool System::openUrl(const TkStringView url)
    {
        @autoreleasepool
        {
#if TK_MACOS
            NSURL* nsUrl = [NSURL URLWithString:AppleUtils::toNSString(url)];
            return nsUrl && [[NSWorkspace sharedWorkspace] openURL:nsUrl];
#elif TK_IOS
            NSURL* nsUrl = [NSURL URLWithString:AppleUtils::toNSString(url)];
            if (!nsUrl)
                return false;
            __block bool opened = false;
            void (^openBlock)(void) = ^{
                opened = [[UIApplication sharedApplication] openURL:nsUrl] == YES;
            };
            if ([NSThread isMainThread])
                openBlock();
            else
                dispatch_sync(dispatch_get_main_queue(), openBlock);
            return opened;
#else
            (void)url;
            return false;
#endif
        }
    }

    void System::setHighDpiAware(bool enable)
    {
        if (!enable)
            setAppleScreenScale(1.0f);
    }

    i32 System::getDpi()
    {
#if TK_IOS
        return static_cast<i32>(163.0f * getAppleScreenScale());
#else
        CGDirectDisplayID display = CGMainDisplayID();
        CGSize size = CGDisplayScreenSize(display);
        const float widthPixels = static_cast<float>(CGDisplayPixelsWide(display));
        if (size.width <= 0.0 || widthPixels <= 0.0f)
            return 96;
        return TkMath::max(static_cast<i32>((widthPixels * 25.4f) / static_cast<float>(size.width)), 72);
#endif
    }

    float System::getDpiScale()
    {
        return static_cast<float>(System::getDpi()) / 96.0f;
    }

    Pos System::getMousePos()
    {
#if TK_MACOS
        CGEventRef event = CGEventCreate(nullptr);
        CGPoint cursor = CGEventGetLocation(event);
        CFRelease(event);
        return Pos(static_cast<float>(cursor.x), static_cast<float>(cursor.y)) * getAppleScreenScale();
#else
        return Pos(0, 0);
#endif
    }

    void System::setMousePos(const Pos pos)
    {
#if TK_MACOS
        CGPoint cursor;
        cursor.x = pos.x / getAppleScreenScale();
        cursor.y = pos.y / getAppleScreenScale();
        CGWarpMouseCursorPosition(cursor);
        CGAssociateMouseAndMouseCursorPosition(true);
#else
        (void)pos;
#endif
    }

    Rect System::getMonitorBounds(const Pos pos)
    {
#if TK_MACOS
        CGPoint point{pos.x / getAppleScreenScale(), pos.y / getAppleScreenScale()};
        CGDirectDisplayID display = 0;
        uint32_t count = 0;
        CGGetDisplaysWithPoint(point, 1, &display, &count);
        if (count == 1)
        {
            CGRect rect = CGDisplayBounds(display);
            return Rect(static_cast<float>(rect.origin.x), static_cast<float>(rect.origin.y), static_cast<float>(rect.size.width), static_cast<float>(rect.size.height));
        }
#else
        (void)pos;
#endif
        return mainDisplayBounds();
    }

    Pos System::getDesktopSize()
    {
        const Rect bounds = mainDisplayBounds();
        return Pos(bounds.w, bounds.h);
    }

    Rect System::getVDesktopBounds()
    {
#if TK_MACOS
        CGDirectDisplayID displays[16]{};
        uint32_t count = 0;
        CGGetOnlineDisplayList(lengthOf(displays), displays, &count);
        if (count == 0)
            return mainDisplayBounds();
        CGRect combined = CGDisplayBounds(displays[0]);
        for (uint32_t i = 1; i < count; ++i)
            combined = CGRectUnion(combined, CGDisplayBounds(displays[i]));
        return Rect(static_cast<float>(combined.origin.x), static_cast<float>(combined.origin.y), static_cast<float>(combined.size.width), static_cast<float>(combined.size.height));
#else
        return mainDisplayBounds();
#endif
    }

    Pos System::GetVDesktopSize()
    {
        const Rect bounds = getVDesktopBounds();
        return Pos(bounds.w, bounds.h);
    }

    BatteryInfo System::getBatteryStats()
    {
        BatteryInfo result;
        result.percent = 1.0f;
#if TK_IOS
        UIDevice* device = [UIDevice currentDevice];
        device.batteryMonitoringEnabled = YES;
        result.percent = TkMath::saturate(static_cast<f32>(device.batteryLevel));
        switch (device.batteryState)
        {
        case UIDeviceBatteryStateUnplugged:
            result.state = BatteryInfo::kDischarging;
            break;
        case UIDeviceBatteryStateCharging:
            result.state = BatteryInfo::kCharging;
            break;
        case UIDeviceBatteryStateFull:
            result.state = BatteryInfo::kConnected;
            break;
        default:
            break;
        }
#endif
        return result;
    }

    NetConType System::getNetConType()
    {
        sockaddr_in address{};
        address.sin_len = sizeof(address);
        address.sin_family = AF_INET;
        SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, reinterpret_cast<const sockaddr*>(&address));
        if (!reachability)
            return NetConType::kUnknown;

        SCNetworkReachabilityFlags flags = 0;
        const bool ok = SCNetworkReachabilityGetFlags(reachability, &flags);
        CFRelease(reachability);
        if (!ok)
            return NetConType::kUnknown;
        if ((flags & kSCNetworkReachabilityFlagsReachable) == 0)
            return flags == 0 ? NetConType::kAirplaneMode : NetConType::kNone;
        if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) != 0)
            return NetConType::kNone;
#if TK_IOS
        if ((flags & kSCNetworkReachabilityFlagsIsWWAN) != 0)
            return NetConType::kCell;
#endif
        return NetConType::kWiFi;
    }

    ScreenOrienType System::getScreenOrienType()
    {
#if TK_IOS
        UIInterfaceOrientation orientation = UIInterfaceOrientationUnknown;
        if ([NSThread isMainThread])
        {
            orientation = [[[[UIApplication sharedApplication] delegate] window] windowScene].interfaceOrientation;
        }
        switch (orientation)
        {
        case UIInterfaceOrientationPortrait:
            return ScreenOrienType::kPortrait;
        case UIInterfaceOrientationPortraitUpsideDown:
            return ScreenOrienType::kPortraitUpsideDown;
        case UIInterfaceOrientationLandscapeLeft:
            return ScreenOrienType::kLandscapeLeft;
        case UIInterfaceOrientationLandscapeRight:
            return ScreenOrienType::kLandscapeRight;
        default:
            break;
        }
#endif
        const Pos size = getDesktopSize();
        return size.x >= size.y ? ScreenOrienType::kLandscapeLeft : ScreenOrienType::kPortrait;
    }

    TkString System::getLocale()
    {
        return s_global.userLocale;
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
        uuid_t uuid;
        uuid_generate(uuid);
        TkMemory::memoryCopy(result.raw, uuid, sizeof(result.raw));
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
#if TK_MACOS
        [[NSPasteboard generalPasteboard] clearContents];
#elif TK_IOS
        [UIPasteboard generalPasteboard].string = @"";
#endif
    }

    void System::setClipBoard(const TkStringView text)
    {
        s_global.clipboardText = text;
        s_global.clipboardData.clear();
        s_global.clipboardFiles.clear();
#if TK_MACOS
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard clearContents];
        [pasteboard setString:AppleUtils::toNSString(text) forType:NSPasteboardTypeString];
#elif TK_IOS
        [UIPasteboard generalPasteboard].string = AppleUtils::toNSString(text);
#endif
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
#if TK_MACOS
        NSString* text = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
        if (text)
            return TkString([text UTF8String]);
#elif TK_IOS
        NSString* text = [UIPasteboard generalPasteboard].string;
        if (text)
            return TkString([text UTF8String]);
#endif
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

#endif
