/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "TaskGraph.hpp"

#include "JobSys.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/utils/Sorting.hpp"

namespace toolkit {
    
    static bool sortTaskGraphSys(TkTaskGraphSys* const& a, TkTaskGraphSys* const& b)
    {
        return b->order > a->order;
    };

    TkTaskGraphSys::~TkTaskGraphSys()
    {
        // Cleanup any outstanding dependencies
        for (auto* e : m_reverseDependencies)
            e->m_dependencies.remove(this);
    }


    void TkTaskGraphSys::addDependency(TkTaskGraphSys* system)
    {
        TK_CHECK(system);
        if (m_dependencies.contains(system))
            return;
        system->m_reverseDependencies.add(this);
        m_dependencies.add(system);
    }


    void TkTaskGraphSys::removeDependency(TkTaskGraphSys* system)
    {
        TK_CHECK(system);
        if (!m_dependencies.contains(system))
            return;
        system->m_reverseDependencies.remove(this);
        m_dependencies.remove(system);
    }


    void TkTaskGraphSys::preExecute(TkTaskGraph* graph)
    {
    }


    void TkTaskGraphSys::execute(TkTaskGraph* graph)
    {
    }


    void TkTaskGraphSys::postExecute(TkTaskGraph* graph)
    {
    }

    const TkStaticArray<TkTaskGraphSys*, 64>& TkTaskGraph::getSystems() const
    {
        return m_systems;
    }


    void TkTaskGraph::addSystem(TkTaskGraphSys* system)
    {
        m_systems.add(system);
    }
    

    void TkTaskGraph::removeSystem(TkTaskGraphSys* system)
    {
        m_systems.remove(system);
    }


    void TkTaskGraph::execute()
    {
        PROFILE();

        for (auto system : m_systems)
            system->preExecute(this);

        m_queue.clear();
        m_remaining.clear();
        m_remaining.add(m_systems);

        while (m_remaining.hasItems())
        {
            // Find systems without dependencies or with already executed dependencies
            for (i32 i = m_remaining.count() - 1; i >= 0; i--)
            {
                auto e = m_remaining[i];
                bool hasReadyDependencies = true;
                for (auto d : e->m_dependencies)
                {
                    if (m_remaining.contains(d))
                    {
                        hasReadyDependencies = false;
                        break;
                    }
                }
                if (hasReadyDependencies)
                {
                    m_queue.add(e);
                    m_remaining.removeAt(i);
                }
            }

            // End if no systems left
            if (m_queue.isEmpty())
                break;

            // Execute in order
            TkSorting::quickSort(m_queue.get(), m_queue.count(), &sortTaskGraphSys);
            TkJobSys::setJobStartingOnDispatch(false);
            m_labels.clear();
            for (i32 i = 0; i < m_queue.count(); i++)
            {
                m_currentSystem = m_queue[i];
                m_currentSystem->execute(this);
            }
            m_currentSystem = nullptr;
            m_queue.clear();

            // Wait for async jobs to finish
            TkJobSys::setJobStartingOnDispatch(true);
            for (const i64 label : m_labels)
                TkJobSys::wait(label);
        }

        for (auto system : m_systems)
            system->postExecute(this);
    }


    void TkTaskGraph::dispatchJob(const TkFunc<void(i32)>& job, i32 jobCount)
    {
        TK_ASSERT(m_currentSystem);
        const i64 label = TkJobSys::dispatch(job, jobCount);
        m_labels.add(label);
    }
}