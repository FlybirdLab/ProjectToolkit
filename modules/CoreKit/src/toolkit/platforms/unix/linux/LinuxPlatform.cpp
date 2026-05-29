/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/System.hpp>

#if TK_LINUX

#include "IncludeX11.hpp"

#include <toolkit/base/Hash.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/debug/Debug.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/debug/Profiler.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/ProcessInitArgs.hpp>
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
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#if TK_CRASH_LOG
#include <execinfo.h>
#endif

extern char **environ;

namespace toolkit
{
	constexpr i32 kBufferSize = 1024;

	struct LinuxSystemGlobals
	{
		CPUStats cpuStats{};
		TkUuid deviceId;
		TkString userLocale;
		TkString computerName;
		TkString userName;
		TkString homeDir;
		TkString tempDir;
		TkString osName{"Linux"};
		TkVersion osVersion{0, 0};
		u64 programSizeMem = 0;
		i32 clockSource = CLOCK_MONOTONIC;
		i32 processMutexFd = -1;

		X11::Display *display = nullptr;
		X11::Window clipboardWindow = 0;
		X11::Atom clipboardAtom = 0;
		X11::Atom targetsAtom = 0;
		X11::Atom utf8Atom = 0;
		X11::Atom textAtom = 0;
		X11::Atom clipboardPropertyAtom = 0;
		TkString clipboardText;
		TkArray<byte> clipboardData;
		TkArray<TkString> clipboardFiles;
	};

	LinuxSystemGlobals s_global{};

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

