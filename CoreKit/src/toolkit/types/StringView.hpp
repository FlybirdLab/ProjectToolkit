// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/utils/StringUtils.hpp>
#include <toolkit/base/Format.hpp>

namespace toolkit {

	// Represents static text view as a sequence of ANSI characters. wcharacters sequence might not be null-terminated
	class TStringView final
	{
	public:
		static TStringView Empty;

	public:
		TStringView(const class TString& str);
		TStringView(const char* str)
		{
			m_data = str;
			m_length = TStringUtils::length(str);
		}
		constexpr TStringView()
			: m_data(nullptr)
			, m_length(0)
		{
		}
		constexpr TStringView(const char* data, i32 length)
			: m_data(data)
			, m_length(length)
		{
		}
		constexpr TStringView(const TStringView& other)
			: m_data(other.m_data)
			, m_length(other.m_length)
		{
		}


	public:
		TK_FORCE_INLINE TStringView& operator=(const char* str)
		{
			m_data = str;
			m_length = TStringUtils::length(str);
			return *this;
		}
		TK_FORCE_INLINE bool operator==(const TStringView& other) const
		{
			return m_length == other.m_length && (m_length == 0 || TStringUtils::compareStrings(m_data, other.m_data, m_length) == 0);
		}
		TK_FORCE_INLINE bool operator!=(const TStringView& other) const
		{
			return !(*this == other);
		}
		TK_FORCE_INLINE bool operator==(const char* other) const
		{
			return *this == TStringView(other);
		}
		TK_FORCE_INLINE bool operator!=(const char* other) const
		{
			return !(*this == TStringView(other));
		}
		bool operator==(const TString& other) const;
		bool operator!=(const TString& other) const;

	public:
		TK_FORCE_INLINE const char& operator[](i32 index) const
		{
			TK_ASSERT(index >= 0 && index <= m_length);
			return m_data[index];
		}

		TK_FORCE_INLINE TStringView& operator=(const TStringView& other)
		{
			if (this != &other)
			{
				m_data = other.m_data;
				m_length = other.m_length;
			}
			return *this;
		}

		i32 compare(const TStringView& str, bool searchCase = true) const;

	public:
		TK_FORCE_INLINE bool isEmpty() const
		{
			return m_length == 0;
		}

		TK_FORCE_INLINE bool hasChars() const
		{
			return m_length != 0;
		}

		TK_FORCE_INLINE constexpr i32 length() const
		{
			return m_length;
		}

		TK_FORCE_INLINE constexpr const char* operator*() const
		{
			return m_data;
		}

		TK_FORCE_INLINE constexpr const char* get() const
		{
			return m_data;
		}

		TK_FORCE_INLINE const char* getText() const
		{
			return m_data ? m_data : (const char*)"";
		}

		TK_FORCE_INLINE const char* begin() const {
			return m_data ? m_data : "";
		}
		TK_FORCE_INLINE const char* end() const {
			return m_data ? m_data + m_length * sizeof(char) : "";
		}

	public:
		// Searches the string for the occurrence of a character.
		i32 find(char c) const;
		// Searches the string for the last occurrence of a character.
		i32 findLast(char c) const;

		bool startsWith(char c, bool searchCase = false) const;
		bool endsWith(char c, bool searchCase = false) const;

		bool startsWith(const TStringView& prefix, bool searchCase = false) const;
		bool endsWith(const TStringView& suffix, bool searchCase = false) const;

	public:
		// Gets the left most given number of characters.
		TStringView left(i32 count) const;
		// Gets the string of characters from the right (end of the string).
		TStringView right(i32 count) const;
		// Retrieves substring created from characters starting from startIndex to the TString end.
		TStringView substring(i32 startIndex) const;
		// Retrieves substring created from characters starting from start index.
		TStringView substring(i32 startIndex, i32 count) const;

	public:
		TString toString() const;

	private:
		const char* m_data;
		i32 m_length;
	};


	inline u32 getHash(const TStringView& key)
	{
		return TStringUtils::getStringsHash(key.get(), key.length());
	}

	bool operator==(const TString& a, const TStringView& b);
	bool operator!=(const TString& a, const TStringView& b);
}

namespace toolkit {

	template<typename T>
	psize formatCustom(FormatBuffer& b, const TStringView& v) {
		b.appendData((void*)v.get(), v.length() * sizeof(char));
		return v.length();
	}

}