/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/collection/Array.hpp>

namespace toolkit {
	
	class TkArrayUtils;

	// Represents a collection of objects that have a common key
	template<typename TKey, typename TSource>
	class IGrouping : public TkArray<TSource>
	{
		friend class TkArrayUtils;

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
	class TkArrayUtils
	{
	public:
		// Searches for the specified object using a custom query and returns the zero-based index of the first occurrence within the entire collection.
		template<typename T>
		static i32 indexOf(const TkArray<T>& obj, const TkFunc<bool(const T&)>& predicate)
		{
			for (i32 i = 0; i < obj.count(); i++)
			{
				if (predicate(obj[i]))
					return i;
			}
			return -1;
		}

		// The Any operator checks, if there are any elements in the collection matching the predicate. It does not select the element, but returns true if at least one element is matched.
		template<typename T>
		static bool any(const TkArray<T>& obj, const TkFunc<bool(const T&)>& predicate)
		{
			for (i32 i = 0; i < obj.count(); i++)
			{
				if (predicate(obj[i]))
					return true;
			}
			return false;
		}

		// All Any operator returns true if all elements match the predicate. It does not select the element, but returns true if all elements are matching.
		template<typename T>
		static i32 all(const TkArray<T>& obj, const TkFunc<bool(const T&)>& predicate)
		{
			for (i32 i = 0; i < obj.count(); i++)
			{
				if (!predicate(obj[i]))
					return false;
			}
			return true;
		}

		// Filters a sequence of values based on a predicate.
		template<typename T>
		static void where(const TkArray<T>& obj, const TkFunc<bool(const T&)>& predicate, TkArray<T>& result)
		{
			for (const T& i : obj)
			{
				if (predicate(i))
					result.add(i);
			}
		}

		// Filters a sequence of values based on a predicate.
		template<typename T>
		static TkArray<T> where(const TkArray<T>& obj, const TkFunc<bool(const T&)>& predicate)
		{
			TkArray<T> result;
			where(obj, predicate, result);
			return result;
		}

		// Projects each element of a sequence into a new form.
		template<typename TResult, typename TSource>
		static void select(const TkArray<TSource>& obj, const TkFunc<TResult(const TSource&)>& selector, TkArray<TResult>& result)
		{
			for (const TSource& i : obj)
				result.add(MoveTemp(selector(i)));
		}

		// Projects each element of a sequence into a new form.
		template<typename TResult, typename TSource>
		static TkArray<TResult> select(const TkArray<TSource>& obj, const TkFunc<TResult(const TSource&)>& selector)
		{
			TkArray<TResult> result;
			select(obj, selector, result);
			return result;
		}

		// Removes all the elements that match the conditions defined by the specified predicate.
		template<typename T>
		static void removeall(TkArray<T>& obj, const TkFunc<bool(const T&)>& predicate)
		{
			for (i32 i = obj.count() - 1; i >= 0; i--)
			{
				if (predicate(obj[i]))
					obj.removeAtKeepOrder(i);
			}
		}

		// Removes all the elements that match the conditions defined by the specified predicate.
		template<typename T>
		static TkArray<T> removeall(const TkArray<T>& obj, const TkFunc<bool(const T&)>& predicate)
		{
			TkArray<T> result;
			for (const T& i : obj)
			{
				if (!predicate(i))
					result.ass(i);
			}
			return result;
		}

		// Groups the elements of a sequence according to a specified key selector function.
		template<typename TSource, typename TKey>
		static void groupBy(const TkArray<TSource>& obj, const TkFunc<TKey(TSource const&)>& keySelector, TkArray<IGrouping<TKey, TSource>>& result)
		{
			TkHashMap<TKey, IGrouping<TKey, TSource>> data;
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