	TkString readFirstLine(const char *path)
	{
		FILE *file = ::fopen(path, "r");
		if (!file)
			return {};

		char buffer[kBufferSize]{};
		char *result = ::fgets(buffer, sizeof(buffer), file);
		::fclose(file);
		if (!result)
			return {};

		i32 length = TkStringUtils::length(buffer);
		while (length > 0 && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r' || buffer[length - 1] == ' '))
			buffer[--length] = 0;

		if (length >= 2 && buffer[0] == '"' && buffer[length - 1] == '"')
			return TkString(buffer + 1, length - 2);
		return TkString(buffer, length);
	}

	bool readOsReleaseValue(const char *key, TkString &value)
	{
		FILE *file = ::fopen("/etc/os-release", "r");
		if (!file)
			return false;

		char line[kBufferSize]{};
		const i32 keyLength = TkStringUtils::length(key);
		while (::fgets(line, sizeof(line), file))
		{
			if (TkStringUtils::compareStrings(line, key, keyLength) != 0 || line[keyLength] != '=')
				continue;

			char *start = line + keyLength + 1;
			i32 length = TkStringUtils::length(start);
			while (length > 0 && (start[length - 1] == '\n' || start[length - 1] == '\r' || start[length - 1] == ' '))
				start[--length] = 0;
			if (length >= 2 && start[0] == '"' && start[length - 1] == '"')
				value = TkString(start + 1, length - 2);
			else
				value = TkString(start, length);
			::fclose(file);
			return true;
		}

		::fclose(file);
		return false;
	}

	TkVersion parseVersionLike(const TkString &text)
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

	u64 parseSizeString(const TkString &text)
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

	i32 readCpuTopologyValue(i32 cpuIndex, const char *name, i32 defaultValue)
	{
		char path[256];
		::snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%i/topology/%s", cpuIndex, name);
		const TkString value = readFirstLine(path);
		i32 result = defaultValue;
		if (value.hasChars())
			TkStringUtils::fromString(value.get(), result);
		return result < 0 ? defaultValue : result;
	}

	bool parseCpuIndex(const char *name, i32 &result)
	{
		if (!name || name[0] != 'c' || name[1] != 'p' || name[2] != 'u' || !TkStringUtils::isDigit(name[3]))
			return false;

		i32 value = 0;
		for (i32 i = 3; name[i]; ++i)
		{
			if (!TkStringUtils::isDigit(name[i]))
				return false;
			value = value * 10 + (name[i] - '0');
		}

		result = value;
		return true;
	}

	void initCpuStats()
	{
		CPUStats &stats = s_global.cpuStats;
		stats = {};

		struct CpuPair
		{
			i32 package;
			i32 core;
		};
		CpuPair pairs[1024]{};
		i32 pairCount = 0;
		i32 packages[1024]{};
		i32 packageCount = 0;
		i32 logicalCount = 0;

		DIR *cpuDir = ::opendir("/sys/devices/system/cpu");
		if (cpuDir)
		{
			dirent *entry = nullptr;
			while ((entry = ::readdir(cpuDir)) != nullptr)
			{
				i32 cpuIndex = 0;
				if (!parseCpuIndex(entry->d_name, cpuIndex))
					continue;

				++logicalCount;
				const i32 core = readCpuTopologyValue(cpuIndex, "core_id", cpuIndex);
				const i32 package = readCpuTopologyValue(cpuIndex, "physical_package_id", 0);

				bool foundPair = false;
				for (i32 i = 0; i < pairCount; ++i)
				{
					if (pairs[i].package == package && pairs[i].core == core)
					{
						foundPair = true;
						break;
					}
				}
				if (!foundPair && pairCount < static_cast<i32>(lengthOf(pairs)))
					pairs[pairCount++] = {package, core};

				bool foundPackage = false;
				for (i32 i = 0; i < packageCount; ++i)
				{
					if (packages[i] == package)
					{
						foundPackage = true;
						break;
					}
				}
				if (!foundPackage && packageCount < static_cast<i32>(lengthOf(packages)))
					packages[packageCount++] = package;
			}
			::closedir(cpuDir);
		}

		if (logicalCount <= 0)
		{
			const long processors = ::sysconf(_SC_NPROCESSORS_ONLN);
			logicalCount = processors > 0 ? static_cast<i32>(processors) : 1;
			pairCount = logicalCount;
			packageCount = 1;
		}

		stats.logicalCoreCount = static_cast<u32>(TkMath::max(logicalCount, 1));
		stats.coreCount = static_cast<u32>(TkMath::max(pairCount, 1));
		stats.packageCount = static_cast<u32>(TkMath::max(packageCount, 1));

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
		::setlocale(LC_ALL, "");
		const char *locale = ::setlocale(LC_CTYPE, nullptr);
		s_global.userLocale = locale && *locale ? locale : "en";
		const i32 encodingSeparator = s_global.userLocale.find('.');
		if (encodingSeparator != -1)
			s_global.userLocale = s_global.userLocale.left(encodingSeparator);
		s_global.userLocale.replace('_', '-');
		if (s_global.userLocale == "C" || s_global.userLocale == "POSIX")
			s_global.userLocale = "en";
	}

	void initUserInfo()
	{
		char buffer[kBufferSize]{};
		if (::gethostname(buffer, sizeof(buffer)) == 0)
			s_global.computerName = buffer;
		else
			s_global.computerName = "Linux";

		passwd pwd{};
		passwd *pwdResult = nullptr;
		if (::getpwuid_r(::getuid(), &pwd, buffer, sizeof(buffer), &pwdResult) == 0 && pwdResult)
		{
			s_global.userName = pwd.pw_name;
			s_global.homeDir = pwd.pw_dir;
		}
		if (s_global.userName.isEmpty())
		{
			const char *user = ::getenv("USER");
			s_global.userName = user && *user ? user : "user";
		}
		if (s_global.homeDir.isEmpty())
		{
			const char *home = ::getenv("HOME");
			s_global.homeDir = home && *home ? home : "/";
		}

		const char *tmp = ::getenv("TMPDIR");
		s_global.tempDir = tmp && *tmp ? tmp : "/tmp";
	}

	void initOsInfo()
	{
		TkString value;
		if (readOsReleaseValue("PRETTY_NAME", value) || readOsReleaseValue("NAME", value))
			s_global.osName = value;
		else
			s_global.osName = "Linux";

		if (readOsReleaseValue("VERSION_ID", value))
			s_global.osVersion = parseVersionLike(value);
		if (s_global.osVersion.major() == 0)
		{
			utsname info{};
			if (::uname(&info) == 0)
				s_global.osVersion = parseVersionLike(info.release);
		}
	}

	bool readMacAddress(byte mac[6])
	{
		DIR *dir = ::opendir("/sys/class/net");
		if (!dir)
			return false;

		dirent *entry = nullptr;
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

		u32 hash = HashFunc<TkString>::get(s_global.computerName);
		combineHash(hash, HashFunc<TkString>::get(s_global.userName));
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
			s_global.deviceId.b = HashFunc<TkString>::get(System::getExecPath());
		}

		s_global.deviceId.c = static_cast<u32>(System::getMemoryStats().physicalMemSize);
		const CPUStats cpu = System::getCPUStats();
		s_global.deviceId.d = static_cast<u32>(cpu.speed) * cpu.logicalCoreCount * cpu.coreCount * cpu.cacheLineSize;
	}

