/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/debug/Debug.hpp>

#if TK_LINUX

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

namespace toolkit {

	bool Debug::isDebuggerPresent()
	{
		static i32 cachedState = -1;
		if (cachedState != -1)
			return cachedState == 1;

		cachedState = 0;
		FILE *file = ::fopen("/proc/self/status", "r");
		if (!file)
			return false;

		char line[256];
		while (::fgets(line, sizeof(line), file))
		{
			if (TkStringUtils::compareStrings(line, "TracerPid:", 10) == 0)
			{
				i32 tracerPid = 0;
				::sscanf(line + 10, "%i", &tracerPid);
				cachedState = tracerPid != 0 ? 1 : 0;
				break;
			}
		}
		::fclose(file);
		return cachedState == 1;
	}

	TkArray<StackFrame> Debug::getStackFrames(i32 skipCount, i32 maxDepth, void *)
	{
		TkArray<StackFrame> result;
#if TK_CRASH_LOG
		void *callstack[120];
		skipCount = TkMath::min(skipCount, static_cast<i32>(lengthOf(callstack)));
		const i32 maxCount = TkMath::min(static_cast<i32>(lengthOf(callstack)), skipCount + maxDepth);
		const i32 count = ::backtrace(callstack, maxCount);
		const i32 useCount = count - skipCount;
		if (useCount > 0)
		{
			char **names = ::backtrace_symbols(callstack + skipCount, useCount);
			result.resize(useCount);
			for (i32 i = 0; i < useCount; ++i)
			{
				StackFrame &frame = result[i];
				TkMemory::memoryClear(&frame, sizeof(frame));
				frame.programCounter = callstack[skipCount + i];
				if (names && names[i])
					TkStringUtils::copyString(TkSpan<char>(frame.func, static_cast<i32>(sizeof(frame.func))), names[i]);
			}
			if (names)
				::free(names);
		}
#else
		(void)skipCount;
		(void)maxDepth;
#endif
		return result;
	}

	void Debug::getStackFrames(symbol_fn fn, i32 skipCount, i32 maxDepth, void *context)
	{
		TkArray<StackFrame> frames = getStackFrames(skipCount, maxDepth, context);
		for (const StackFrame &frame : frames)
			fn(frame);
	}

	void Debug::collectCrashData(const TkString &, void *)
	{
	}

	void Debug::log(const TkStringView msg)
	{
		LogInfo("%s", msg.getText());
	}

	void system_msgbox_show(const char *msg, const char *title, int)
	{
		LogError("[%s] %s", title ? title : "Message", msg ? msg : "");
	}

#if TK_DEBUG
	bool runningFromATerminal()
	{
		return ::isatty(STDERR_FILENO) != 0;
	}
#endif
}

#endif
