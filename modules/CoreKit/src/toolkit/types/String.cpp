/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "String.hpp"
#include "StringView.hpp"

#include "toolkit/collection/Array.hpp"

namespace toolkit
{

	TkString TkString::Empty{};
	TkString::TkString(IAllocator &alloc)
		: m_alloc{alloc}
	{
	}
	TkString::TkString(const TkStringView &str, IAllocator &alloc)
		: m_alloc{alloc}
	{
		set(str.get(), str.length());
	}

	TkString &TkString::operator/=(const char *str)
	{
		const i32 length = m_length;
		if (length > 0 && m_data[length - 1] != '/' && m_data[length - 1] != '\\' && (str == nullptr || (str[0] != '/' && str[0] != '\\')))
		{
			*this += '/';
		}
		return *this += str;
	}

	TkString &TkString::operator/=(const char c)
	{
		const i32 length = m_length;
		if (length > 0 && m_data[length - 1] != '/' && m_data[length - 1] != '\\')
		{
			*this += '/';
		}
		return *this += c;
	}

	TkString &TkString::operator/=(const TkStringView &str)
	{
		const i32 length = m_length;
		if (length > 1 && m_data[length - 1] != '/' && m_data[length - 1] != '\\' && (str == nullptr || (str[0] != '/' && str[0] != '\\')))
		{
			*this += '/';
		}
		return *this += str;
	}

	bool TkString::startsWith(char c, bool searchCase) const
	{
		const i32 length = TkString::length();
		if (searchCase == true)
			return length > 0 && m_data[0] == c;
		return length > 0 && TkStringUtils::toLower(m_data[0]) == TkStringUtils::toLower(c);
	}

	bool TkString::endsWith(char c, bool searchCase) const
	{
		const i32 length = TkString::length();
		if (searchCase == true)
			return length > 0 && m_data[length - 1] == c;
		return length > 0 && TkStringUtils::toLower(m_data[length - 1]) == TkStringUtils::toLower(c);
	}

	bool TkString::startsWith(const TkString &prefix, bool searchCase) const
	{
		if (prefix.isEmpty())
			return true;
		if (length() < prefix.length())
			return false;
		if (searchCase == false)
			return TkStringUtils::equalIStrings(this->getText(), *prefix, prefix.length()) == 0;
		return TkStringUtils::equalStrings(this->getText(), *prefix, prefix.length()) == 0;
	}

	bool TkString::endsWith(const TkString &suffix, bool searchCase) const
	{
		if (suffix.isEmpty())
			return true;
		if (length() < suffix.length())
			return false;
		if (!searchCase)
			return TkStringUtils::equalIStrings(&(*this)[TkString::length() - suffix.length()], *suffix);
		return TkStringUtils::equalStrings(&(*this)[length() - suffix.length()], *suffix);
	}

	i32 TkString::replace(char searchwchar, char replacementwchar, bool searchCase)
	{
		i32 replacedwchars = 0;
		const i32 length = TkString::length();
		if (searchCase == false)
		{
			const char toCompare = TkStringUtils::toLower(searchwchar);
			for (i32 i = 0; i < length; i++)
			{
				if (TkStringUtils::toLower(m_data[i]) == toCompare)
				{
					m_data[i] = replacementwchar;
					replacedwchars++;
				}
			}
		}
		else
		{
			for (i32 i = 0; i < length; i++)
			{
				if (m_data[i] == searchwchar)
				{
					m_data[i] = replacementwchar;
					replacedwchars++;
				}
			}
		}
		return replacedwchars;
	}

	i32 TkString::replace(const char *searchText, const char *replacementText, bool searchCase)
	{
		const i32 searchTextLength = TkStringUtils::length(searchText);
		const i32 replacementTextLength = TkStringUtils::length(replacementText);
		return replace(searchText, searchTextLength, replacementText, replacementTextLength, searchCase);
	}

