// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/base/Format.hpp>
#include <toolkit/memory/ContainerAllocation.hpp>
#include <toolkit/utils/StringUtils.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/memory/Memory.hpp>

namespace toolkit {

	// Represents text as a sequence of UTF8 characters. Container uses a single dynamic memory allocation to store the characters data. wcharacters sequence is always null-terminated.
	class TString final
	{
	public:
		// Instance of the empty string.
		static TString Empty;

	public:
		TString(IAllocator& alloc = TMemory::getDefault());
		TString(const TString& str, IAllocator& alloc = TMemory::getDefault())
		    : m_alloc {alloc}
		{
			set(str.get(), str.length());
		}
		TString(TString&& str, IAllocator& alloc = TMemory::getDefault()) noexcept
			: m_alloc {alloc}
		{
			m_data = str.m_data;
			m_length = str.m_length;
			str.m_data = nullptr;
			str.m_length = 0;
		}
		TString(const char* str, IAllocator& alloc = TMemory::getDefault())
			: TString(str, TStringUtils::length(str), alloc)
		{
		}
		TString(const char* start, const char* end, IAllocator& alloc = TMemory::getDefault())
			: TString(start, static_cast<i32>(end - start), alloc)
		{
		}
		explicit TString(const char* str, i32 length, IAllocator& alloc = TMemory::getDefault())
			: m_alloc {alloc}
		{
			set(str, length);
		}
		TString(const TStringView& str, IAllocator& alloc = TMemory::getDefault());
		~TString()
		{
			TMemory::free(m_alloc, m_data);
		}

	public:
		// Clears this instance. Frees the memory and sets the string to empty.
		void clear()
		{
			TMemory::free(m_alloc, m_data);
			m_data = nullptr;
			m_length = 0;
		}

	public:
		// Gets the character at the specific index.
		TK_FORCE_INLINE char& operator[](i32 index)
		{
			TK_ASSERT(index >= 0 && index < m_length);
			return m_data[index];
		}


		// Gets the character at the specific index.
		TK_FORCE_INLINE const char& operator[](i32 index) const
		{
			TK_ASSERT(index >= 0 && index < m_length);
			return m_data[index];
		}

	public:
		bool startsWith(char c, bool searchCase = true) const;
		bool endsWith(char c, bool searchCase = true) const;

		bool startsWith(const TString& prefix, bool searchCase = true) const;
		bool endsWith(const TString& suffix, bool searchCase = true) const;

		bool startsWith(const TStringView& prefix, bool searchCase = true) const;
		bool endsWith(const TStringView& suffix, bool searchCase = true) const;

		i32 replace(char searchwchar, char replacementwchar, bool searchCase = true);
		i32 replace(const char* searchText, const char* replacementText, bool searchCase = true);
		i32 replace(const char* searchText, i32 searchTextLength, const char* replacementText, i32 replacementTextLength, bool searchCase = true);

		void reverse();
		void resize(i32 length);

		// Lexicographically tests how this string compares to the other given string.
		// In case sensitive mode 'A' is less than 'a'.
		i32 compare(const TString& str, bool searchCase = true) const;

	public:
		// Returns true if string is empty.
		TK_FORCE_INLINE bool isEmpty() const
		{
			return m_length == 0;
		}

		// Returns true if string isn't empty.
		TK_FORCE_INLINE bool hasChars() const
		{
			return m_length != 0;
		}

		// Gets the length of the string.
		TK_FORCE_INLINE i32 length() const
		{
			return m_length;
		}

		TK_FORCE_INLINE const char* operator*() const
		{
			return m_data;
		}
		TK_FORCE_INLINE char* operator*()
		{
			return m_data;
		}
		TK_FORCE_INLINE char* get()
		{
			return m_data;
		}
		TK_FORCE_INLINE const char* get() const
		{
			return m_data;
		}
		TK_FORCE_INLINE const char* getText() const
		{
			return m_data ? m_data : (const char*)"";
		}

	public:
		TK_FORCE_INLINE bool contains(const char* subStr, bool searchCase = true) const
		{
			return find(subStr, searchCase) != -1;
		}
		TK_FORCE_INLINE bool contains(const TString& subStr, bool searchCase = true) const
		{
			return find(*subStr, searchCase) != -1;
		}

		i32 find(char c) const;
		i32 findLast(char c) const;

		i32 find(const char* subStr, bool searchCase = true, i32 startPosition = -1) const;
		i32 findLast(const char* subStr, bool searchCase = true, i32 startPosition = -1) const;

		TK_FORCE_INLINE i32 find(const TString& subStr, bool searchCase = true, i32 startPosition = -1) const
		{
			return find(subStr.get(), searchCase, startPosition);
		}
		TK_FORCE_INLINE i32 findLast(const TString& subStr, bool searchCase = true, i32 startPosition = -1) const
		{
			return findLast(subStr.get(), searchCase, startPosition);
		}

