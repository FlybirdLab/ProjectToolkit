// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "String.hpp"
#include "StringView.hpp"

#include "toolkit/collection/Array.hpp"

namespace toolkit {

	TString TString::Empty{};
	TString::TString(IAllocator& alloc)
	    : m_alloc {alloc}
	{}
	TString::TString(const TStringView& str, IAllocator& alloc)
	    : m_alloc {alloc}
	{
		set(str.get(), str.length());
	}


	TString& TString::operator/=(const char* str)
	{
		const i32 length = m_length;
		if (length > 0 && m_data[length - 1] != '/' && m_data[length - 1] != '\\'
			&& (str == nullptr || (str[0] != '/' && str[0] != '\\')))
		{
			*this += '/';
		}
		return *this += str;
	}


	TString& TString::operator/=(const char c)
	{
		const i32 length = m_length;
		if (length > 0 && m_data[length - 1] != '/' && m_data[length - 1] != '\\')
		{
			*this += '/';
		}
		return *this += c;
	}


	TString& TString::operator/=(const TStringView& str)
	{
		const i32 length = m_length;
		if (length > 1 && m_data[length - 1] != '/' && m_data[length - 1] != '\\'
			&& (str == nullptr || (str[0] != '/' && str[0] != '\\')))
		{
			*this += '/';
		}
		return *this += str;
	}


	bool TString::startsWith(char c, bool searchCase) const
	{
		const i32 length = TString::length();
		if (searchCase == true)
			return length > 0 && m_data[0] == c;
		return length > 0 && TStringUtils::toLower(m_data[0]) == TStringUtils::toLower(c);
	}


	bool TString::endsWith(char c, bool searchCase) const
	{
		const i32 length = TString::length();
		if (searchCase == true)
			return length > 0 && m_data[length - 1] == c;
		return length > 0 && TStringUtils::toLower(m_data[length - 1]) == TStringUtils::toLower(c);
	}


	bool TString::startsWith(const TString& prefix, bool searchCase) const
	{
		if (prefix.isEmpty())
			return true;
		if (length() < prefix.length())
			return false;
		if (searchCase == false)
			return TStringUtils::compareIStrings(this->getText(), *prefix, prefix.length()) == 0;
		return TStringUtils::compareStrings(this->getText(), *prefix, prefix.length()) == 0;
	}


	bool TString::endsWith(const TString& suffix, bool searchCase) const
	{
		if (suffix.isEmpty())
			return true;
		if (length() < suffix.length())
			return false;
		if (searchCase == false)
			return TStringUtils::compareIStrings(&(*this)[TString::length() - suffix.length()], *suffix) == 0;
		return TStringUtils::compareStrings(&(*this)[length() - suffix.length()], *suffix) == 0;
	}


