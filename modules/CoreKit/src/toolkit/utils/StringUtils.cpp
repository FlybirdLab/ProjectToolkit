/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "StringUtils.hpp"

#include <cmath>

#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/types/Span.hpp"
#include "toolkit/environment.hpp"

#include <cctype>
#include <cstring>

#if TK_WINDOWS
#include <string.h>
#else
#include <strings.h>
#endif

namespace toolkit
{

	static char makeLowercase(char c)
	{
		return c >= 'A' && c <= 'Z' ? c - ('A' - 'a') : c;
	}

	i32 TkStringUtils::hexDigit(char c)
	{
		i32 result = 0;
		if (c >= '0' && c <= '9')
			result = c - '0';
		else if (c >= 'a' && c <= 'f')
			result = c + 10 - 'a';
		else if (c >= 'A' && c <= 'F')
			result = c + 10 - 'A';
		return result;
	}

	bool TkStringUtils::equalStrings(TkStringView lhs, TkStringView rhs)
	{
		if (rhs.length() != lhs.length())
			return false;
		return strncmp(lhs.begin(), rhs.begin(), lhs.length()) == 0;
	}
	bool TkStringUtils::equalStrings(TkStringView lhs, TkStringView rhs, size_t len)
	{
		return equalStrings(lhs.begin(), rhs.begin(), len);
	}
	bool TkStringUtils::equalStrings(const char *lhs, const char *rhs)
	{
		return strcmp(lhs, rhs) == 0;
	}
	bool TkStringUtils::equalStrings(const char *lhs, const char *rhs, size_t len)
	{
		return strncmp(lhs, rhs, len) == 0;
	}

	bool TkStringUtils::equalIStrings(TkStringView lhs, TkStringView rhs)
	{
		if (lhs.length() != rhs.length())
			return false;
		return equalIStrings(lhs.begin(), rhs.begin());
	}
	bool TkStringUtils::equalIStrings(const char *lhs, const char *rhs)
	{
#if TK_WINDOWS
		return _stricmp(lhs, rhs) == 0;
#else
		return strcasecmp(lhs, rhs) == 0;
#endif
	}
	bool TkStringUtils::equalIStrings(const char *lhs, const char *rhs, size_t len)
	{
#if TK_WINDOWS
		return _strnicmp(lhs, rhs, len) == 0;
#else
		return strncasecmp(lhs, rhs, len) == 0;
#endif
	}
	bool TkStringUtils::equalIStrings(TkStringView lhs, TkStringView rhs, size_t len)
	{
		return equalIStrings(lhs.begin(), rhs.begin(), len);
	}

	i32 TkStringUtils::compareStrings(const char *str1, const char *str2)
	{
		return strcmp(str1, str2);
	}
	i32 TkStringUtils::compareStrings(const char *str1, const char *str2, i32 maxCount)
	{
		return strncmp(str1, str2, maxCount);
	}
	i32 TkStringUtils::compareIStrings(const char *str1, const char *str2)
	{
#if TK_WINDOWS
		return _stricmp(str1, str2);
#else
		return strcasecmp(str1, str2);
#endif
	}
	i32 TkStringUtils::compareIStrings(const char *str1, const char *str2, i32 maxCount)
	{
#if TK_WINDOWS
		return _strnicmp(str1, str2, maxCount);
#else
		return strncasecmp(str1, str2, maxCount);
#endif
	}

	i32 TkStringUtils::length(const char *str)
	{
		return str ? static_cast<i32>(strlen(str)) : 0;
	}
    i32 TkStringUtils::length(const char8_t *str)
    {
		static_assert(sizeof(char8_t) == sizeof(char));
		return TkStringUtils::length((const char*)str);
    }
    i32 TkStringUtils::length(const wchar *str)
    {
		return str ? static_cast<i32>(wcslen(str)) : 0;
	}

