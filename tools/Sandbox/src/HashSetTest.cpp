/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "HashSetTest.hpp"
#include "toolkit/base/Hash.hpp"
#include <toolkit/collection/HashSet.hpp>

namespace toolkit
{
    void Test_HashSet_Basic()
    {
        TkHashSet<int> s;
        TK_ASSERT(s.isEmpty());

        s.add(1);
        s.add(2);
        s.add(3);

        TK_ASSERT(s.count() == 3);
        TK_ASSERT(s.contains(1));
        TK_ASSERT(s.contains(2));
        TK_ASSERT(s.contains(3));
        TK_ASSERT(!s.contains(4));
    }

    void Test_HashSet_Duplicate()
    {
        TkHashSet<int> s;
        TK_ASSERT(s.add(42));
        TK_ASSERT(!s.add(42));
        TK_ASSERT(s.count() == 1);
    }

    void Test_HashSet_Remove()
    {
        TkHashSet<int> s;
        for (int i = 0; i < 100; ++i)
            s.add(i);

        for (int i = 0; i < 50; ++i)
            TK_ASSERT(s.remove(i));

        for (int i = 0; i < 50; ++i)
            TK_ASSERT(!s.contains(i));

        for (int i = 50; i < 100; ++i)
            TK_ASSERT(s.contains(i));

        TK_ASSERT(s.count() == 50);
    }

    void Test_HashSet_Rehash()
    {
        TkHashSet<int> s;
        for (int i = 0; i < 10000; ++i)
            s.add(i);

        for (int i = 0; i < 10000; ++i)
            TK_ASSERT(s.contains(i));

        for (int i = 0; i < 5000; ++i)
            s.remove(i);

        for (int i = 0; i < 5000; ++i)
            TK_ASSERT(!s.contains(i));

        for (int i = 5000; i < 10000; ++i)
            TK_ASSERT(s.contains(i));
    }

    struct TestObjB
    {
        static int Alive;
        int v;

        TestObjB(int v) : v(v) { ++Alive; }
        TestObjB(const TestObjB &o) : v(o.v) { ++Alive; }
        ~TestObjB() { --Alive; }

        bool operator==(const TestObjB &o) const { return v == o.v; }
    };

    int TestObjB::Alive = 0;

    void Test_HashSet_Lifetime()
    {
        // {
        //     TkHashSet<TestObjB> s;
        //     for (int i = 0; i < 100; ++i)
        //         s.add(TestObjB(i));

        //     TK_ASSERT(TestObjB::Alive == 100);

        //     for (int i = 0; i < 50; ++i)
        //         s.remove(TestObjB(i));

        //     TK_ASSERT(TestObjB::Alive == 50);
        // }
        // TK_ASSERT(TestObjB::Alive == 0);
    }

    void testHashSet()
    {
        Test_HashSet_Basic();
        Test_HashSet_Duplicate();
        Test_HashSet_Lifetime();
        Test_HashSet_Rehash();
        Test_HashSet_Remove();
    }
}