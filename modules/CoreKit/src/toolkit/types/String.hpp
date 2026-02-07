/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target: StringTest.cpp
 */

#pragma once

#include <toolkit/base/Format.hpp>
#include <toolkit/utils/StringUtils.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/memory/Memory.hpp>

namespace toolkit
{

	// Represents text as a sequence of UTF8 characters. Container uses a single dynamic memory allocation to store the characters data. wcharacters sequence is always null-terminated.
	class TkString final
	{
	public:
		// Instance of the empty string.
		static TkString Empty;

	public:
		TkString(IAllocator &alloc = TkMemory::getDefault());
		TkString(const TkString &str, IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
			set(str.get(), str.length());
		}
		TkString(TkString &&str, IAllocator &alloc = TkMemory::getDefault()) noexcept
			: m_alloc{alloc}
		{
			m_data = str.m_data;
			m_length = str.m_length;
			str.m_data = nullptr;
			str.m_length = 0;
		}
		TkString(const char *str, IAllocator &alloc = TkMemory::getDefault())
			: TkString(str, TkStringUtils::length(str), alloc)
		{
		}
		TkString(const char *start, const char *end, IAllocator &alloc = TkMemory::getDefault())
			: TkString(start, static_cast<i32>(end - start), alloc)
		{
		}
		explicit TkString(const char *str, i32 length, IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
			set(str, length);
		}
		TkString(const TkStringView &str, IAllocator &alloc = TkMemory::getDefault());
		~TkString()
		{
			if (!m_data) return;
			TkMemory::free(m_alloc, m_data);
			m_data = nullptr;
		}

	public:
		// Clears this instance. Frees the memory and sets the string to empty.
		void clear()
		{
			TkMemory::free(m_alloc, m_data);
			m_data = nullptr;
			m_length = 0;
		}

	public:
		// Gets the character at the specific index.
		TK_FORCE_INLINE char &operator[](i32 index)
		{
			TK_ASSERT(index >= 0 && index < m_length);
			return m_data[index];
		}

		// Gets the character at the specific index.
		TK_FORCE_INLINE const char &operator[](i32 index) const
		{
			TK_ASSERT(index >= 0 && index < m_length);
			return m_data[index];
		}

	public:
		bool startsWith(char c, bool searchCase = true) const;
		bool endsWith(char c, bool searchCase = true) const;

		bool startsWith(TkStringView prefix, bool searchCase = true) const;
		bool endsWith(TkStringView suffix, bool searchCase = true) const;

		bool startsWith(const TkString &prefix, bool searchCase = true) const;
		bool endsWith(const TkString &suffix, bool searchCase = true) const;

		i32 replace(char searchwchar, char replacementwchar, bool searchCase = true);
		i32 replace(const char *searchText, const char *replacementText, bool searchCase = true);
		i32 replace(const char *searchText, i32 searchTextLength, const char *replacementText, i32 replacementTextLength, bool searchCase = true);

		void reverse();
		void resize(i32 length);

		// Lexicographically tests how this string compares to the other given string.
		// In case sensitive mode 'A' is less than 'a'.
		i32 compare(const TkString &str, bool searchCase = true) const;

	public:
		TK_FORCE_INLINE bool isEmpty() const
		{
			return m_length == 0;
		}

		TK_FORCE_INLINE bool hasChars() const
		{
			return m_length != 0;
		}

		TK_FORCE_INLINE i32 length() const
		{
			return m_length;
		}

		TK_FORCE_INLINE const char *operator*() const
		{
			return m_data;
		}
		TK_FORCE_INLINE char *operator*()
		{
			return m_data;
		}
		TK_FORCE_INLINE char *get()
		{
			return m_data;
		}
		TK_FORCE_INLINE const char *get() const
		{
			return m_data;
		}
		TK_FORCE_INLINE const char *getText() const
		{
			return m_data ? m_data : (const char *)"";
		}

	public:
		TK_FORCE_INLINE bool contains(const char *subStr, bool searchCase = true) const
		{
			return find(subStr, searchCase) != -1;
		}
		TK_FORCE_INLINE bool contains(const TkString &subStr, bool searchCase = true) const
		{
			return find(*subStr, searchCase) != -1;
		}

		i32 find(char c) const;
		i32 findLast(char c) const;

		i32 find(const char *subStr, bool searchCase = true, i32 startPosition = -1) const;
		i32 findLast(const char *subStr, bool searchCase = true, i32 startPosition = -1) const;

		TK_FORCE_INLINE i32 find(const TkString &subStr, bool searchCase = true, i32 startPosition = -1) const
		{
			return find(subStr.get(), searchCase, startPosition);
		}
		TK_FORCE_INLINE i32 findLast(const TkString &subStr, bool searchCase = true, i32 startPosition = -1) const
		{
			return findLast(subStr.get(), searchCase, startPosition);
		}