	bool TkStringUtils::endsWithInsensitive(TkStringView str, TkStringView suffix)
	{
		if (str.length() < suffix.length())
			return false;
		TkStringView fs{str.end() - suffix.length(), suffix.length()};
		return equalIStrings(fs, suffix);
	}

	bool TkStringUtils::endsWith(TkStringView str, TkStringView suffix)
	{
		if (str.length() < suffix.length())
			return false;
		TkStringView fs{str.end() - suffix.length(), suffix.length()};
		return equalStrings(fs, suffix);
	}

	const char *TkStringUtils::find(const char *str, char toFind)
	{
		const char *c = str;
		const char *end = str + sizeof(char) * length(str);
		while (c != end)
		{
			if (*c == toFind)
				return c;
			++c;
		}
		return nullptr;
	}

	const char *TkStringUtils::find(TkStringView str, char toFind)
	{
		const char *c = str.begin();
		while (c != str.end())
		{
			if (*c == toFind)
				return c;
			++c;
		}
		return nullptr;
	}

	bool TkStringUtils::makeLowercase(TkSpan<char> output, TkStringView src)
	{
		char *destination = output.begin();
		if (src.length() + 1 > output.length())
			return false;

		const char *source = src.begin();
		while (source != src.end())
		{
			*destination = toolkit::makeLowercase(*source);
			++destination;
			++source;
		}
		*destination = 0;
		return true;
	}

	const char *TkStringUtils::find(const char *str, const char *toFind)
	{
		return strstr(str, toFind);
	}
	const char *TkStringUtils::find(TkStringView str, TkStringView toFind)
	{
		return find(str.begin(), toFind.begin());
	}
	const char *TkStringUtils::findInsensitive(const char *str, const char *toFind)
	{
		if (toFind == nullptr || str == nullptr)
		{
			return nullptr;
		}

		const char findInitial = (char)toUpper(*toFind);
		const i32 length = TkStringUtils::length(toFind++) - 1;
		char c = *str++;
		while (c)
		{
			c = (char)toUpper(c);
			if (c == findInitial && !compareIStrings(str, toFind, length))
			{
				return str - 1;
			}

			c = *str++;
		}

		return nullptr;
	}
	const char *TkStringUtils::findInsensitive(TkStringView str, TkStringView toFind)
	{
		return findInsensitive(str.begin(), toFind.begin());
	}

	bool TkStringUtils::contains(TkStringView haystack, char needle)
	{
		return find(haystack, needle) != nullptr;
	}

	char *TkStringUtils::copyString(TkSpan<char> dst, TkStringView src)
	{
		if (dst.length() < 1)
			return dst.begin();

		TK_ASSERT(dst.begin() >= src.end() || dst.begin() <= src.begin());

		u32 length = dst.length();
		char *tmp = dst.begin();
		const char *srcp = src.begin();
		while (srcp != src.end() && length > 1)
		{
			*tmp = *srcp;
			--length;
			++tmp;
			++srcp;
		}
		*tmp = 0;
		return tmp;
	}

	const char *TkStringUtils::reverseFind(TkStringView haystack, char c)
	{
		if (haystack.length() == 0)
			return nullptr;

		const char *tmp = haystack.end() - 1;
		while (tmp >= haystack.begin())
		{
			if (*tmp == c)
				return tmp;
			--tmp;
		}

		return nullptr;
	}

	char *TkStringUtils::catString(TkSpan<char> destination, TkStringView source)
	{
		char *dst = destination.begin();
		u32 length = destination.length();
		while (*dst && length)
		{
			--length;
			++dst;
		}
		return copyString(TkSpan{dst, static_cast<i32>(length)}, source);
	}

	static void reverse(char *str, int length)
	{
		char *beg = str;
		char *end = str + length - 1;
		while (beg < end)
		{
			char tmp = *beg;
			*beg = *end;
			*end = tmp;
			++beg;
			--end;
		}
	}

