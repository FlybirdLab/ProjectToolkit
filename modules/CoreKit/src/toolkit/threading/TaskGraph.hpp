/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/collection/StaticArray.hpp>

namespace toolkit {
    
    class TkTaskGraph;

    // System that can generate work into Task Graph for asynchronous execution.
    class TkTaskGraphSys final
    {
        friend class TkTaskGraph;
    public:
        // The execution order of the system (systems with higher order are executed later, lower first).
        i32 order = 0;

    public:
        ~TkTaskGraphSys();

        // Adds the dependency on the system execution. Before this system can be executed the given dependant system has to be executed first.
        void addDependency(TkTaskGraphSys* system);
        // Removes the dependency on the system execution.
        void removeDependency(TkTaskGraphSys* system);
        // Called before executing any systems of the graph. Can be used to initialize data (synchronous).
        virtual void preExecute(TkTaskGraph* graph);
        // Executes the system logic and schedules the asynchronous work.
        virtual void execute(TkTaskGraph* graph);
        // Called after executing all systems of the graph. Can be used to cleanup data (synchronous).
        virtual void postExecute(TkTaskGraph* graph);

    private:
        TkStaticArray<TkTaskGraphSys*, 16> m_dependencies;
        TkStaticArray<TkTaskGraphSys*, 16> m_reverseDependencies;
    };


    // Graph-based asynchronous tasks scheduler for high-performance computing and processing.
    class TkTaskGraph final
    {
    public:
        // Gets the list of systems.
        const TkStaticArray<TkTaskGraphSys*, 64>& getSystems() const;
        // Adds the system to the graph for the execution.
        void addSystem(TkTaskGraphSys* system);
        // Removes the system from the graph.
        void removeSystem(TkTaskGraphSys* system);
        // Schedules the asynchronous systems execution including ordering and dependencies handling.
        void execute();
        // Dispatches the job for the execution.
        void dispatchJob(const TkFunc<void(i32)>& job, i32 jobCount = 1);

    private:
        TkStaticArray<TkTaskGraphSys*, 64> m_systems;
        TkStaticArray<TkTaskGraphSys*, 64> m_remaining;
        TkStaticArray<TkTaskGraphSys*, 64> m_queue;
        TkStaticArray<i64, 64> m_labels;
        TkTaskGraphSys* m_currentSystem = nullptr;
    };
}