	i32 TkString::replace(const char *searchText, i32 searchTextLength, const char *replacementText, i32 replacementTextLength, bool searchCase)
	{
		if (!hasChars() || searchTextLength == 0)
			return 0;

		i32 replacedCount = 0;

		if (searchTextLength == replacementTextLength)
		{
			char *pos = (char *)(searchCase == false ? TkStringUtils::findInsensitive(m_data, searchText) : TkStringUtils::find(m_data, searchText));
			while (pos != nullptr)
			{
				replacedCount++;

				for (i32 i = 0; i < replacementTextLength; i++)
					pos[i] = replacementText[i];

				if (pos + searchTextLength - **this < length())
					pos = (char *)(searchCase == false ? TkStringUtils::findInsensitive(pos + searchTextLength, searchText) : TkStringUtils::find(pos + searchTextLength, searchText));
				else
					break;
			}
		}
		else if (contains(searchText, searchCase))
		{
			char *readPosition = m_data;
			char *searchPosition = (char *)(searchCase == false ? TkStringUtils::findInsensitive(readPosition, searchText) : TkStringUtils::find(readPosition, searchText));
			while (searchPosition != nullptr)
			{
				replacedCount++;
				readPosition = searchPosition + searchTextLength;
				searchPosition = (char *)(searchCase == false ? TkStringUtils::findInsensitive(readPosition, searchText) : TkStringUtils::find(readPosition, searchText));
			}

			const auto oldLength = m_length;
			const auto oldData = m_data;
			m_length += replacedCount * (replacementTextLength - searchTextLength);
			m_data = (char *)TkMemory::allocate(m_alloc, (m_length + 1) * sizeof(char), 16);

			char *writePosition = m_data;
			readPosition = oldData;
			searchPosition = (char *)(searchCase == false ? TkStringUtils::findInsensitive(readPosition, searchText) : TkStringUtils::find(readPosition, searchText));
			while (searchPosition != nullptr)
			{
				const i32 writeOffset = (i32)(searchPosition - readPosition);
				memcpy(writePosition, readPosition, writeOffset * sizeof(char));
				writePosition += writeOffset;

				if (replacementTextLength > 0)
					memcpy(writePosition, replacementText, replacementTextLength * sizeof(char));
				writePosition += replacementTextLength;

				readPosition = searchPosition + searchTextLength;
				searchPosition = (char *)(searchCase == false ? TkStringUtils::findInsensitive(readPosition, searchText) : TkStringUtils::find(readPosition, searchText));
			}

			const i32 writeOffset = (i32)(oldData - readPosition) + oldLength;
			memcpy(writePosition, readPosition, writeOffset * sizeof(char));

			m_data[m_length] = 0;
			TkMemory::free(m_alloc, oldData);
		}

		return replacedCount;
	}

	void TkString::reverse()
	{
		// char c;
		// i32 tmp, count = m_length, end = count / 2;
		// for (i32 i = 0; i < end; i++)
		// {
		// 	tmp = count - i - 2;
		// 	c = m_data[i];
		// 	m_data[i] = m_data[tmp];
		// 	m_data[tmp] = c;
		// }
		for (i32 i = 0, j = m_length - 1; i < j; ++i, --j)
			toolkit::swap(m_data[i], m_data[j]);
	}

	void TkString::resize(i32 length)
	{
		TK_ASSERT(length >= 0);
		if (m_length != length)
		{
			const auto oldData = m_data;
			const auto minLength = m_length < length ? m_length : length;
			m_length = length;
			m_data = (char *)TkMemory::allocate(m_alloc, (length + 1) * sizeof(char), 16);
			memcpy(m_data, oldData, minLength * sizeof(char));
			m_data[length] = 0;
			TkMemory::free(m_alloc, oldData);
		}
	}

	i32 TkString::compare(const TkString &str, bool searchCase) const
	{
		if (searchCase == true)
			return TkStringUtils::compareStrings(this->getText(), str.getText());
		return TkStringUtils::compareIStrings(this->getText(), str.getText());
	}

	i32 TkString::find(char c) const
	{
		const char *TK_RESTRICT start = get();
		for (const char *TK_RESTRICT data = start, *TK_RESTRICT dataEnd = data + m_length; data != dataEnd; ++data)
		{
			if (*data == c)
			{
				return static_cast<i32>(data - start);
			}
		}
		return -1;
	}

