// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/collection/Array.hpp>

namespace toolkit {
	
	class TArrayUtils;

	// Represents a collection of objects that have a common key
	template<typename TKey, typename TSource>
	class IGrouping : public TArray<TSource>
	{
		friend class TArrayUtils;

	protected:
		TKey m_key;

	public:
		// Gets the common key.
		TK_FORCE_INLINE const TKey& getKey() const
		{
			return m_key;
		}

		// Gets the common key.
		TK_FORCE_INLINE TKey getKey()
		{
			return m_key;
		}
	};


	// Array collection extension methods and helpers
	class TArrayUtils
	{
	public:
		// Searches for the specified object using a custom query and returns the zero-based index of the first occurrence within the entire collection.
		template<typename T, typename AllocationType>
		static i32 indexOf(const TArray<T, AllocationType>& obj, const TFunc<bool(const T&)>& predicate)
		{
			for (i32 i = 0; i < obj.count(); i++)
			{
				if (predicate(obj[i]))
					return i;
			}
			return TK_INVALID_INDEX;
		}

		// The Any operator checks, if there are any elements in the collection matching the predicate. It does not select the element, but returns true if at least one element is matched.
		template<typename T, typename AllocationType>
		static bool any(const TArray<T, AllocationType>& obj, const TFunc<bool(const T&)>& predicate)
		{
			for (i32 i = 0; i < obj.count(); i++)
			{
				if (predicate(obj[i]))
					return true;
			}
			return false;
		}

		// All Any operator returns true if all elements match the predicate. It does not select the element, but returns true if all elements are matching.
		template<typename T, typename AllocationType>
		static i32 all(const TArray<T, AllocationType>& obj, const TFunc<bool(const T&)>& predicate)
		{
			for (i32 i = 0; i < obj.count(); i++)
			{
				if (!predicate(obj[i]))
					return false;
			}
			return true;
		}

		// Filters a sequence of values based on a predicate.
		template<typename T, typename AllocationType>
		static void where(const TArray<T, AllocationType>& obj, const TFunc<bool(const T&)>& predicate, TArray<T, AllocationType>& result)
		{
			for (const T& i : obj)
			{
				if (predicate(i))
					result.add(i);
			}
		}

		// Filters a sequence of values based on a predicate.
		template<typename T, typename AllocationType>
		static TArray<T, AllocationType> where(const TArray<T, AllocationType>& obj, const TFunc<bool(const T&)>& predicate)
		{
			TArray<T, AllocationType> result;
			where(obj, predicate, result);
			return result;
		}

		// Projects each element of a sequence into a new form.
		template<typename TResult, typename TSource, typename AllocationType>
		static void select(const TArray<TSource, AllocationType>& obj, const TFunc<TResult(const TSource&)>& selector, TArray<TResult, AllocationType>& result)
		{
			for (const TSource& i : obj)
				result.add(MoveTemp(selector(i)));
		}

		// Projects each element of a sequence into a new form.
		template<typename TResult, typename TSource, typename AllocationType>
		static TArray<TResult, AllocationType> select(const TArray<TSource, AllocationType>& obj, const TFunc<TResult(const TSource&)>& selector)
		{
			TArray<TResult, AllocationType> result;
			select(obj, selector, result);
			return result;
		}

		// Removes all the elements that match the conditions defined by the specified predicate.
		template<typename T, typename AllocationType>
		static void removeall(TArray<T, AllocationType>& obj, const TFunc<bool(const T&)>& predicate)
		{
			for (i32 i = obj.count() - 1; i >= 0; i--)
			{
				if (predicate(obj[i]))
					obj.removeAtKeepOrder(i);
			}
		}

		// Removes all the elements that match the conditions defined by the specified predicate.
		template<typename T, typename AllocationType>
		static TArray<T, AllocationType> removeall(const TArray<T, AllocationType>& obj, const TFunc<bool(const T&)>& predicate)
		{
			TArray<T, AllocationType> result;
			for (const T& i : obj)
			{
				if (!predicate(i))
					result.ass(i);
			}
			return result;
		}

		// Groups the elements of a sequence according to a specified key selector function.
		template<typename TSource, typename TKey, typename AllocationType>
		static void groupBy(const TArray<TSource, AllocationType>& obj, const TFunc<TKey(TSource const&)>& keySelector, TArray<IGrouping<TKey, TSource>, AllocationType>& result)
		{
			CHashMap<TKey, IGrouping<TKey, TSource>> data;
			for (i32 i = 0; i < obj.count(); i++)
			{
				const TKey key = keySelector(obj[i]);
				auto& group = data[key];
				group.m_key = key;
				group.add(obj[i]);
			}
			result.clear();
			data.getValues(result);
		}
	};
}