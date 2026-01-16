/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Debug.hpp"

#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/types/StringBuilder.hpp"

namespace toolkit
{
    TkString Debug::getStackTrace(i32 skipCount, i32 maxDepth, void *context)
    {
        TkStringBuilder result {};
        TkArray<StackFrame> stackFrames = getStackFrames(skipCount, maxDepth, context);
        for (const auto &frame : stackFrames)
        {
            const TkStringView functionNameStr(frame.func);
            if (TkStringUtils::length(frame.file) != 0)
            {
                result.append("   at ").append(functionNameStr);
                if (!functionNameStr.endsWith(')'))
                    result.append("()");
                result.appendFormat("in %s:line %i\n", frame.file, frame.line);
            }
            else if (TkStringUtils::length(frame.func) != 0)
            {
                result.append("   at ").append(functionNameStr);
                if (!functionNameStr.endsWith(')'))
                    result.append("()");
                result.append('\n');
            }
            else
            {
                result.appendFormat("   at 0x%x\n", (u64)frame.programCounter);
            }
        }
        return result.toString();
    }
}