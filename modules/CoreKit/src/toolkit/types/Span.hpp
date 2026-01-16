/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit {

    // A container of a contiguous region of arbitrary memory
    template<typename T>
    class TkSpan
    {
    public:
        TkSpan()
            : m_data {nullptr}
            , m_length {0} 
        { }
        TkSpan(const T* data, i32 length)
            : m_data {(T*)data}
            , m_length {length} 
        { }
        TkSpan(T* begin, T* end)
            : m_data {begin}
            , m_length {end - begin} 
        { }
        template <int N>
        TkSpan(T(&value)[N])
            : m_data {value}
            , m_length {N} 
        { }

    public:
        TK_FORCE_INLINE bool isValid() const
        {
            return m_data != nullptr;
        }
        TK_FORCE_INLINE bool isInvalid() const
        {
            return m_data == nullptr;
        }

        TK_FORCE_INLINE i32 length() const
        {
            return m_length;
        }

        TK_FORCE_INLINE T* get()
        {
            return m_data;
        }
        TK_FORCE_INLINE const T* get() const
        {
            return m_data;
        }
        template<typename U>
        TK_FORCE_INLINE U* get() const
        {
            return (U*)m_data;
        }

    public:
        TK_FORCE_INLINE T& operator[](i32 index)
        {
            TK_ASSERT(index >= 0 && index < m_length);
            return m_data[index];
        }
        TK_FORCE_INLINE const T& operator[](i32 index) const
        {
            TK_ASSERT(index >= 0 && index < m_length);
            return m_data[index];
        }

        TK_FORCE_INLINE T* begin()
        {
            return m_data;
        }

        TK_FORCE_INLINE T* end()
        {
            return m_data + m_length;
        }

        TK_FORCE_INLINE const T* begin() const
        {
            return m_data;
        }

        TK_FORCE_INLINE const T* end() const
        {
            return m_data + m_length;
        }

    protected:
        T* m_data;
        i64 m_length;
    };


    template<typename T>
    inline TkSpan<T> toSpan(const T* ptr, i32 length)
    {
        return TkSpan<T>(ptr, length);
    }


    template<typename T, typename U = T>
    inline TkSpan<U> toSpan(const TkArray<T>& data)
    {
        return TkSpan<U>((U*)data.get(), data.count());
    }


    template<typename T>
    inline bool spanContains(const TkSpan<T> span, const T& value)
    {
        for (i32 i = 0; i < span.length(); i++)
        {
            if (span.get()[i] == value)
                return true;
        }
        return false;
    }

}