	i32 TkString::findLast(char c) const
	{
		const char *TK_RESTRICT end = get() + m_length;
		for (const char *TK_RESTRICT data = end, *TK_RESTRICT dataStart = data - m_length; data != dataStart;)
		{
			--data;
			if (*data == c)
			{
				return static_cast<i32>(data - dataStart);
			}
		}
		return -1;
	}

	i32 TkString::find(const char *subStr, bool searchCase, i32 startPosition) const
	{
		if (!subStr || !*subStr || !m_data)
			return -1;
		const char *start = m_data;
		if (startPosition != -1)
			start += startPosition < length() ? startPosition : length();
		const char *tmp = searchCase == false ? TkStringUtils::findInsensitive(start, subStr) : TkStringUtils::find(start, subStr);
		return tmp ? static_cast<i32>(tmp - **this) : -1;
	}

	i32 TkString::findLast(const char *subStr, bool searchCase, i32 startPosition) const
	{
		const i32 subStrLen = TkStringUtils::length(subStr);
		if (subStrLen == 0 || !m_data)
			return -1;
		if (startPosition == -1)
			startPosition = length();
		const char *start = m_data;
		if (searchCase == false)
		{
			for (i32 i = startPosition - subStrLen; i >= 0; i--)
			{
				if (TkStringUtils::compareIStrings(start + i, subStr, subStrLen) == 0)
					return i;
			}
		}
		else
		{
			for (i32 i = startPosition - subStrLen; i >= 0; i--)
			{
				if (TkStringUtils::compareStrings(start + i, subStr, subStrLen) == 0)
					return i;
			}
		}
		return -1;
	}

	i32 TkString::findFirstOf(char c, i32 startPos) const
	{
		for (i32 i = startPos; i < length(); i++)
		{
			if (m_data[i] == c)
				return i;
		}
		return -1;
	}

	i32 TkString::findFirstOf(const char *str, i32 startPos) const
	{
		if (!str)
			return -1;
		for (i32 i = startPos; i < m_length; i++)
		{
			const char c = m_data[i];
			const char *s = str;
			while (*s)
			{
				if (c == *s)
					return i;
				s++;
			}
		}
		return -1;
	}

	void TkString::reserve(i32 length)
	{
		TK_ASSERT(length >= 0);
		if (length == m_length)
			return;
		TkMemory::free(m_alloc, m_data);
		if (length != 0)
		{
			m_data = (char *)TkMemory::allocate(m_alloc, (length + 1) * sizeof(char), 16);
			m_data[length] = 0;
		}
		else
		{
			m_data = nullptr;
		}
		m_length = length;
	}

	void TkString::set(const char *chars, i32 length)
	{
		TK_ASSERT(length >= 0);
		if (length == m_length)
		{
			if (m_data == chars)
				return;
			TkMemory::memoryCopy(m_data, chars, length * sizeof(char));
		}
		else
		{
			char *data = nullptr;
			if (length != 0)
			{
				data = (char *)TkMemory::allocate(m_alloc, (length + 1) * sizeof(char), 16);
				TkMemory::memoryCopy(data, chars, length * sizeof(char));
				data[length] = '\0';
			}
			TkMemory::free(m_alloc, m_data);
			m_data = data;
			m_length = length;
		}
	}

	void TkString::append(const char *chars, i32 count)
	{
		if (count == 0)
			return;

		const auto oldData = m_data;
		const auto oldLength = m_length;

		m_length = oldLength + count;
		m_data = (char *)TkMemory::allocate(m_alloc, (m_length + 1) * sizeof(char), 16);

		memcpy(m_data, oldData, oldLength * sizeof(char));
		memcpy(m_data + oldLength, chars, count * sizeof(char));
		m_data[m_length] = 0;

		TkMemory::free(m_alloc, oldData);
	}

	TkString &TkString::operator+=(const TkStringView &str)
	{
		append(str.get(), str.length());
		return *this;
	}

	TkString &TkString::operator=(const TkStringView &s)
	{
		set(s.get(), s.length());
		return *this;
	}