		i32 findFirstOf(char c, i32 startPos = 0) const;
		i32 findFirstOf(const char* str, i32 startPos = 0) const;

		void reserve(i32 length);

	public:
		// Sets an array of characters to the string.
		void set(const char* chars, i32 length);

		// Appends an array of characters to the string.
		void append(const char* chars, i32 count);
		// Appends the specified text to this string.
		TK_FORCE_INLINE TString& append(const TString& text)
		{
			append(text.get(), text.length());
			return *this;
		}
		// Appends the specified character to this string.
		TK_FORCE_INLINE TString& append(const char c)
		{
			TK_ASSERT(c != 0);
			append(&c, 1);
			return *this;
		}


		// Appends the specified text to this string
		TK_FORCE_INLINE TString& operator+=(const char* str)
		{
			append(str, TStringUtils::length(str));
			return *this;
		}
		// Appends the specified character to this string.
		TString& operator+=(const char c)
		{
			TK_ASSERT(c != 0);
			append(&c, 1);
			return *this;
		}
		// Appends the specified text to this string.
		TK_FORCE_INLINE TString& operator+=(const TString& str)
		{
			append(str.get(), str.length());
			return *this;
		}


		// Appends the specified text to this string.
		TString& operator+=(const TStringView& str);
		// Concatenates a string with a character.
		friend TString operator+(const TString& a, const char b)
		{
			TString result;
			result.m_length = a.length() + 1;
			result.m_data = (char*)TMemory::allocate(result.m_alloc, (result.m_length + 1) * sizeof(char), 16);
			memcpy(result.m_data, a.get(), a.length() * sizeof(char));
			result.m_data[a.length()] = b;
			result.m_data[result.m_length] = 0;
			return result;
		}

	public:
		TK_FORCE_INLINE TString& operator=(TString&& s) noexcept
		{
			if (this != &s)
			{
				TMemory::free(m_alloc, m_data);
				m_data = s.m_data;
				m_length = s.m_length;
				s.m_data = nullptr;
				s.m_length = 0;
			}
			return *this;
		}
		TK_FORCE_INLINE TString& operator=(const TString& s)
		{
			if (this != &s)
				set(s.get(), s.length());
			return *this;
		}
		TString& operator=(const TStringView& s);
		TString& operator=(const char* str)
		{
			if (m_data != str)
				set(str, TStringUtils::length(str));
			return *this;
		}
		TString& operator=(const char c)
		{
			set(&c, 1);
			return *this;
		}

	public:
		TK_FORCE_INLINE friend TString operator+(const TString& a, const TString& b)
		{
			return concatTStrings<const TString&, const TString&>(a, b);
		}
		TK_FORCE_INLINE friend TString operator+(TString&& a, const TString& b)
		{
			return concatTStrings(moveTemp(a), b);
		}
		TK_FORCE_INLINE friend TString operator+(const TString& a, TString&& b)
		{
			return concatTStrings<const TString&, TString&&>(a, moveTemp(b));
		}
		TK_FORCE_INLINE friend TString operator+(TString&& a, TString&& b)
		{
			return concatTStrings<TString&&, TString&&>(moveTemp(a), moveTemp(b));
		}
		TK_FORCE_INLINE friend TString operator+(const char* a, const TString& b)
		{
			return concatwcharstoString<const TString&>(a, b);
		}
		TK_FORCE_INLINE friend TString operator+(const char* a, TString&& b)
		{
			return concatwcharstoString<TString&&>(a, moveTemp(b));
		}
		TK_FORCE_INLINE friend TString operator+(const TString& a, const char* b)
		{
			return concatTStringTowchars<const TString&>(a, b);
		}
		TK_FORCE_INLINE friend TString operator+(TString&& a, const char* b)
		{
			return concatTStringTowchars<TString&&>(moveTemp(a), b);
		}
		TK_FORCE_INLINE bool operator<=(const char* other) const
		{
			return TStringUtils::compareStrings(this->getText(), other) <= 0;
		}
		TK_FORCE_INLINE bool operator<(const char* other) const
		{
			return TStringUtils::compareStrings(this->getText(), other) < 0;
		}
		TK_FORCE_INLINE bool operator<(const TString& other) const
		{
			return TStringUtils::compareStrings(this->getText(), other.getText()) < 0;
		}
		TK_FORCE_INLINE bool operator>=(const char* other) const
		{
			return TStringUtils::compareStrings(this->getText(), other) >= 0;
		}
		TK_FORCE_INLINE bool operator>(const char* other) const
		{
			return TStringUtils::compareStrings(this->getText(), other) > 0;
		}
		TK_FORCE_INLINE bool operator>(const TString& other) const
		{
			return TStringUtils::compareStrings(this->getText(), other.getText()) > 0;
		}
		TK_FORCE_INLINE bool operator==(const char* other) const
		{
			return TStringUtils::compareStrings(this->getText(), other) == 0;
		}
		TK_FORCE_INLINE bool operator==(const TString& other) const
		{
			return TStringUtils::compareStrings(this->getText(), other.getText()) == 0;
		}
		TK_FORCE_INLINE bool operator!=(const char* other) const
		{
			return TStringUtils::compareStrings(this->getText(), other) != 0;
		}
		TK_FORCE_INLINE bool operator!=(const TString& other) const
		{
			return TStringUtils::compareStrings(this->getText(), other.getText()) != 0;
		}
		TK_FORCE_INLINE bool operator<=(const TString& other) const
		{
			return TStringUtils::compareStrings(this->getText(), other.getText()) <= 0;
		}
		TK_FORCE_INLINE bool operator>=(const TString& other) const
		{
			return TStringUtils::compareStrings(this->getText(), other.getText()) >= 0;
		}