	bool TkStringUtils::fromString(TkStringView str, u64 &result)
	{
		i64 sum = 0;
		const char *p = str.begin();
		auto length = str.length();
		while (length--)
		{
			i32 c = *p++ - 48;
			if (c < 0 || c > 9)
				return false;
			sum = 10 * sum + c;
		}
		result = sum;
		return true;
	}

	bool TkStringUtils::fromString(TkStringView str, u32 &result)
	{
		u64 tmp;
		const bool b = fromString(str, tmp);
		result = (u32)tmp;
		return b;
	}

	bool TkStringUtils::fromString(TkStringView str, u16 &result)
	{
		u64 tmp;
		const bool b = fromString(str, tmp);
		result = (u16)tmp;
		return b;
	}

	bool TkStringUtils::fromString(TkStringView str, u8 &result)
	{
		u64 tmp;
		const bool b = fromString(str, tmp);
		result = (u8)tmp;
		return b;
	}

	bool TkStringUtils::fromString(TkStringView str, i64 &result)
	{
		i64 sum = 0;
		const char *p = str.begin();
		auto length = str.length();
		bool negate = false;
		while (length--)
		{
			i32 c = *p++ - 48;
			if (c == -3)
			{
				negate = true;
				continue;
			}
			if (c < 0 || c > 9)
				return false;
			sum = 10 * sum + c;
		}
		if (negate)
			sum = -sum;
		result = sum;
		return true;
	}

	bool TkStringUtils::fromString(TkStringView str, i32 &result)
	{
		i64 tmp;
		const bool b = fromString(str, tmp);
		result = (i32)tmp;
		return b;
	}

	bool TkStringUtils::fromString(TkStringView str, i16 &result)
	{
		i64 tmp;
		const bool b = fromString(str, tmp);
		result = (i16)tmp;
		return b;
	}

	bool TkStringUtils::fromString(TkStringView str, i8 &result)
	{
		i64 tmp;
		const bool b = fromString(str, tmp);
		result = (i8)tmp;
		return b;
	}

	bool TkStringUtils::fromString(TkStringView str, f32 &result)
	{
		char *end = nullptr;
		result = strtof(str.begin(), &end);
		return end != str.begin();
	}

	bool TkStringUtils::fromString(TkStringView input, bool &value)
	{
		bool val = equalStrings(input, "true");
		value = val;
		return true;
	}

	const char *TkStringUtils::fromStringOctal(TkStringView input, u32 &value)
	{
		if (input.isEmpty())
			return nullptr;

		const char *c = input.begin();
		value = 0;
		if (*c < '0' || *c > '7')
			return nullptr;

		while (c != input.end() && *c >= '0' && *c <= '7')
		{
			value *= 8;
			value += *c - '0';
			++c;
		}
		return c;
	}

	void TkStringUtils::toStringPretty(i32 value, TkSpan<char> output)
	{
		char *c = output.begin();
		u32 length = output.length();
		if (length > 0)
		{
			if (value < 0)
			{
				value = -value;
				--length;
				*c = '-';
				++c;
			}
			return toStringPretty((unsigned int)value, TkSpan(c, length));
		}
	}

	void TkStringUtils::toStringPretty(u32 value, TkSpan<char> output)
	{
		return toStringPretty(u64(value), output);
	}

	void TkStringUtils::toStringPretty(u64 value, TkSpan<char> output)
	{
		char *c = output.begin();
		char *num_start = output.begin();
		u32 length = output.length();
		if (length > 0)
		{
			if (value == 0)
			{
				if (length == 1)
				{
					return;
				}
				*c = '0';
				*(c + 1) = 0;
				return;
			}
			int counter = 0;
			while (value > 0 && length > 1)
			{
				*c = value % 10 + '0';
				value = value / 10;
				--length;
				++c;
				if ((counter + 1) % 3 == 0 && length > 1 && value > 0)
				{
					*c = ',';
					++c;
					counter = 0;
				}
				else
				{
					++counter;
				}
			}
			if (length > 0)
			{
				reverse(num_start, (int)(c - num_start));
				*c = 0;
				return;
			}
		}
		return;
	}

