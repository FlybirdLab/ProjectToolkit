// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/collection/Array.hpp>

namespace toolkit {

	// Universal representation of a contiguous region of arbitrary memory
	template<typename T>
	class TSpan
	{
	public:
		TSpan()
			: m_data(nullptr)
			, m_length(0) {
		}
		TSpan(const T* data, i32 length)
			: m_data((T*)data)
			, m_length(length) {
		}
		TSpan(T* begin, T* end)
			: m_data(begin)
			, m_length(end - begin) {
		}
		template <int N>
		TSpan(T(&value)[N])
			: m_data(value)
			, m_length(N) {
		}

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
		i32 m_length;
	};


	template<typename T>
	inline TSpan<T> toTSpan(const T* ptr, i32 length)
	{
		return TSpan<T>(ptr, length);
	}


	template<typename T, typename U = T, typename AllocationType = HeapAllocation>
	inline TSpan<U> toTSpan(const TArray<T, AllocationType>& data)
	{
		return TSpan<U>((U*)data.get(), data.count());
	}


	template<typename T>
	inline bool spanContains(const TSpan<T> span, const T& value)
	{
		for (i32 i = 0; i < span.length(); i++)
		{
			if (span.get()[i] == value)
				return true;
		}
		return false;
	}

}