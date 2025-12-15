// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/utils/StringUtils.hpp>
#include <toolkit/base/Format.hpp>

namespace toolkit {

	// Represents static text view as a sequence of ANSI characters. wcharacters sequence might not be null-terminated
	class TkStringView final
	{
	public:
		static TkStringView Empty;

	public:
		TkStringView(const class TkString& str);
		TkStringView(const char* str)
		{
			m_data = str;
			m_length = TkStringUtils::length(str);
		}
		constexpr TkStringView()
			: m_data(nullptr)
			, m_length(0)
		{
		}
		constexpr TkStringView(const char* data, i32 length)
			: m_data(data)
			, m_length(length)
		{
		}
		constexpr TkStringView(const TkStringView& other)
			: m_data(other.m_data)
			, m_length(other.m_length)
		{
		}


	public:
		TK_FORCE_INLINE TkStringView& operator=(const char* str)
		{
			m_data = str;
			m_length = TkStringUtils::length(str);
			return *this;
		}
		TK_FORCE_INLINE bool operator==(const TkStringView& other) const
		{
			return m_length == other.m_length && (m_length == 0 || TkStringUtils::compareStrings(m_data, other.m_data, m_length) == 0);
		}
		TK_FORCE_INLINE bool operator!=(const TkStringView& other) const
		{
			return !(*this == other);
		}
		TK_FORCE_INLINE bool operator==(const char* other) const
		{
			return *this == TkStringView(other);
		}
		TK_FORCE_INLINE bool operator!=(const char* other) const
		{
			return !(*this == TkStringView(other));
		}
		bool operator==(const TkString& other) const;
		bool operator!=(const TkString& other) const;

	public:
		TK_FORCE_INLINE const char& operator[](i32 index) const
		{
			TK_ASSERT(index >= 0 && index <= m_length);
			return m_data[index];
		}

		TK_FORCE_INLINE TkStringView& operator=(const TkStringView& other)
		{
			if (this != &other)
			{
				m_data = other.m_data;
				m_length = other.m_length;
			}
			return *this;
		}

		i32 compare(const TkStringView& str, bool searchCase = true) const;

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

		bool startsWith(const TkStringView& prefix, bool searchCase = false) const;
		bool endsWith(const TkStringView& suffix, bool searchCase = false) const;

	public:
		// Gets the left most given number of characters.
		TkStringView left(i32 count) const;
		// Gets the string of characters from the right (end of the string).
		TkStringView right(i32 count) const;
		// Retrieves substring created from characters starting from startIndex to the TkString end.
		TkStringView substring(i32 startIndex) const;
		// Retrieves substring created from characters starting from start index.
		TkStringView substring(i32 startIndex, i32 count) const;

	public:
		TkString toString() const;

	private:
		const char* m_data;
		i32 m_length;
	};


	inline u32 getHash(const TkStringView& key)
	{
		return TkStringUtils::getStringsHash(key.get(), key.length());
	}

	bool operator==(const TkString& a, const TkStringView& b);
	bool operator!=(const TkString& a, const TkStringView& b);
}

namespace toolkit {

	template<typename T>
	psize formatCustom(FormatBuffer& b, const TkStringView& v) {
		b.appendData((void*)v.get(), v.length() * sizeof(char));
		return v.length();
	}

}