	char *TkStringUtils::toString(i32 value, TkSpan<char> output)
	{
		char *c = output.begin();
		u32 length = output.length();
		if (length < 2)
			return nullptr;

		if (value < 0)
		{
			value = -value;
			--length;
			c[0] = '-';
			c[1] = 0;
			++c;
		}
		return toString((u32)value, TkSpan(c, length));
	}

	char *TkStringUtils::toString(i64 value, TkSpan<char> output)
	{
		char *c = output.begin();
		u32 length = output.length();
		if (length < 2)
			return nullptr;
		if (value < 0)
		{
			value = -value;
			--length;
			c[0] = '-';
			c[1] = 0;
			++c;
		}
		return toString((u64)value, TkSpan(c, length));
	}

	char *TkStringUtils::toString(u64 value, TkSpan<char> output)
	{
		char *c = output.begin();
		char *num_start = output.begin();
		u32 length = output.length();
		if (length < 2)
			return nullptr;

		if (value == 0)
		{
			*c = '0';
			*(c + 1) = 0;
			return c + 1;
		}

		while (value > 0 && length > 0)
		{
			*c = value % 10 + '0';
			value = value / 10;
			--length;
			++c;
		}

		if (length > 0)
		{
			reverse(num_start, (int)(c - num_start));
			*c = 0;
			return c;
		}

		*(c - 1) = 0;
		return nullptr;
	}

	void TkStringUtils::toStringHex(u8 value, TkSpan<char> output)
	{
		// FIX Hex output format error
		if (output.length() < 3)
		{ // 2 char + '\0'
			return;
		}
		u8 first = value / 16;
		output[0] = (first > 9) ? ('A' + first - 10) : ('0' + first);

		u8 second = value % 16;
		output[1] = (second > 9) ? ('A' + second - 10) : ('0' + second);

		output[2] = '\0'; // end string
	}

	char *TkStringUtils::toString(u32 value, TkSpan<char> output)
	{
		char *c = output.begin();
		char *num_start = output.begin();
		u32 length = output.length();
		if (length < 2)
			return nullptr;

		if (value == 0)
		{
			*c = '0';
			*(c + 1) = 0;
			return c + 1;
		}

		while (value > 0 && length > 0)
		{
			*c = value % 10 + '0';
			value = value / 10;
			--length;
			++c;
		}
		if (length > 0)
		{
			reverse(num_start, (int)(c - num_start));
			*c = 0;
			return c;
		}
		*(c - 1) = 0;
		return nullptr;
	}

	TkString TkStringUtils::toString(i32 value)
	{
		char val[11];
		TkSpan<char> outVal{val};
		char *end = toString(value, outVal);
		if (end == nullptr)
		{
			return TkString::Empty;
		}
		return TkString{&val[0], end};
	}

	TkString TkStringUtils::toString(i64 value)
	{
		char val[21];
		TkSpan<char> outVal{val};
		char *end = toString(value, outVal);
		if (end == nullptr)
		{
			return TkString::Empty;
		}
		return TkString{&val[0], end};
	}

	TkString TkStringUtils::toString(u32 value)
	{
		char val[20];
		TkSpan<char> outVal{val};
		char *end = toString(value, outVal);
		if (end == nullptr)
		{
			return TkString::Empty;
		}
		return TkString{&val[0], end};
	}

	TkString TkStringUtils::toString(u64 value)
	{
		char val[21];
		TkSpan<char> outVal{val};
		char *end = toString(value, outVal);
		if (end == nullptr)
		{
			return TkString::Empty;
		}
		return TkString{&val[0], end};
		return TkString::format("{}", value);
	}