	X11::Display *ensureDisplay()
	{
		if (s_global.display)
			return s_global.display;

		static const bool xThreadsInitialized = X11::XInitThreads() != 0;
		(void)xThreadsInitialized;

		s_global.display = X11::XOpenDisplay(nullptr);
		if (!s_global.display)
			return nullptr;

		s_global.clipboardAtom = X11::XInternAtom(s_global.display, "CLIPBOARD", 0);
		s_global.targetsAtom = X11::XInternAtom(s_global.display, "TARGETS", 0);
		s_global.utf8Atom = X11::XInternAtom(s_global.display, "UTF8_STRING", 0);
		s_global.textAtom = X11::XInternAtom(s_global.display, "TEXT", 0);
		s_global.clipboardPropertyAtom = X11::XInternAtom(s_global.display, "TOOLKIT_CLIPBOARD", 0);

		const i32 screen = X11::XDefaultScreen(s_global.display);
		s_global.clipboardWindow = X11::XCreateSimpleWindow(s_global.display, X11::XRootWindow(s_global.display, screen), 0, 0, 1, 1, 0, 0, 0);
		return s_global.display;
	}

	void handleClipboardSelectionRequest(X11::XSelectionRequestEvent &request)
	{
		X11::XSelectionEvent response{};
		response.type = SelectionNotify;
		response.display = request.display;
		response.requestor = request.requestor;
		response.selection = request.selection;
		response.target = request.target;
		response.time = request.time;
		response.property = request.property;

		if (request.target == s_global.targetsAtom)
		{
			X11::Atom targets[] = {s_global.targetsAtom, s_global.utf8Atom, s_global.textAtom};
			X11::XChangeProperty(request.display, request.requestor, request.property, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(targets), lengthOf(targets));
		}
		else if (request.target == s_global.utf8Atom || request.target == s_global.textAtom)
		{
			const X11::Atom target = request.target == s_global.textAtom ? XA_STRING : s_global.utf8Atom;
			X11::XChangeProperty(request.display, request.requestor, request.property, target, 8, PropModeReplace, reinterpret_cast<const unsigned char *>(s_global.clipboardText.getText()), s_global.clipboardText.length());
		}
		else
		{
			response.property = 0;
		}

		X11::XSendEvent(request.display, request.requestor, 0, 0, reinterpret_cast<X11::XEvent *>(&response));
		X11::XFlush(request.display);
	}

	void pumpX11Events()
	{
		X11::Display *display = ensureDisplay();
		if (!display)
			return;

		while (X11::XPending(display) > 0)
		{
			X11::XEvent event{};
			X11::XNextEvent(display, &event);
			if (event.type == SelectionRequest)
				handleClipboardSelectionRequest(event.xselectionrequest);
		}
	}

