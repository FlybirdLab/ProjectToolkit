#include "CmdLine.hpp"

#include "toolkit/collection/Array.hpp"
#include <toolkit/utils/Convertor.hpp>
#include <iostream>

namespace toolkit {

    static OptionsData s_options;

    bool parseArg(char* ptr, char*& start, char*& end)
    {
        // Skip leading whitespaces
        while (*ptr && TkStringUtils::isWhitespace(*ptr))
            ptr++;

        bool isInComma = false;
        bool isUglyQuote = false;
        start = ptr;
        while (*ptr)
        {
            char c = *ptr;
            if (TkStringUtils::isWhitespace(c) && !isInComma)
            {
                // End
                end = ptr;
                return false;
            }
            else if (c == '\"' || c == '\'')
            {
                if (isInComma)
                {
                    // End comma
                    end = ptr;
                    if (isUglyQuote)
                    {
                        ptr += 2;
                        end -= 2;
                    }
                    return false;
                }
                else
                {
                    // Special case (eg. Visual Studio Code adds soo many quotes to the args with spaces)
                    isUglyQuote = TkStringUtils::compareStrings(ptr, "\"\\\\\"", 4) == 0;
                    if (isUglyQuote)
                    {
                        ptr += 3;
                    }

                    // Start comma
                    isInComma = true;
                    start = ptr + 1;
                }
            }

            ptr++;
        }
        return true;
    }


    bool parseCmdLine(const char* cmdLine)
    {
        s_options.args = cmdLine;

        i32 length = TkStringUtils::length(cmdLine);
        if (length == 0)
            return false;
        TkArray<char> buffer;
        buffer.resize(length + 2);
        memcpy(buffer.get(), cmdLine, sizeof(char) * length);
        buffer[length++] = ' ';
        buffer[length++] = 0;
        char* end = buffer.get() + length;

        char* pos;
        char* argStart;
        char* argEnd;
        i32 len;

#define PARSE_BOOL_SWITCH(text, field) \
		pos = (char*)TkStringUtils::findInsensitive(buffer.get(), text); \
		if (pos) \
		{ \
			len = lengthOf(text) - 1; \
			TkConvertor::unsafeMemoryCopy(pos, pos + len, (end - pos - len) * 2); \
			*(end - len) = 0; \
			end -= len; \
			s_options.field = true; \
		}
#define PARSE_ARG_SWITCH(text, field) \
		pos = (char*)TkStringUtils::findInsensitive(buffer.get(), text); \
		if (pos) \
		{ \
			len = lengthOf(text) - 1; \
			if (parseArg(pos + len, argStart, argEnd)) \
			{ \
				std::cout << "Failed to parse argument." << std::endl; \
				return true; \
			} \
			s_options.field = TkString(argStart, static_cast<i32>(argEnd - argStart)); \
			len = static_cast<i32>((argEnd - pos) + 1); \
			TkConvertor::unsafeMemoryCopy(pos, pos + len, (end - pos - len) * 2); \
			*(end - len) = 0; \
			end -= len; \
		}

#define PARSE_ARG_OPT_SWITCH(text, field) \
		pos = (char*)TkStringUtils::findInsensitive(buffer.get(), text); \
		if (pos) \
		{ \
			len = lengthOf(text) - 1; \
			if (parseArg(pos + len, argStart, argEnd)) \
				s_options.field = String::Empty; \
			else \
            { \
			    s_options.field = TkString(argStart, static_cast<i32>(argEnd - argStart)); \
			    len = static_cast<i32>((argEnd - pos) + 1); \
			    TkConvertor::unsafeMemoryCopy(pos, pos + len, (end - pos - len) * 2); \
			    *(end - len) = 0; \
			    end -= len; \
			} \
		}

        PARSE_ARG_SWITCH("-source ", sourcePath);
        PARSE_ARG_SWITCH("-s ", sourcePath);
        PARSE_ARG_SWITCH("-output ", outputPath);
        PARSE_ARG_SWITCH("-o ", outputPath);

        return false;
    }


    OptionsData* getCmdLine() {
        return &s_options;
    }
}