	static int getDecimalDigitCount(float frac, int maxDigits = 7)
	{
		frac = fabsf(frac);
		int count = 0;
		for (int i = 0; i < maxDigits; ++i)
		{
			frac *= 10.0f;
			int digit = (int)(frac + 1e-4f); // add little bias to avoid conflict
			frac -= digit;

			++count;

			// check ends
			if (fabsf(frac) < 1e-4f)
			{
				break;
			}
		}
		return count;
	}

	static int getDecimalDigitCountD(double frac, int maxDigits = 17)
	{
		frac = std::fabs(frac);
		int count = 0;
		int lastNonZeroDigit = 0;

		for (int i = 0; i < maxDigits; ++i)
		{
			frac *= 10.0;
			int digit = (int)(frac + 1e-12);
			frac -= digit;

			++count;
			if (digit != 0)
				lastNonZeroDigit = count;

			if (std::fabs(frac) < 1e-12)
			{
				break;
			}
		}
		return lastNonZeroDigit;
	}

	TkString TkStringUtils::toString(f32 value)
	{
		char val[21];
		TkSpan<char> outVal{val};
		auto after_point = getDecimalDigitCount(value - floor(value));
		char *end = toString((f32)value, outVal, after_point);
		if (end == nullptr)
		{
			return TkString::Empty;
		}
		return TkString{&val[0], end};
		return TkString::format("{}", value);
	}

	TkString TkStringUtils::toString(f64 value)
	{
		char val[21];
		TkSpan<char> outVal{val};
		auto after_point = getDecimalDigitCountD(value - floor(value));
		char *end = toString((f32)value, outVal, after_point);
		if (end == nullptr)
		{
			return TkString::Empty;
		}
		return TkString{&val[0], end};
		return TkString::format("{}", value);
	}

	// returns new end or null if failed
	static char *increment(const char *output, char *end, bool is_space_after)
	{
		char carry = 1;
		{
			char *c = end - 1; // skip '\0'
			while (c >= output)
			{
				if (*c == '.')
				{
					--c;
				}
				*c += carry;
				if (*c > '9')
				{
					*c = '0';
					carry = 1;
				}
				else
				{
					carry = 0;
					break;
				}
				--c;
			}
		}
		if (carry && is_space_after)
		{
			char *c = end; // including '\0' at the end of the String
			while (c >= output)
			{
				*(c + 1) = *c;
				--c;
			}
			++c;
			*c = '1';
			return end + 1;
		}
		if (carry)
			return nullptr;
		return end;
	}

	char *TkStringUtils::toString(f32 value, TkSpan<char> output, int after_point)
	{
		return toString(double(value), output, after_point);
	}

	char *TkStringUtils::toString(bool value, TkSpan<char> output)
	{
		if (output.length() < (value ? 5 : 6)) // FIX: "true" need 5 bytes（include '\0'），"false" need 6
			return nullptr;

		TkStringView str = value ? "true" : "false";
		return copyString(output, str);
	}

	char *TkStringUtils::toString(f64 value, TkSpan<char> out, int after_point)
	{
		char *output = out.begin();
		u32 length = out.length();
		if (length < 2)
			return nullptr;

		if (value < 0)
		{
			*output = '-';
			++output;
			value = -value;
			--length;
		}
		// int part
		int exponent = value == 0 ? 0 : (int)log10(value);
		double num = value;
		char *c = output;
		if (num < 1 && length > 1)
		{
			*c = '0';
			++c;
			--length;
		}
		else
		{
			while ((num >= 1 || exponent >= 0) && length > 1)
			{
				const double power = pow(10.0, (double)exponent);
				char digit = (char)floor(num / power);
				num -= digit * power;
				*c = digit + '0';
				--exponent;
				--length;
				++c;
			}
		}
		// decimal part
		double dec_part = num;
		if (length > 1 && after_point > 0)
		{
			*c = '.';
			++c;
			--length;
		}
		else if (length > 0 && after_point == 0)
		{
			*c = 0;
			return c;
		}
		else
		{
			return nullptr;
		}
		while (length > 1 && after_point > 0)
		{
			dec_part *= 10;
			char tmp = (char)dec_part;
			*c = tmp + '0';
			dec_part -= tmp;
			++c;
			--length;
			--after_point;
		}
		*c = 0;
		if ((int)(dec_part + 0.5f))
			c = increment(output, c, length > 1);
		return c;
	}

