/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include <toolkit/threading/TaskGraph.hpp>
#include <toolkit/platforms/System.hpp>

class TaskGraphTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        System::init();
        System::beforeRun();
    }

    static void TearDownTestSuite()
    {
        System::beforeQuit();
        System::quit();
    }
};

TEST_F(TaskGraphTest, AddAndRemoveSystems)
{
    TkTaskGraph graph;
    TkTaskGraphSys a;
    TkTaskGraphSys b;

    graph.addSystem(&a);
    graph.addSystem(&b);

    ASSERT_EQ(graph.getSystems().count(), 2);
    EXPECT_EQ(graph.getSystems()[0], &a);
    EXPECT_EQ(graph.getSystems()[1], &b);

    graph.removeSystem(&a);

    ASSERT_EQ(graph.getSystems().count(), 1);
    EXPECT_EQ(graph.getSystems()[0], &b);
}

TEST_F(TaskGraphTest, ExecuteEmptyGraph)
{
    TkTaskGraph graph;
    graph.execute();
    SUCCEED();
}

TEST_F(TaskGraphTest, ExecuteGraphWithDependenciesDoesNotCrash)
{
    TkTaskGraph graph;
    TkTaskGraphSys first;
    TkTaskGraphSys second;
    TkTaskGraphSys third;

    first.order = 10;
    second.order = 20;
    third.order = 30;

    second.addDependency(&first);
    third.addDependency(&second);

    graph.addSystem(&third);
    graph.addSystem(&first);
    graph.addSystem(&second);

    graph.execute();
    SUCCEED();
}