	bool TkString::startsWith(TkStringView prefix, bool searchCase) const
	{
		if (prefix.isEmpty() || prefix.length() > length())
			return false;
		if (!searchCase)
			return TkStringUtils::equalIStrings(this->getText(), *prefix, prefix.length());
		return TkStringUtils::equalStrings(this->getText(), *prefix, prefix.length());
	}

	bool TkString::endsWith(TkStringView suffix, bool searchCase) const
	{
		if (suffix.isEmpty() || suffix.length() > length())
			return false;
		if (!searchCase)
			return TkStringUtils::equalIStrings(&(*this)[length() - suffix.length()], *suffix);
		return TkStringUtils::equalStrings(&(*this)[length() - suffix.length()], *suffix);
	}

	TkString TkString::toLowerAscii() const
	{
		TkString result(*this);
		for (i32 i = 0; i < result.length(); i++)
			result[i] = TkStringUtils::toLower(result[i]);
		return result;
	}

	TkString TkString::toUpperAscii() const
	{
		TkString result(*this);
		for (i32 i = 0; i < result.length(); i++)
			result[i] = TkStringUtils::toUpper(result[i]);
		return result;
	}

	void TkString::insert(i32 startIndex, const TkString &other)
	{
		TK_ASSERT(other.m_data != m_data);
		const i32 myLength = length();
		const i32 otherLength = other.length();
		TK_ASSERT(startIndex >= 0 && startIndex < m_length);

		if (otherLength == 0)
			return;

		if (myLength == 0)
		{
			operator=(other);
			return;
		}

		const auto oldData = m_data;
		const auto oldLength = m_length;

		m_length = oldLength + otherLength;
		m_data = (char *)TkMemory::allocate(m_alloc, (m_length + 1) * sizeof(char), 16);

		memcpy(m_data, oldData, startIndex * sizeof(char));
		memcpy(m_data + startIndex, other.get(), otherLength * sizeof(char));
		memcpy(m_data + startIndex + otherLength, oldData + startIndex, (oldLength - startIndex) * sizeof(char));
		m_data[m_length] = 0;

		TkMemory::free(m_alloc, oldData);
	}

	void TkString::remove(i32 startIndex, i32 length)
	{
		//FIX:
		//TODO: implement capacity to reduce realloc
		const char *oldData = m_data;
		const i32 oldLength = m_length;

		TK_ASSERT(startIndex >= 0);
		TK_ASSERT(length >= 0);
		TK_ASSERT(startIndex + length <= oldLength);

		if (length == 0)
			return;

		if (startIndex == 0 && length == oldLength)
		{
			clear();
			return;
		}

		const i32 newLength = oldLength - length;
		char *newData = (char *)TkMemory::allocate(
			m_alloc, (newLength + 1) * sizeof(char), 16);

		if (startIndex > 0)
		{
			memcpy(newData, oldData, startIndex);
		}

		const i32 tailLength = oldLength - (startIndex + length);
		if (tailLength > 0)
		{
			memcpy(newData + startIndex,
				   oldData + startIndex + length,
				   tailLength);
		}

		newData[newLength] = '\0';

		TkMemory::free(m_alloc, (void*)oldData);

		m_data = newData;
		m_length = newLength;
	}

	void TkString::split(char c, TkArray<TkString> &results) const
	{
		results.clear();
		i32 start = 0;
		i32 length = this->length();

		for (i32 i = 0; i < length; i++)
		{
			if (m_data[i] == c)
			{
				i32 count = i - start;
				if (count > 0)
				{
					results.add(substring(start, count));
				}
				start = i + 1;
			}
		}

		const i32 count = length - start;
		if (count > 0)
		{
			results.add(substring(start, count));
		}
	}

	TkString TkString::toLower() const
	{
		return TkString();
	}

	TkString TkString::toUpper() const
	{
		return {};
	}

	TkString TkString::caseFold() const
	{
		return TkString();
	}

	bool TkString::equalsIgnoreCase(const TkStringView &) const
	{
		return false;
	}

	TkString TkString::trimmed() const
	{
		return TkString();
	}

	TkString TkString::simplified() const
	{
		return TkString();
	}
}