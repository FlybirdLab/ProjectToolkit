/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {
    
    // Lightweight multi-threaded jobs execution scheduler. Uses a pool of threads and supports work-stealing concept.
    class TkJobSys final
    {
    public:
        // Executes the job (utility to call dispatch and wait for the end).
        static void execute(const TkFunc<void(i32)>& job, i32 jobCount = 1);
        // Dispatches the job for the execution.
        static i64 dispatch(const TkFunc<void(i32)>& job, i32 jobCount = 1);
        // Dispatches the job for the execution after all of dependant jobs will complete.
        static i64 dispatch(const TkFunc<void(i32)>& job, TkSpan<i64> dependencies, i32 jobCount = 1);

    public:
        // Waits for all dispatched jobs to finish.
        static void wait();
        // Waits for all dispatched jobs until a given label to finish (i.e. waits for a Dispatch that returned that label).
        static void wait(i64 label);

    public:
        // Sets whether automatically start jobs execution on Dispatch. If disabled jobs won't be executed until it gets re-enabled. Can be used to optimize execution of multiple dispatches that should overlap.
        static void setJobStartingOnDispatch(bool value);
        // Gets the amount of job system threads.
        static i32 getThreadsCount();
    
    public:
        static bool init();
        static void beforeExit();
        static void dispose();
    };
}