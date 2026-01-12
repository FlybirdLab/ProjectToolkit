/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ChunkedArrayTest.hpp"

namespace toolkit {
    
    struct TestObjA
	{
		static inline i32 ctorCount = 0;
		static inline i32 dtorCount = 0;
		static inline i32 copyCount = 0;

		i32 value = 0;

		TestObjA()
		{
			++ctorCount;
		}

		explicit TestObjA(i32 v)
			: value(v)
		{
			++ctorCount;
		}

		TestObjA(const TestObjA &o)
			: value(o.value)
		{
			++copyCount;
			++ctorCount;
		}

		~TestObjA()
		{
			++dtorCount;
		}

		bool operator==(const TestObjA &o) const
		{
			return value == o.value;
		}

		static void reset()
		{
			ctorCount = dtorCount = copyCount = 0;
		}
	};

	void basic()
	{
		TestObjA::reset();

		TkChunkedArray<TestObjA, 4> arr;

		TK_ASSERT(arr.isEmpty());
		TK_ASSERT(arr.count() == 0);
		TK_ASSERT(arr.capacity() == 0);

		for (i32 i = 0; i < 10; ++i)
			arr.add(TestObjA(i));

		TK_ASSERT(arr.count() == 10);
		TK_ASSERT(arr.capacity() >= 10);

		for (i32 i = 0; i < arr.count(); ++i)
			TK_ASSERT(arr[i].value == i);

		// iterator sequence
		i32 expect = 0;
		for (auto it = arr.begin(); it != arr.end(); ++it)
		{
			TK_ASSERT(it->value == expect);
			++expect;
		}

		TK_ASSERT(expect == 10);
	}

	void remove()
	{
		TestObjA::reset();

		TkChunkedArray<TestObjA, 4> arr;
		for (i32 i = 0; i < 9; ++i)
			arr.add(TestObjA(i));

		// remove item in the middle
		auto it = arr.iteratorAt(3);
		arr.remove(it);

		TK_ASSERT(arr.count() == 8);

		// check index 3 item is the last item
		TK_ASSERT(arr[3].value == 8);

		// all items can be access
		bool found[9] = {};
		for (i32 i = 0; i < arr.count(); ++i)
			found[arr[i].value] = true;

		TK_ASSERT(!found[3]);
		for (i32 i = 0; i < 9; ++i)
		{
			if (i == 3)
				continue;
			TK_ASSERT(found[i]);
		}
	}

	void resize()
	{
		TestObjA::reset();

		TkChunkedArray<TestObjA, 4> arr;

		arr.resize(10);
		TK_ASSERT(arr.count() == 10);

		arr.resize(3);
		TK_ASSERT(arr.count() == 3);

		arr.resize(16);
		TK_ASSERT(arr.count() == 16);
		TK_ASSERT(arr.capacity() >= 16);
	}

	void lifetime()
	{
		TestObjA::reset();

		{
			TkChunkedArray<TestObjA, 4> arr;
			for (i32 i = 0; i < 10; ++i)
				arr.add(TestObjA(i));

			arr.clear();
			TK_ASSERT(arr.count() == 0);
		}

		TK_ASSERT(TestObjA::dtorCount >= TestObjA::ctorCount);
	}

	void iteratorEdge()
	{
		TkChunkedArray<i32, 3> arr;

		TK_ASSERT(arr.begin() == arr.end());

		arr.add(1);
		auto it = arr.begin();
		TK_ASSERT(*it == 1);
		++it;
		TK_ASSERT(it == arr.end());
	}

	void testChunkedArray()
	{
		basic();
		remove();
		resize();
		lifetime();
		iteratorEdge();
	}
}