		i32 findFirstOf(char c, i32 startPos = 0) const;
		i32 findFirstOf(const char *str, i32 startPos = 0) const;

		void reserve(i32 length);

	public:
		// Sets an array of characters to the string.
		void set(const char *chars, i32 length);

		// Appends an array of characters to the string.
		void append(const char *chars, i32 count);
		// Appends the specified text to this string.
		TK_FORCE_INLINE TkString &append(const TkString &text)
		{
			append(text.get(), text.length());
			return *this;
		}
		// Appends the specified character to this string.
		TK_FORCE_INLINE TkString &append(const char c)
		{
			TK_ASSERT(c != 0);
			append(&c, 1);
			return *this;
		}

		// Appends the specified text to this string
		TK_FORCE_INLINE TkString &operator+=(const char *str)
		{
			append(str, TkStringUtils::length(str));
			return *this;
		}
		// Appends the specified character to this string.
		TkString &operator+=(const char c)
		{
			TK_ASSERT(c != 0);
			append(&c, 1);
			return *this;
		}
		// Appends the specified text to this string.
		TK_FORCE_INLINE TkString &operator+=(const TkString &str)
		{
			append(str.get(), str.length());
			return *this;
		}

		// Appends the specified text to this string.
		TkString &operator+=(const TkStringView &str);
		// Concatenates a string with a character.
		friend TkString operator+(const TkString &a, const char b)
		{
			TkString result;
			result.m_length = a.length() + 1;
			result.m_data = (char *)TkMemory::allocate(result.m_alloc, (result.m_length + 1) * sizeof(char), 16);
			TkMemory::memoryCopy(result.m_data, a.get(), a.length() * sizeof(char));
			result.m_data[a.length()] = b;
			result.m_data[result.m_length] = 0;
			return result;
		}

	public:
		TK_FORCE_INLINE TkString &operator=(TkString &&s) noexcept
		{
			if (this != &s)
			{
				TkMemory::free(m_alloc, m_data);
				m_data = s.m_data;
				m_length = s.m_length;
				s.m_data = nullptr;
				s.m_length = 0;
			}
			return *this;
		}
		TK_FORCE_INLINE TkString &operator=(const TkString &s)
		{
			if (this != &s)
				set(s.get(), s.length());
			return *this;
		}
		TkString &operator=(const TkStringView &s);
		TkString &operator=(const char *str)
		{
			if (m_data != str)
				set(str, TkStringUtils::length(str));
			return *this;
		}
		TkString &operator=(const char c)
		{
			set(&c, 1);
			return *this;
		}

