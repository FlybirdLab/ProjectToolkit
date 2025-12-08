// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {

	class TStringUtils
	{
	public:
		// --- Misc ---
		static int length(const char* str);
		// skip '\0' until found real '\0'
		static TString skipZeroToZero(const char* str);
		static i32 hexDigit(char c);

		static u32 getStringsHash(const char* str);
		static u32 getStringsHash(const char* str, i32 length);

	public:
		// --- Character Utilities ---
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
		// --- Search Functions ---

		static const char* find(const char* str, char toFind);
		static const char* find(TStringView str, char toFind);
		static const char* find(const char* str, const char* toFind);
		static const char* find(TStringView str, TStringView toFind);

		static const char* findInsensitive(const char* str, const char* toFind);
		static const char* findInsensitive(TStringView str, TStringView toFind);

		static const char* reverseFind(TStringView str, char c);
		static bool contains(TStringView str, char toFind);

	public:
		// --- Equality ---
		static bool equalStrings(const char* lhs, const char* rhs);
		static bool equalStrings(const char* lhs, const char* rhs, size_t len);
		static bool equalStrings(TStringView lhs, TStringView rhs);
		static bool equalStrings(TStringView lhs, TStringView rhs, size_t len);

		static bool equalIStrings(const char* lhs, const char* rhs);
		static bool equalIStrings(const char* lhs, const char* rhs, size_t len);
		static bool equalIStrings(TStringView lhs, TStringView rhs);
		static bool equalIStrings(TStringView lhs, TStringView rhs, size_t len);

	public:
		// --- Comparison ---
		static i32 compareStrings(const char* str1, const char* str2);
		static i32 compareStrings(const char* str1, const char* str2, i32 maxCount);
		static i32 compareIStrings(const char* str1, const char* str2);
		static i32 compareIStrings(const char* str1, const char* str2, i32 maxCount);

	public:
		// --- Prefix/Suffix ---
		static bool startsWith(TStringView str, TStringView prefix);
		static bool startsWithInsensitive(TStringView str, TStringView prefix);
		static bool endsWith(TStringView str, TStringView suffix);
		static bool endsWithInsensitive(TStringView str, TStringView suffix);

	public:
		// --- Conversion To TString ---
		static char* toTString(bool value, TSpan<char> output);
		static char* toTString(i32 value, TSpan<char> output);
		static char* toTString(i64 value, TSpan<char> output);
		static char* toTString(u32 value, TSpan<char> output);
		static char* toTString(u64 value, TSpan<char> output);
		static char* toTString(f32 value, TSpan<char> output, int after_point);
		static char* toTString(f64 value, TSpan<char> output, int after_point);

		static void toTStringHex(u8 value, TSpan<char> output);
		static void toTStringPretty(i32 value, TSpan<char> output);
		static void toTStringPretty(u32 value, TSpan<char> output);
		static void toTStringPretty(u64 value, TSpan<char> output);

		static TString toTString(i32 value);
		static TString toTString(i64 value);
		static TString toTString(u32 value);
		static TString toTString(u64 value);
		// TODO need test
		static TString toTString(f32 value);
		static TString toTString(f64 value);

	public:
		// --- Conversion From TString ---
		static bool fromTString(TStringView str, u64& result);
		static bool fromTString(TStringView str, u32& result);
		static bool fromTString(TStringView str, u16& result);
		static bool fromTString(TStringView str, u8& result);
		static bool fromTString(TStringView str, i64& result);
		static bool fromTString(TStringView str, i32& result);
		static bool fromTString(TStringView str, i16& result);
		static bool fromTString(TStringView str, i8& result);
		static bool fromTString(TStringView str, f32& result);
		static bool fromTString(TStringView str, bool& value);

		static bool fromHexString(const char* str, i32 length, u32& result);

		static const char* fromTStringOctal(TStringView str, u32& value);

	public:
		// --- Copy/Concat/Transform ---
		static char* copyString(TSpan<char> output, TStringView source);
		static char* catString(TSpan<char> output, TStringView source);
		static bool makeLowercase(TSpan<char> output, TStringView source);

	};


	TK_FORCE_INLINE bool fromHexString(const char* str, u32& result)
	{
		return TStringUtils::fromHexString(str, TStringUtils::length(str), result);
	}
	// supporting cobalt hashing model
	TK_FORCE_INLINE u32 getHash(const char* key) {
		return TStringUtils::getStringsHash(key);
	}
	TK_FORCE_INLINE u32 getHash(const char* key, i32 length) {
		return TStringUtils::getStringsHash(key, length);
	}

}