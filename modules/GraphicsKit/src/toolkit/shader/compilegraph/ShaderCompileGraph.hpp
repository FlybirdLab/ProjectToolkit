/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/shader/compilegraph/ShaderCompileJob.hpp>

namespace toolkit
{
    // Container of pending shader compile jobs. v1 is a flat list -- a real
    // graph (with dependencies between jobs from `#include`s or inter-stage
    // reflection sharing) lands in a later phase once we wire DependencyScanner
    // results into ShaderManifestCompiler.
    class ShaderCompileGraph
    {
    public:
        void addJob(const ShaderCompileJob &job) { m_jobs.add(job); }
        const TkArray<ShaderCompileJob> &jobs() const { return m_jobs; }
        TkArray<ShaderCompileJob> &jobs() { return m_jobs; }
        void clear() { m_jobs.clear(); }
        i32 count() const { return m_jobs.count(); }

    private:
        TkArray<ShaderCompileJob> m_jobs;
    };
}