	public:
		TString& operator/=(const char* str);
		TString& operator/=(char c);
		TK_FORCE_INLINE TString& operator/=(const TString& str)
		{
			return operator/=(*str);
		}
		TString& operator/=(const TStringView& str);
		TK_FORCE_INLINE TString operator/(const char* str) const
		{
			return TString(*this) /= str;
		}
		TK_FORCE_INLINE TString operator/(const char c) const
		{
			return TString(*this) /= c;
		}
		TK_FORCE_INLINE TString operator/(const TString& str) const
		{
			return TString(*this) /= str;
		}
		TK_FORCE_INLINE TString operator/(const TStringView& str) const
		{
			return TString(*this) /= str;
		}

	public:
		TString toLower() const;
		TString toUpper() const;


		TK_FORCE_INLINE TString left(i32 count) const
		{
			const i32 countClamped = count < 0 ? 0 : count < length() ? count : length();
			return TString(**this, countClamped);
		}
		TK_FORCE_INLINE TString right(i32 count) const
		{
			const i32 countClamped = count < 0 ? 0 : count < length() ? count : length();
			return TString(**this + length() - countClamped);
		}

		TString substring(i32 startIndex) const
		{
			TK_ASSERT(startIndex >= 0 && startIndex < length());
			return TString(m_data + startIndex, m_length - startIndex);
		}
		TString substring(i32 startIndex, i32 count) const
		{
			TK_ASSERT(startIndex >= 0 && startIndex + count <= length() && count >= 0);
			return TString(m_data + startIndex, count);
		}


		// Inserts string into current string instance at given location.
		void insert(i32 startIndex, const TString& other);
		// Removes characters from the string at given location and length.
		void remove(i32 startIndex, i32 length);
		// Splits a string into subTStringUtils that are based on the character.
		void split(char c, TArray<TString, HeapAllocation>& results) const;

	public:
		// Formats the message and gets it as a string.
		template<typename... Args>
		static TString format(const char* format, const Args& ... args)
		{
			FormatBuffer buf {TMemory::getDefault()}; //TODO allow customize later
			Formatter::format(buf, format, args...);
			return TString(buf.getPointer(), buf.getUsed());
		}

	public:
		TString toString() const
		{
			return TString(get(), length());
		}

	private:
		template<typename T1, typename T2>
		static TString concatTStrings(T1 left, T2 right)
		{
			if (left.isEmpty())
				return moveTemp(right);
			if (right.isEmpty())
				return moveTemp(left);

			const char* leftStr = left.get();
			const i32 leftLen = left.length();
			const char* rightStr = right.get();
			const i32 rightLen = right.length();

			TString result;
			result.reserve(leftLen + rightLen);
			memcpy(result.get(), leftStr, leftLen * sizeof(char));
			memcpy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}


		template<typename T>
		static TString concatwcharstoString(const char* left, T right)
		{
			if (!left || !*left)
				return moveTemp(right);

			const char* leftStr = left;
			const i32 leftLen = TStringUtils::length(left);
			const char* rightStr = right.get();
			const i32 rightLen = right.length();

			TString result;
			result.reserve(leftLen + rightLen);
			memcpy(result.get(), leftStr, leftLen * sizeof(char));
			memcpy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}


		template<typename T>
		static TString concatTStringTowchars(T left, const char* right)
		{
			if (!right || !*right)
				return moveTemp(left);

			const char* leftStr = left.get();
			const i32 leftLen = left.length();
			const char* rightStr = right;
			const i32 rightLen = TStringUtils::length(right);

			TString result;
			result.reserve(leftLen + rightLen);
			memcpy(result.get(), leftStr, leftLen * sizeof(char));
			memcpy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}


	private:
		IAllocator& m_alloc;
		char* m_data = nullptr;
		i32 m_length = 0;
	};


	inline u32 getHash(const TString& key)
	{
		return TStringUtils::getStringsHash(key.get());
	}

}


namespace toolkit {

	template<typename T>
	psize formatCustom(FormatBuffer& b, const TString& v) {
		b.appendData((void*)v.get(), v.length() * sizeof(char));
		return v.length();
	}

}