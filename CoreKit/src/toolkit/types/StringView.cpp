// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "StringView.hpp"
#include "String.hpp"

namespace toolkit {

	TStringView TStringView::Empty;


	TStringView::TStringView(const TString& str)
		: TStringView(str.get(), str.length())
	{
	}


	bool TStringView::operator==(const TString& other) const
	{
		return this->compare(TStringView(other)) == 0;
	}


	bool TStringView::operator!=(const TString& other) const
	{
		return this->compare(TStringView(other)) != 0;
	}


	TStringView TStringView::left(i32 count) const
	{
		const i32 countClamped = count < 0 ? 0 : count < length() ? count : length();
		return TStringView(**this, countClamped);
	}


	TStringView TStringView::right(i32 count) const
	{
		const i32 countClamped = count < 0 ? 0 : count < length() ? count : length();
		return TStringView(**this + countClamped, length() - countClamped);
	}


	i32 TStringView::compare(const TStringView& str, bool searchCase) const
	{
		const bool thisIsShorter = TStringView::length() < str.length();
		const i32 minLength = thisIsShorter ? TStringView::length() : str.length();
		const i32 prefixCompare = searchCase ? TStringUtils::compareStrings(getText(), str.getText(), minLength) : TStringUtils::compareIStrings(getText(), str.getText(), minLength);
		if (prefixCompare != 0)
			return prefixCompare;
		if (TStringView::length() == str.length())
			return 0;
		return thisIsShorter ? -1 : 1;
	}


	i32 TStringView::find(char c) const
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


	i32 TStringView::findLast(char c) const
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


	bool TStringView::startsWith(char c, bool searchCase) const
	{
		const i32 length = TStringView::length();
		if (!searchCase)
			return length > 0 && TStringUtils::toLower(m_data[0]) == TStringUtils::toLower(c);
		return length > 0 && m_data[0] == c;
	}


	bool TStringView::endsWith(char c, bool searchCase) const
	{
		const i32 length = TStringView::length();
		if (!searchCase)
			return length > 0 && TStringUtils::toLower(m_data[length - 1]) == TStringUtils::toLower(c);
		return length > 0 && m_data[length - 1] == c;
	}


	bool TStringView::startsWith(const TStringView& prefix, bool searchCase) const
	{
		if (prefix.isEmpty() || TStringView::length() < prefix.length())
			return false;
		// We know that this TStringView is not empty, and therefore Get() below is valid.
		if (searchCase == false)
			return TStringUtils::compareIStrings(this->get(), *prefix, prefix.length()) == 0;
		return TStringUtils::compareStrings(this->get(), *prefix, prefix.length()) == 0;
	}


	bool TStringView::endsWith(const TStringView& suffix, bool searchCase) const
	{
		if (suffix.isEmpty() || TStringView::length() < suffix.length())
			return false;
		// We know that this TStringView is not empty, and therefore accessing data below is valid.
		if (searchCase == false)
			return TStringUtils::compareIStrings(&(*this)[length() - suffix.length()], *suffix) == 0;
		return TStringUtils::compareStrings(&(*this)[length() - suffix.length()], *suffix) == 0;
	}


	TStringView TStringView::substring(i32 startIndex) const
	{
		TK_ASSERT(startIndex >= 0 && startIndex < length());
		return {get() + startIndex, length() - startIndex};
	}


	TStringView TStringView::substring(i32 startIndex, i32 count) const
	{
		TK_ASSERT(startIndex >= 0 && startIndex + count <= length() && count >= 0);
		return {get() + startIndex, count };
	}


	TString TStringView::toString() const
	{
		return TString(m_data, m_length);
	}


	bool operator==(const TString& a, const TStringView& b)
	{
		return a.length() == b.length() && TStringUtils::compareStrings(a.getText(), b.getText(), b.length()) == 0;
	}


	bool operator!=(const TString& a, const TStringView& b)
	{
		return a.length() != b.length() || TStringUtils::compareStrings(a.getText(), b.getText(), b.length()) != 0;
	}

}