	bool TkStringUtils::startsWith(TkStringView str, TkStringView prefix)
	{
		if (str.length() < prefix.length())
			return false;
		TkStringView pfx(str.begin(), prefix.length());
		return equalStrings(pfx, prefix, prefix.length());
	}

	bool TkStringUtils::startsWithInsensitive(TkStringView str, TkStringView prefix)
	{
		if (str.length() < prefix.length())
			return false;
		TkStringView pfx(str.begin(), prefix.length());
		return equalIStrings(pfx, prefix, prefix.length());
	}

	char TkStringUtils::toLower(char c)
	{
		if (c >= 'A' && c <= 'Z')
			return c - 'A' + 'a';
		return c;
	}

	bool TkStringUtils::isWhitespace(char c) { return c == ' ' || c == '\n' || c == '\r' || c == '\t'; }
	bool TkStringUtils::isUpper(char c) { return std::isupper(static_cast<unsigned char>(c)) != 0; }
	bool TkStringUtils::isLower(char c) { return std::islower(static_cast<unsigned char>(c)) != 0; }
	bool TkStringUtils::isAlpha(char c) { return std::isalpha(static_cast<unsigned char>(c)) != 0; }
	bool TkStringUtils::isPunct(char c) { return std::ispunct(static_cast<unsigned char>(c)) != 0; }
	bool TkStringUtils::isAlnum(char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0; }
	bool TkStringUtils::isDigit(char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }
	bool TkStringUtils::isHexDigit(char c) { return std::isxdigit(static_cast<unsigned char>(c)) != 0; }
	char TkStringUtils::toUpper(char c) { return static_cast<char>(std::toupper(static_cast<unsigned char>(c))); }

	bool TkStringUtils::fromHexString(const char *str, i32 length, u32 &result)
	{
		result = 0;

		u32 sum = 0;
		const char *p = str;
		const char *end = str + length;
		if (*p == '0' && *(p + 1) == 'x')
			p += 2;
		while (*p && p < end)
		{
			i32 c = *p - '0';
			if (c < 0 || c > 9)
			{
				c = toLower(*p) - 'a' + 10;
				if (c < 10 || c > 15)
					return false;
			}
			sum = 16 * sum + c;
			p++;
		}
		result = sum;
		return true;
	}

	bool TkStringUtils::fromHexString(const char *str, i32 length)
	{
		u32 sum = 0;
		const char *p = str;
		const char *end = str + length;
		if (*p == '0' && *(p + 1) == 'x')
			p += 2;
		while (*p && p < end)
		{
			i32 c = *p - '0';
			if (c < 0 || c > 9)
			{
				c = toLower(*p) - 'a' + 10;
				if (c < 10 || c > 15)
					return false;
			}
			sum = 16 * sum + c;
			p++;
		}
		return true;
	}

	TkString TkStringUtils::skipZeroToZero(const char *str)
	{
		const char *end = str;
		while (*end != '\0')
		{
			end++;
			if (*end == '\0')
				end++;
		}
		return TkString{str, (i32)(end - str)};
	}

	u32 TkStringUtils::getStringsHash(const char *str)
	{
		u32 hash = 5381;
		char c;
		if (str)
		{
			while ((c = *str++) != 0)
				hash = ((hash << 5) + hash) + (u32)c;
		}
		return hash;
	}

	u32 TkStringUtils::getStringsHash(const char *str, i32 length)
	{
		u32 hash = 5381;
		char c;
		if (str)
		{
			while ((c = *str++) != 0 && length-- > 0)
				hash = ((hash << 5) + hash) + (u32)c;
		}
		return hash;
	}

}
