/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/compilegraph/CompilerScheduler.hpp>

#include <toolkit/debug/Logger.hpp>

namespace toolkit
{
    void CompilerScheduler::setJobCount(u32 jobs)
    {
        if (jobs == 0)
            jobs = 1;
        if (jobs > 1)
        {
            LogWarn("[CompilerScheduler] --jobs=%u requested but parallel compile "
                    "is not enabled yet; falling back to serial execution.", jobs);
        }
        m_jobCount = jobs;
    }
}
