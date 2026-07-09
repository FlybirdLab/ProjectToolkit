/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "StringView.hpp"
#include "String.hpp"

namespace toolkit
{

	TkStringView TkStringView::Empty;

	TkStringView::TkStringView(const TkString &str)
		: TkStringView(str.get(), str.length())
	{
	}

	bool TkStringView::operator==(const TkString &other) const
	{
		return this->compare(TkStringView(other)) == 0;
	}

	bool TkStringView::operator!=(const TkString &other) const
	{
		return this->compare(TkStringView(other)) != 0;
	}

	TkStringView TkStringView::left(i32 count) const
	{
		const i32 countClamped = count < 0 ? 0 : count < length() ? count
																  : length();
		return TkStringView(**this, countClamped);
	}

	TkStringView TkStringView::right(i32 count) const
	{
		//FIX:
		const i32 countClamped =
			count < 0 ? 0 : (count < length() ? count : length());

		return TkStringView(
			get() + (length() - countClamped),
			countClamped);
	}

	i32 TkStringView::compare(const TkStringView &str, bool searchCase) const
	{
		const bool thisIsShorter = TkStringView::length() < str.length();
		const i32 minLength = thisIsShorter ? TkStringView::length() : str.length();
		const i32 prefixCompare = searchCase ? TkStringUtils::compareStrings(getText(), str.getText(), minLength) : TkStringUtils::compareIStrings(getText(), str.getText(), minLength);
		if (prefixCompare != 0)
			return prefixCompare;
		if (TkStringView::length() == str.length())
			return 0;
		return thisIsShorter ? -1 : 1;
	}

	i32 TkStringView::find(char c) const
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

	i32 TkStringView::findLast(char c) const
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

	bool TkStringView::startsWith(char c, bool searchCase) const
	{
		const i32 length = TkStringView::length();
		if (!searchCase)
			return length > 0 && TkStringUtils::toLower(m_data[0]) == TkStringUtils::toLower(c);
		return length > 0 && m_data[0] == c;
	}

	bool TkStringView::endsWith(char c, bool searchCase) const
	{
		const i32 length = TkStringView::length();
		if (!searchCase)
			return length > 0 && TkStringUtils::toLower(m_data[length - 1]) == TkStringUtils::toLower(c);
		return length > 0 && m_data[length - 1] == c;
	}

	bool TkStringView::startsWith(const TkStringView prefix, bool searchCase) const
	{
		if (prefix.isEmpty() || length() < prefix.length())
			return false;

		if (searchCase)
			return TkStringUtils::equalStrings(get(), *prefix, prefix.length());
		return TkStringUtils::equalIStrings(get(), *prefix, prefix.length());
	}

	bool TkStringView::endsWith(const TkStringView suffix, bool searchCase) const
	{
		if (suffix.isEmpty() || length() < suffix.length())
			return false;

		const char *start = get() + length() - suffix.length();

		if (searchCase)
			return TkStringUtils::equalStrings(start, *suffix, suffix.length());
		return TkStringUtils::equalIStrings(start, *suffix, suffix.length());
	}

	TkStringView TkStringView::substring(i32 startIndex) const
	{
		TK_ASSERT(startIndex >= 0 && startIndex < length());
		return {get() + startIndex, length() - startIndex};
	}

	TkStringView TkStringView::substring(i32 startIndex, i32 count) const
	{
		TK_ASSERT(startIndex >= 0 && startIndex + count <= length() && count >= 0);
		return {get() + startIndex, count};
	}

	TkString TkStringView::toString() const
	{
		return TkString(m_data, m_length);
	}

	bool operator==(const TkString &a, const TkStringView &b)
	{
		return a.length() == b.length() && TkStringUtils::compareStrings(a.getText(), b.getText(), b.length()) == 0;
	}

	bool operator!=(const TkString &a, const TkStringView &b)
	{
		return a.length() != b.length() || TkStringUtils::compareStrings(a.getText(), b.getText(), b.length()) != 0;
	}

}