	public:
		TK_FORCE_INLINE friend TkString operator+(const TkString &a, const TkString &b)
		{
			return concatStrings<const TkString &, const TkString &>(a, b);
		}
		TK_FORCE_INLINE friend TkString operator+(TkString &&a, const TkString &b)
		{
			return concatStrings(moveTemp(a), b);
		}
		TK_FORCE_INLINE friend TkString operator+(const TkString &a, TkString &&b)
		{
			return concatStrings<const TkString &, TkString &&>(a, moveTemp(b));
		}
		TK_FORCE_INLINE friend TkString operator+(TkString &&a, TkString &&b)
		{
			return concatStrings<TkString &&, TkString &&>(moveTemp(a), moveTemp(b));
		}
		TK_FORCE_INLINE friend TkString operator+(const char *a, const TkString &b)
		{
			return concatCharsToString<const TkString &>(a, b);
		}
		TK_FORCE_INLINE friend TkString operator+(const char *a, TkString &&b)
		{
			return concatCharsToString<TkString &&>(a, moveTemp(b));
		}
		TK_FORCE_INLINE friend TkString operator+(const TkString &a, const char *b)
		{
			return concatStringToChars<const TkString &>(a, b);
		}
		TK_FORCE_INLINE friend TkString operator+(TkString &&a, const char *b)
		{
			return concatStringToChars<TkString &&>(moveTemp(a), b);
		}
		TK_FORCE_INLINE bool operator<=(const char *other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other) <= 0;
		}
		TK_FORCE_INLINE bool operator<(const char *other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other) < 0;
		}
		TK_FORCE_INLINE bool operator<(const TkString &other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other.getText()) < 0;
		}
		TK_FORCE_INLINE bool operator>=(const char *other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other) >= 0;
		}
		TK_FORCE_INLINE bool operator>(const char *other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other) > 0;
		}
		TK_FORCE_INLINE bool operator>(const TkString &other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other.getText()) > 0;
		}
		TK_FORCE_INLINE bool operator==(const char *other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other) == 0;
		}
		TK_FORCE_INLINE bool operator==(const TkString &other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other.getText()) == 0;
		}
		TK_FORCE_INLINE bool operator!=(const char *other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other) != 0;
		}
		TK_FORCE_INLINE bool operator!=(const TkString &other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other.getText()) != 0;
		}
		TK_FORCE_INLINE bool operator<=(const TkString &other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other.getText()) <= 0;
		}
		TK_FORCE_INLINE bool operator>=(const TkString &other) const
		{
			return TkStringUtils::compareStrings(this->getText(), other.getText()) >= 0;
		}

	public:
		TkString &operator/=(const char *str);
		TkString &operator/=(char c);
		TK_FORCE_INLINE TkString &operator/=(const TkString &str)
		{
			return operator/=(*str);
		}
		TkString &operator/=(const TkStringView &str);
		TK_FORCE_INLINE TkString operator/(const char *str) const
		{
			return TkString(*this) /= str;
		}
		TK_FORCE_INLINE TkString operator/(const char c) const
		{
			return TkString(*this) /= c;
		}
		TK_FORCE_INLINE TkString operator/(const TkString &str) const
		{
			return TkString(*this) /= str;
		}
		TK_FORCE_INLINE TkString operator/(const TkStringView &str) const
		{
			return TkString(*this) /= str;
		}

	public:
		TkString toLowerAscii() const;
		TkString toUpperAscii() const;

		TkString toLower() const;
		TkString toUpper() const;
		TkString caseFold() const;
		bool equalsIgnoreCase(const TkStringView &) const;

		TkString trimmed() const;
		TkString simplified() const;

		TK_FORCE_INLINE TkString left(i32 count) const
		{
			const i32 countClamped = count < 0 ? 0 : count < length() ? count
																	  : length();
			return TkString(**this, countClamped);
		}
		TK_FORCE_INLINE TkString right(i32 count) const
		{
			const i32 countClamped = count < 0 ? 0 : count < length() ? count
																	  : length();
			return TkString(**this + length() - countClamped);
		}

		TkString substring(i32 startIndex) const
		{
			TK_ASSERT(startIndex >= 0 && startIndex < length());
			return TkString(m_data + startIndex, m_length - startIndex);
		}
		TkString substring(i32 startIndex, i32 count) const
		{
			TK_ASSERT(startIndex >= 0 && startIndex + count <= length() && count >= 0);
			return TkString(m_data + startIndex, count);
		}

		// Inserts string into current string instance at given location.
		void insert(i32 startIndex, const TkString &other);
		// Removes characters from the string at given location and length.
		void remove(i32 startIndex, i32 length);
		// Splits a string into subTkStringUtils that are based on the character.
		void split(char c, TkArray<TkString> &results) const;

	public:
		// Formats the message and gets it as a string.
		template <typename... Args>
		static TkString format(const char *format, const Args &...args)
		{
			FormatBuffer buf{TkMemory::getDefault()}; // TODO allow customize later
			Formatter::format(buf, format, args...);
			return TkString(buf.getPointer(), buf.getUsed(), TkMemory::getDefault());
		}

	public:
		TkString toString() const
		{
			return TkString(get(), length());
		}

	private:
		template <typename T1, typename T2>
		static TkString concatStrings(T1 left, T2 right)
		{
			if (left.isEmpty())
				return moveTemp(right);
			if (right.isEmpty())
				return moveTemp(left);

			const char *leftStr = left.get();
			const i32 leftLen = left.length();
			const char *rightStr = right.get();
			const i32 rightLen = right.length();

			TkString result;
			result.reserve(leftLen + rightLen);
			TkMemory::memoryCopy(result.get(), leftStr, leftLen * sizeof(char));
			TkMemory::memoryCopy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}

		template <typename T>
		static TkString concatCharsToString(const char *left, T right)
		{
			if (!left || !*left)
				return moveTemp(right);

			const char *leftStr = left;
			const i32 leftLen = TkStringUtils::length(left);
			const char *rightStr = right.get();
			const i32 rightLen = right.length();

			TkString result;
			result.reserve(leftLen + rightLen);
			TkMemory::memoryCopy(result.get(), leftStr, leftLen * sizeof(char));
			TkMemory::memoryCopy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}

		template <typename T>
		static TkString concatStringToChars(T left, const char *right)
		{
			if (!right || !*right)
				return moveTemp(left);

			const char *leftStr = left.get();
			const i32 leftLen = left.length();
			const char *rightStr = right;
			const i32 rightLen = TkStringUtils::length(right);

			TkString result;
			result.reserve(leftLen + rightLen);
			TkMemory::memoryCopy(result.get(), leftStr, leftLen * sizeof(char));
			TkMemory::memoryCopy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}

	private:
		IAllocator &m_alloc;
		char *m_data = nullptr;
		i32 m_length = 0;
	};

	template <>
	struct HashFunc<TkString>
	{
		static u32 get(const TkString &key)
		{
			return TkStringUtils::getStringsHash(key.get());
		}
	};

	template <>
	inline psize formatCustom(FormatBuffer &b, const TkString &v)
	{
		b.appendCharData(v.get(), v.length() * sizeof(char));
		return v.length();
	}

}