	i32 TString::replace(char searchwchar, char replacementwchar, bool searchCase)
	{
		i32 replacedwchars = 0;
		const i32 length = TString::length();
		if (searchCase == false)
		{
			const char toCompare = TStringUtils::toLower(searchwchar);
			for (i32 i = 0; i < length; i++)
			{
				if (TStringUtils::toLower(m_data[i]) == toCompare)
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


	i32 TString::replace(const char* searchText, const char* replacementText, bool searchCase)
	{
		const i32 searchTextLength = TStringUtils::length(searchText);
		const i32 replacementTextLength = TStringUtils::length(replacementText);
		return replace(searchText, searchTextLength, replacementText, replacementTextLength, searchCase);
	}


	i32 TString::replace(const char* searchText, i32 searchTextLength, const char* replacementText, i32 replacementTextLength, bool searchCase)
	{
		if (!hasChars() || searchTextLength == 0)
			return 0;

		i32 replacedCount = 0;

		if (searchTextLength == replacementTextLength)
		{
			char* pos = (char*)(searchCase == false ? TStringUtils::findInsensitive(m_data, searchText) : TStringUtils::find(m_data, searchText));
			while (pos != nullptr)
			{
				replacedCount++;

				for (i32 i = 0; i < replacementTextLength; i++)
					pos[i] = replacementText[i];

				if (pos + searchTextLength - **this < length())
					pos = (char*)(searchCase == false ? TStringUtils::findInsensitive(pos + searchTextLength, searchText) : TStringUtils::find(pos + searchTextLength, searchText));
				else
					break;
			}
		}
		else if (contains(searchText, searchCase))
		{
			char* readPosition = m_data;
			char* searchPosition = (char*)(searchCase == false ? TStringUtils::findInsensitive(readPosition, searchText) : TStringUtils::find(readPosition, searchText));
			while (searchPosition != nullptr)
			{
				replacedCount++;
				readPosition = searchPosition + searchTextLength;
				searchPosition = (char*)(searchCase == false ? TStringUtils::findInsensitive(readPosition, searchText) : TStringUtils::find(readPosition, searchText));
			}

			const auto oldLength = m_length;
			const auto oldData = m_data;
			m_length += replacedCount * (replacementTextLength - searchTextLength);
			m_data = (char*)TMemory::allocate(m_alloc, (m_length + 1) * sizeof(char), 16);

			char* writePosition = m_data;
			readPosition = oldData;
			searchPosition = (char*)(searchCase == false ? TStringUtils::findInsensitive(readPosition, searchText) : TStringUtils::find(readPosition, searchText));
			while (searchPosition != nullptr)
			{
				const i32 writeOffset = (i32)(searchPosition - readPosition);
				memcpy(writePosition, readPosition, writeOffset * sizeof(char));
				writePosition += writeOffset;

				if (replacementTextLength > 0)
					memcpy(writePosition, replacementText, replacementTextLength * sizeof(char));
				writePosition += replacementTextLength;

				readPosition = searchPosition + searchTextLength;
				searchPosition = (char*)(searchCase == false ? TStringUtils::findInsensitive(readPosition, searchText) : TStringUtils::find(readPosition, searchText));
			}

			const i32 writeOffset = (i32)(oldData - readPosition) + oldLength;
			memcpy(writePosition, readPosition, writeOffset * sizeof(char));

			m_data[m_length] = 0;
			TMemory::free(m_alloc, oldData);
		}

		return replacedCount;
	}


	void TString::reverse()
	{
		char c;
		i32 tmp, count = m_length, end = count / 2;
		for (i32 i = 0; i < end; i++)
		{
			tmp = count - i - 2;
			c = m_data[i];
			m_data[i] = m_data[tmp];
			m_data[tmp] = c;
		}
	}


	void TString::resize(i32 length)
	{
		TK_ASSERT(length >= 0);
		if (m_length != length)
		{
			const auto oldData = m_data;
			const auto minLength = m_length < length ? m_length : length;
			m_length = length;
			m_data = (char*)TMemory::allocate(m_alloc, (length + 1) * sizeof(char), 16);
			memcpy(m_data, oldData, minLength * sizeof(char));
			m_data[length] = 0;
			TMemory::free(m_alloc, oldData);
		}
	}


	i32 TString::compare(const TString& str, bool searchCase) const
	{
		if (searchCase == true)
			return TStringUtils::compareStrings(this->getText(), str.getText());
		return TStringUtils::compareIStrings(this->getText(), str.getText());
	}


	i32 TString::find(char c) const
	{
		const char* TK_RESTRICT start = get();
		for (const char* TK_RESTRICT data = start, *TK_RESTRICT dataEnd = data + m_length; data != dataEnd; ++data)
		{
			if (*data == c)
			{
				return static_cast<i32>(data - start);
			}
		}
		return -1;
	}


	i32 TString::findLast(char c) const
	{
		const char* TK_RESTRICT end = get() + m_length;
		for (const char* TK_RESTRICT data = end, *TK_RESTRICT dataStart = data - m_length; data != dataStart;)
		{
			--data;
			if (*data == c)
			{
				return static_cast<i32>(data - dataStart);
			}
		}
		return -1;
	}


	i32 TString::find(const char* subStr, bool searchCase, i32 startPosition) const
	{
		if (subStr == nullptr || !m_data)
			return -1;
		const char* start = m_data;
		if (startPosition != -1)
			start += startPosition < length() ? startPosition : length();
		const char* tmp = searchCase == false ? TStringUtils::findInsensitive(start, subStr) : TStringUtils::find(start, subStr);
		return tmp ? static_cast<i32>(tmp - **this) : -1;
	}


	i32 TString::findLast(const char* subStr, bool searchCase, i32 startPosition) const
	{
		const i32 subStrLen = TStringUtils::length(subStr);
		if (subStrLen == 0 || !m_data)
			return -1;
		if (startPosition == -1)
			startPosition = length();
		const char* start = m_data;
		if (searchCase == false)
		{
			for (i32 i = startPosition - subStrLen; i >= 0; i--)
			{
				if (TStringUtils::compareIStrings(start + i, subStr, subStrLen) == 0)
					return i;
			}
		}
		else
		{
			for (i32 i = startPosition - subStrLen; i >= 0; i--)
			{
				if (TStringUtils::compareStrings(start + i, subStr, subStrLen) == 0)
					return i;
			}
		}
		return -1;
	}


	i32 TString::findFirstOf(char c, i32 startPos) const
	{
		for (i32 i = startPos; i < length(); i++)
		{
			if (m_data[i] == c)
				return i;
		}
		return -1;
	}


	i32 TString::findFirstOf(const char* str, i32 startPos) const
	{
		if (!str)
			return -1;
		for (i32 i = startPos; i < m_length; i++)
		{
			const char c = m_data[i];
			const char* s = str;
			while (*s)
			{
				if (c == *s)
					return i;
				s++;
			}
		}
		return -1;
	}


	void TString::reserve(i32 length)
	{
		TK_ASSERT(length >= 0);
		if (length == m_length)
			return;
		TMemory::free(m_alloc, m_data);
		if (length != 0)
		{
			m_data = (char*)TMemory::allocate(m_alloc, (length + 1) * sizeof(char), 16);
			m_data[length] = 0;
		}
		else
		{
			m_data = nullptr;
		}
		m_length = length;
	}


	void TString::set(const char* chars, i32 length)
	{
		TK_ASSERT(length >= 0);
		if (length == m_length)
		{
			if (m_data == chars)
				return;
			memcpy(m_data, chars, length * sizeof(char));
		}
		else
		{
			char* data = nullptr;
			if (length != 0)
			{
				data = (char*)TMemory::allocate(m_alloc, (length + 1) * sizeof(char), 16);
				memcpy(data, chars, length * sizeof(char));
				data[length] = 0;
			}
			TMemory::free(m_alloc, m_data);
			m_data = data;
			m_length = length;
		}
	}


	void TString::append(const char* chars, i32 count)
	{
		if (count == 0)
			return;

		const auto oldData = m_data;
		const auto oldLength = m_length;

		m_length = oldLength + count;
		m_data = (char*)TMemory::allocate(m_alloc, (m_length + 1) * sizeof(char), 16);

		memcpy(m_data, oldData, oldLength * sizeof(char));
		memcpy(m_data + oldLength, chars, count * sizeof(char));
		m_data[m_length] = 0;

		TMemory::free(m_alloc, oldData);
	}


	TString& TString::operator+=(const TStringView& str)
	{
		append(str.get(), str.length());
		return *this;
	}


	TString& TString::operator=(const TStringView& s)
	{
		set(s.get(), s.length());
		return *this;
	}


	bool TString::startsWith(const TStringView& prefix, bool searchCase) const
	{
		if (prefix.isEmpty() || prefix.length() > length())
			return false;
		if (searchCase == false)
			return !TStringUtils::compareIStrings(this->getText(), *prefix, prefix.length());
		return !TStringUtils::compareStrings(this->getText(), *prefix, prefix.length());
	}


	bool TString::endsWith(const TStringView& suffix, bool searchCase) const
	{
		if (suffix.isEmpty() || suffix.length() > length())
			return false;
		if (searchCase == false)
			return !TStringUtils::compareIStrings(&(*this)[length() - suffix.length()], *suffix);
		return !TStringUtils::compareStrings(&(*this)[length() - suffix.length()], *suffix);
	}


	TString TString::toLower() const
	{
		TString result(*this);
		for (i32 i = 0; i < result.length(); i++)
			result[i] = TStringUtils::toLower(result[i]);
		return result;
	}


	TString TString::toUpper() const
	{
		TString result(*this);
		for (i32 i = 0; i < result.length(); i++)
			result[i] = TStringUtils::toUpper(result[i]);
		return result;
	}


	void TString::insert(i32 startIndex, const TString& other)
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
		m_data = (char*)TMemory::allocate(m_alloc, (m_length + 1) * sizeof(char), 16);

		memcpy(m_data, oldData, startIndex * sizeof(char));
		memcpy(m_data + startIndex, other.get(), otherLength * sizeof(char));
		memcpy(m_data + startIndex + otherLength, oldData + startIndex, (oldLength - startIndex) * sizeof(char));
		m_data[m_length] = 0;

		TMemory::free(m_alloc, oldData);
	}


	void TString::remove(i32 startIndex, i32 length)
	{
		const auto oldData = m_data;
		const auto oldLength = m_length;
		TK_ASSERT(startIndex >= 0 && startIndex + length <= oldLength);

		if (startIndex == 0 && oldLength == length)
		{
			clear();
			return;
		}

		m_length = oldLength - length;
		m_data = (char*)TMemory::allocate(m_alloc, (m_length + 1) * sizeof(char), 16);

		memcpy(m_data, oldData, startIndex * sizeof(char));
		memcpy(m_data + startIndex, oldData + startIndex + length, length * sizeof(char));
		m_data[m_length] = 0;

		TMemory::free(m_alloc, oldData);
	}


	void TString::split(char c, TArray<TString>& results) const
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
}