/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 * Test target: StringUtilsTest.cpp
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Hash.hpp>

namespace toolkit
{

	class TkStringUtils
	{
	public:
		static i32 length(const char *str);
		static i32 length(const char8_t *str);
		static i32 length(const wchar *str);
		static TkString skipZeroToZero(const char *str);
		static i32 hexDigit(char c);

		static u32 getStringsHash(const char *str);
		static u32 getStringsHash(const char *str, i32 length);

	public:
		// Character Utils
		static char toLower(char c);
		static char toUpper(char c);

		static bool isUpper(char c);
		static bool isLower(char c);
		static bool isAlpha(char c);
		static bool isPunct(char c);
		static bool isAlnum(char c);
		static bool isDigit(char c);
		static bool isWhitespace(char c);
		static bool isHexDigit(char c);

	public:
		// Search Functions
		static const char *find(const char *str, char toFind);
		static const char *find(TkStringView str, char toFind);
		static const char *find(const char *str, const char *toFind);
		static const char *find(TkStringView str, TkStringView toFind);

		static const char *findInsensitive(const char *str, const char *toFind);
		static const char *findInsensitive(TkStringView str, TkStringView toFind);

		static const char *reverseFind(TkStringView str, char c);
		static bool contains(TkStringView str, char toFind);

	public:
		// Equality
		static bool equalStrings(const char *lhs, const char *rhs);
		static bool equalStrings(const char *lhs, const char *rhs, size_t len);
		static bool equalStrings(TkStringView lhs, TkStringView rhs);
		static bool equalStrings(TkStringView lhs, TkStringView rhs, size_t len);

		static bool equalIStrings(const char *lhs, const char *rhs);
		static bool equalIStrings(const char *lhs, const char *rhs, size_t len);
		static bool equalIStrings(TkStringView lhs, TkStringView rhs);
		static bool equalIStrings(TkStringView lhs, TkStringView rhs, size_t len);

	public:
		// Comparison
		static i32 compareStrings(const char *str1, const char *str2);
		static i32 compareStrings(const char *str1, const char *str2, i32 maxCount);
		static i32 compareIStrings(const char *str1, const char *str2);
		static i32 compareIStrings(const char *str1, const char *str2, i32 maxCount);

	public:
		// Prefix/Suffix
		static bool startsWith(TkStringView str, TkStringView prefix);
		static bool startsWithInsensitive(TkStringView str, TkStringView prefix);
		static bool endsWith(TkStringView str, TkStringView suffix);
		static bool endsWithInsensitive(TkStringView str, TkStringView suffix);

	public:
		// Conversion To TkString
		static char *toString(bool value, TkSpan<char> output);
		static char *toString(i32 value, TkSpan<char> output);
		static char *toString(i64 value, TkSpan<char> output);
		static char *toString(u32 value, TkSpan<char> output);
		static char *toString(u64 value, TkSpan<char> output);
		static char *toString(f32 value, TkSpan<char> output, int after_point);
		static char *toString(f64 value, TkSpan<char> output, int after_point);

		static void toStringHex(u8 value, TkSpan<char> output);
		static void toStringPretty(i32 value, TkSpan<char> output);
		static void toStringPretty(u32 value, TkSpan<char> output);
		static void toStringPretty(u64 value, TkSpan<char> output);

		static TkString toString(i32 value);
		static TkString toString(i64 value);
		static TkString toString(u32 value);
		static TkString toString(u64 value);
		static TkString toString(f32 value);
		static TkString toString(f64 value);

	public:
		// Conversion From TkStringView
		static bool fromString(TkStringView str, u64 &result);
		static bool fromString(TkStringView str, u32 &result);
		static bool fromString(TkStringView str, u16 &result);
		static bool fromString(TkStringView str, u8 &result);
		static bool fromString(TkStringView str, i64 &result);
		static bool fromString(TkStringView str, i32 &result);
		static bool fromString(TkStringView str, i16 &result);
		static bool fromString(TkStringView str, i8 &result);
		static bool fromString(TkStringView str, f32 &result);
		static bool fromString(TkStringView str, bool &value);

		static bool fromHexString(const char *str, i32 length, u32 &result);
		static bool fromHexString(const char *str, i32 length);

		static const char *fromStringOctal(TkStringView str, u32 &value);

	public:
		// Copy/Concat/Transform
		static char *copyString(TkSpan<char> output, TkStringView source);
		static char *catString(TkSpan<char> output, TkStringView source);
		static bool makeLowercase(TkSpan<char> output, TkStringView source);
	};

	TK_FORCE_INLINE bool fromHexString(const char *str, u32 &result)
	{
		return TkStringUtils::fromHexString(str, TkStringUtils::length(str), result);
	}

	// supporting toolkit hashing model
	template <>
	struct HashFunc<char *>
	{
		static u32 get(const char *k)
		{
			return TkStringUtils::getStringsHash(k);
		}
		static u32 get(const char *k, i32 length)
		{
			return TkStringUtils::getStringsHash(k, length);
		}
	};

}