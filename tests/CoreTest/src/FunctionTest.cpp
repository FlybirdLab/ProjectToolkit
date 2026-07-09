/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/base/Function.hpp"

static int g_staticValue = 0;
static void StaticFunc(int v)
{
    g_staticValue += v;
}

TEST(TkFunc, StaticBindCall)
{
    g_staticValue = 0;
    TkFunc<void(int)> f;
    f.bind<StaticFunc>();
    EXPECT_TRUE(f.isBinded());
    f(5);
    EXPECT_EQ(g_staticValue, 5);

    TkFunc<void(int)> f2(f);
    f2(3);
    EXPECT_EQ(g_staticValue, 8);

    TkFunc<void(int)> f3(std::move(f2));
    f3(2);
    EXPECT_EQ(g_staticValue, 10);
    EXPECT_FALSE(f2.isBinded());
}

struct MyObj
{
    int value = 0;
    void add(int v) { value += v; }
    void addConst(int v) const { (const_cast<MyObj *>(this))->value += v; }
};

TEST(TkFunc, MemberBindCall)
{
    MyObj obj;
    TkFunc<void(int)> f;
    f.bind<MyObj, &MyObj::add>(&obj);
    f(5);
    EXPECT_EQ(obj.value, 5);

    // const member
    TkFunc<void(int)> f2;
    f2.bind<MyObj, &MyObj::addConst>(&obj);
    f2(3);
    EXPECT_EQ(obj.value, 8);

    // copy and move
    TkFunc<void(int)> f3(f);
    f3(2);
    EXPECT_EQ(obj.value, 10);

    TkFunc<void(int)> f4(std::move(f3));
    f4(1);
    EXPECT_EQ(obj.value, 11);
    EXPECT_FALSE(f3.isBinded());
}

TEST(TkFunc, LambdaBindCall)
{
    int value = 0;
    auto lambda = [&value](int v) { value += v; };

    TkFunc<void(int)> f(lambda);
    EXPECT_TRUE(f.isBinded());
    f(10);
    EXPECT_EQ(value, 10);

    TkFunc<void(int)> f2(f);
    f2(5);
    EXPECT_EQ(value, 15);

    TkFunc<void(int)> f3(std::move(f2));
    f3(5);
    EXPECT_EQ(value, 20);
    EXPECT_FALSE(f2.isBinded());
}

TEST(TkFunc, Unbind)
{
    int value = 0;
    TkFunc<void(int)> f([&](int v) { value += v; });
    EXPECT_TRUE(f.isBinded());
    f.unbind();
    EXPECT_FALSE(f.isBinded());
}

TEST(TkFunc, BasicBindCall)
{
    int sum = 0;
    TkAction<int> action;

    action.bind<StaticFunc>();
    g_staticValue = 0;
    action(4);
    EXPECT_EQ(g_staticValue, 4);

    action.bind([&](int v) { sum += v; });
    action(3);
    EXPECT_EQ(g_staticValue, 7); // staticFunc called again
    EXPECT_EQ(sum, 3);

    MyObj obj;
    action.bind<MyObj, &MyObj::add>(&obj);
    action(5);
    EXPECT_EQ(obj.value, 5);
    EXPECT_EQ(sum, 8); // lambda called again
}

TEST(TkFunc, UnbindAll)
{
    int sum = 0;
    TkAction<int> action;
    action.bind([&](int v) { sum += v; });
    action.bind([&](int v) { sum += v * 2; });

    action(2);
    EXPECT_EQ(sum, 6);

    action.unbindAll();
    EXPECT_EQ(action.count(), 0);
    sum = 0;
    action(5); // nothing should be called
    EXPECT_EQ(sum, 0);
}

TEST(TkFunc, BindUnique)
{
    int sum = 0;
    TkAction<int> action;

    TkFunc<void(int)> fn = [&](int v) { sum += v; };
    action.bindUnique(fn);
    action.bindUnique(fn);
    action(3);
    EXPECT_EQ(sum, 3);
}

TEST(TkFunc, GetBindings)
{
    TkAction<int> action;
    action.bind<StaticFunc>();

    TkFunc<void(int)> buffer[4];
    int count = action.getBindings(buffer, 4);
    EXPECT_EQ(count, 1);
    EXPECT_TRUE(buffer[0].isBinded());
}