	TkString readExternalClipboardText()
	{
		X11::Display *display = ensureDisplay();
		if (!display || !s_global.clipboardWindow)
			return TkString::Empty;

		if (X11::XGetSelectionOwner(display, s_global.clipboardAtom) == s_global.clipboardWindow)
			return s_global.clipboardText;

		X11::XDeleteProperty(display, s_global.clipboardWindow, s_global.clipboardPropertyAtom);
		X11::XConvertSelection(display, s_global.clipboardAtom, s_global.utf8Atom, s_global.clipboardPropertyAtom, s_global.clipboardWindow, CurrentTime);
		X11::XFlush(display);

		for (i32 i = 0; i < 50; ++i)
		{
			while (X11::XPending(display) > 0)
			{
				X11::XEvent event{};
				X11::XNextEvent(display, &event);
				if (event.type == SelectionRequest)
				{
					handleClipboardSelectionRequest(event.xselectionrequest);
				}
				else if (event.type == SelectionNotify && event.xselection.property != 0)
				{
					X11::Atom actualType = 0;
					int actualFormat = 0;
					unsigned long itemCount = 0;
					unsigned long bytesAfter = 0;
					unsigned char *data = nullptr;
					const int result = X11::XGetWindowProperty(display, s_global.clipboardWindow, s_global.clipboardPropertyAtom, 0, LONG_MAX / 4, 0, AnyPropertyType, &actualType, &actualFormat, &itemCount, &bytesAfter, &data);
					if (result == Success && data)
					{
						TkString text(reinterpret_cast<const char *>(data), static_cast<i32>(itemCount));
						X11::XFree(data);
						return text;
					}
					if (data)
						X11::XFree(data);
					return TkString::Empty;
				}
			}
			TkThread::sleep(2);
		}

		return TkString::Empty;
	}

	Rect getFallbackDesktopBounds()
	{
		X11::Display *display = ensureDisplay();
		if (!display)
			return Rect(0, 0, 800, 600);

		const i32 screen = X11::XDefaultScreen(display);
		return Rect(0, 0, static_cast<f32>(X11::XDisplayWidth(display, screen)), static_cast<f32>(X11::XDisplayHeight(display, screen)));
	}

	bool System::init()
	{
		if (!BaseSystem::init())
			return false;

		timespec ts{};
		s_global.clockSource = ::clock_gettime(CLOCK_MONOTONIC, &ts) == 0 ? CLOCK_MONOTONIC : CLOCK_REALTIME;

		initOsInfo();
		initUserInfo();
		initLocale();
		initCpuStats();
		s_global.programSizeMem = System::getProcessMemoryStats().usedPhysicalSize;
		initDeviceId();
		ensureDisplay();

		return true;
	}

	void System::logInfo()
	{
		LogInfo("Linux %s %i-bit (%s)", s_global.osName.getText(), TK_PLATFORM_64BITS ? 64 : 32, s_global.osVersion.toString().getText());
		BaseSystem::logInfo();
	}

	void System::beforeRun()
	{
	}

	void System::update()
	{
		pumpX11Events();
	}

	void System::beforeQuit()
	{
	}

