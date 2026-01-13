/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/base/Format.hpp>
#include <toolkit/utils/StringUtils.hpp>

namespace toolkit
{

    // Impl for strings building class
    class TkStringBuilder
    {
    private:
        // Array with characters of the string (it's not null-terminated)
        TkArray<char> m_data;
        IAllocator& m_alloc;

    public:
        TkStringBuilder(IAllocator& alloc = TkMemory::getDefault());
        TkStringBuilder(i32 capacity, IAllocator& alloc = TkMemory::getDefault());

    public:
        TK_FORCE_INLINE i32 capacity() const
        {
            return m_data.capacity();
        }
        TK_FORCE_INLINE void setCapacity(const i32 capacity)
        {
            m_data.setCapacity(capacity);
        }

        TK_FORCE_INLINE i32 length() const
        {
            return m_data.count();
        }
        TK_FORCE_INLINE void clear()
        {
            m_data.clear();
        }

        void toString(TkString &result) const;

    public:
        TkStringBuilder &append(const char c);
        TkStringBuilder &append(const char *str);
        TkStringBuilder &append(const char *str, i32 length);
        TkStringBuilder &append(const TkString &str);
        TkStringBuilder &append(const TkStringView &str);
        TkStringBuilder &append(i32 val);
        TkStringBuilder &append(u32 val);

		template<typename... Args>
		TkStringBuilder& appendFormat(const char* format, const Args& ... args)
		{
			FormatBuffer buf { m_alloc };
			Formatter::format(buf, format, args...);
            return append(buf.getPointer(), buf.getUsed());
		}

        TkStringBuilder &appendLine();
        TkStringBuilder &appendLine(i32);
        TkStringBuilder &appendLine(u32);
        TkStringBuilder &appendLine(const char *str);
        TkStringBuilder &appendLine(const TkString &str);

    public:
        // Gets pointer to the string.
        TK_FORCE_INLINE const char *operator*() const
        {
            return m_data.count() > 0 ? m_data.get() : "";
        }
        // Gets string buffer as array of wchars.
        TK_FORCE_INLINE TkArray<char> &getCharArray()
        {
            return m_data;
        }
        // Gets string buffer as const array of wchars.
        TK_FORCE_INLINE const TkArray<char> &getCharArray() const
        {
            return m_data;
        }

    public:
        TkString toString() const;
        TkStringView toStringView() const;
    };

    inline u32 getHash(const TkStringBuilder &key)
    {
        return TkStringUtils::getStringsHash(key.getCharArray().get(), key.length());
    }
}


namespace toolkit {

	template<typename T>
	inline psize formatCustom(FormatBuffer& b, const TkStringBuilder& v) {
		b.appendData((void*)v.getCharArray().get(), v.length() * sizeof(char));
		return v.length();
	}

}