	void System::quit()
	{
		releaseProcessMutex();
		if (s_global.display)
		{
			if (s_global.clipboardWindow)
			{
				X11::XDestroyWindow(s_global.display, s_global.clipboardWindow);
				s_global.clipboardWindow = 0;
			}
			X11::XCloseDisplay(s_global.display);
			s_global.display = nullptr;
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
		else
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
		FILE *file = ::fopen("/proc/self/statm", "r");
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

	LibHandle System::loadLib(const char *filename)
	{
		TK_ASSERT(filename);
		PROFILE();
#if TK_TRACY_ENABLE
		ZoneText(filename, TkStringUtils::length(filename));
#endif
		void *handle = ::dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
		if (!handle)
			LogError("[LinuxSystem] Failed to load library %s: %s", filename, ::dlerror());
		return handle;
	}

	void System::freeLib(LibHandle handle)
	{
		if (handle)
			::dlclose(handle);
	}

	void *System::getLibSymbol(LibHandle handle, const char *symbol)
	{
		return handle ? ::dlsym(handle, symbol) : nullptr;
	}

	i32 System::newProcess(ProcessInitArgs &args)
	{
		LogInfo("Command: %s %s", args.file.getText(), args.args.getText());
		if (args.workDir.hasChars())
			LogInfo("Working directory: %s", args.workDir.getText());

		const bool captureOutput = args.logOutput || args.saveOutput;
		i32 pipeHandles[2] = {-1, -1};
		if (captureOutput && ::pipe(pipeHandles) != 0)
		{
			LogWarn("[LinuxSystem] Failed to create pipe errno=%i", errno);
			pipeHandles[0] = pipeHandles[1] = -1;
		}

		const TkString commandLine = shellQuote(args.file) + " " + args.args;
		const pid_t pid = ::fork();
		if (pid < 0)
		{
			LogWarn("[LinuxSystem] Failed to fork process errno=%i", errno);
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

			::execl("/bin/sh", "sh", "-c", commandLine.getText(), static_cast<char *>(nullptr));
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
			{
				LogWarn("[LinuxSystem] waitpid failed errno=%i", errno);
				resultCode = errno;
			}
			else if (WIFEXITED(status))
			{
				resultCode = WEXITSTATUS(status);
			}
			else if (WIFSIGNALED(status))
			{
				resultCode = 128 + WTERMSIG(status);
			}
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

	void System::getSystemTime(TimeStats &time)
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

	void System::getUTCTime(TimeStats &time)
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
		return s_global.tempDir;
	}

	bool System::setWorkDir(const TkString &path)
	{
		return ::chdir(path.getText()) == 0;
	}

	bool System::openUrl(const TkStringView url)
	{
		ProcessInitArgs args;
		args.file = "xdg-open";
		args.args = shellQuote(url);
		args.waitForQuit = false;
		args.logOutput = false;
		args.saveOutput = false;
		return System::newProcess(args) == 0;
	}

	void System::setHighDpiAware(bool)
	{
	}

	i32 System::getDpi()
	{
		X11::Display *display = ensureDisplay();
		if (!display)
			return 96;

		const i32 screen = X11::XDefaultScreen(display);
		const i32 widthPx = X11::XDisplayWidth(display, screen);
		const i32 widthMm = X11::XDisplayWidthMM(display, screen);
		if (widthPx <= 0 || widthMm <= 0)
			return 96;

		return static_cast<i32>((static_cast<double>(widthPx) * 25.4 / static_cast<double>(widthMm)) + 0.5);
	}

	float System::getDpiScale()
	{
		return static_cast<float>(System::getDpi()) / 96.0f;
	}

	Pos System::getMousePos()
	{
		X11::Display *display = ensureDisplay();
		if (!display)
			return Pos(0, 0);

		const i32 screenCount = X11::XScreenCount(display);
		for (i32 i = 0; i < screenCount; ++i)
		{
			X11::Window root = X11::XRootWindow(display, i);
			X11::Window child = 0;
			X11::Window rootOut = 0;
			i32 rootX = 0;
			i32 rootY = 0;
			i32 childX = 0;
			i32 childY = 0;
			unsigned int mask = 0;
			if (X11::XQueryPointer(display, root, &rootOut, &child, &rootX, &rootY, &childX, &childY, &mask))
				return Pos(rootX, rootY);
		}
		return Pos(0, 0);
	}

	void System::setMousePos(const Pos pos)
	{
		X11::Display *display = ensureDisplay();
		if (!display)
			return;
		const i32 screen = X11::XDefaultScreen(display);
		X11::XWarpPointer(display, 0, X11::XRootWindow(display, screen), 0, 0, 0, 0, static_cast<i32>(pos.x), static_cast<i32>(pos.y));
		X11::XFlush(display);
	}

	Rect System::getMonitorBounds(const Pos)
	{
		return getFallbackDesktopBounds();
	}

	Pos System::getDesktopSize()
	{
		const Rect bounds = getFallbackDesktopBounds();
		return Pos(bounds.w, bounds.h);
	}

	Rect System::getVDesktopBounds()
	{
		return getFallbackDesktopBounds();
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

		DIR *dir = ::opendir("/sys/class/power_supply");
		if (!dir)
			return result;

		dirent *entry = nullptr;
		while ((entry = ::readdir(dir)) != nullptr)
		{
			if (TkStringUtils::compareStrings(entry->d_name, ".") == 0 || TkStringUtils::compareStrings(entry->d_name, "..") == 0)
				continue;

			char path[PATH_MAX];
			::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/type", entry->d_name);
			if (readFirstLine(path) != "Battery")
				continue;

			::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", entry->d_name);
			i32 capacity = 100;
			TkStringUtils::fromString(readFirstLine(path).getText(), capacity);
			result.percent = TkMath::clamp(static_cast<f32>(capacity) / 100.0f, 0.0f, 1.0f);

			::snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", entry->d_name);
			const TkString status = readFirstLine(path);
			if (status == "Charging")
				result.state = BatteryInfo::kCharging;
			else if (status == "Discharging")
				result.state = BatteryInfo::kDischarging;
			else if (status == "Full" || status == "Not charging")
				result.state = BatteryInfo::kConnected;
			::closedir(dir);
			return result;
		}

		::closedir(dir);
		return result;
	}

	NetConType System::getNetConType()
	{
		DIR *dir = ::opendir("/sys/class/net");
		if (!dir)
			return NetConType::kUnknown;

		NetConType result = NetConType::kNone;
		dirent *entry = nullptr;
		while ((entry = ::readdir(dir)) != nullptr)
		{
			if (TkStringUtils::compareStrings(entry->d_name, ".") == 0 ||
				TkStringUtils::compareStrings(entry->d_name, "..") == 0 ||
				TkStringUtils::compareStrings(entry->d_name, "lo") == 0)
			{
				continue;
			}

			char path[PATH_MAX];
			::snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", entry->d_name);
			if (readFirstLine(path) != "up")
				continue;

			::snprintf(path, sizeof(path), "/sys/class/net/%s/wireless", entry->d_name);
			struct stat info{};
			result = ::stat(path, &info) == 0 ? NetConType::kWiFi : NetConType::kEthernet;
			break;
		}
		::closedir(dir);
		return result;
	}

	ScreenOrienType System::getScreenOrienType()
	{
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

	void System::createUuid(TkUuid &result)
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

	void System::getEnvVars(TkHashMap<TkString, TkString> &result)
	{
		for (char **entry = environ; entry && *entry; ++entry)
		{
			char *separator = ::strchr(*entry, '=');
			if (!separator)
				result[TkString(*entry)] = TkString::Empty;
			else
				result[TkString(*entry, static_cast<i32>(separator - *entry))] = TkString(separator + 1);
		}
	}

	bool System::getEnvVar(const TkStringView name, TkString &value)
	{
		const TkString key(name);
		const char *env = ::getenv(key.getText());
		if (!env)
			return false;
		value = env;
		return true;
	}

	bool System::setEnvVar(const TkStringView name, const TkString &value)
	{
		const TkString key(name);
		return ::setenv(key.getText(), value.getText(), 1) == 0;
	}

	bool System::createProcessMutex(const char *name)
	{
		releaseProcessMutex();

		char path[PATH_MAX];
		::snprintf(path, sizeof(path), "/tmp/%s.pid", name);
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
		X11::Display *display = ensureDisplay();
		if (display && s_global.clipboardWindow && X11::XGetSelectionOwner(display, s_global.clipboardAtom) == s_global.clipboardWindow)
			X11::XSetSelectionOwner(display, s_global.clipboardAtom, 0, CurrentTime);
	}

	void System::setClipBoard(const TkStringView text)
	{
		s_global.clipboardText = text;
		s_global.clipboardData.clear();
		s_global.clipboardFiles.clear();

		X11::Display *display = ensureDisplay();
		if (display && s_global.clipboardWindow)
		{
			X11::XSetSelectionOwner(display, s_global.clipboardAtom, s_global.clipboardWindow, CurrentTime);
			X11::XFlush(display);
		}
	}

	void System::setClipBoard(const TkSpan<byte> data)
	{
		s_global.clipboardText.clear();
		s_global.clipboardFiles.clear();
		s_global.clipboardData.set(data.get(), data.length());
	}

	void System::setClipBoard(const TkArray<TkString> &files)
	{
		s_global.clipboardText.clear();
		s_global.clipboardData.clear();
		s_global.clipboardFiles = files;
	}

	TkString System::getClipBoardAsText()
	{
		TkString text = readExternalClipboardText();
		return text.hasChars() ? text : s_global.clipboardText;
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
