/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/graphics/GfxResource.hpp>

namespace toolkit
{
    // Represent a GPU query that measures execution time of GPU operations.
    // The query will measure any GPU operations that take place between its Begin() and End() calls.
    class TK_API GfxTimerQuery : public GfxResource
    {
    public:
        virtual ~GfxTimerQuery() = default;

    public:
        // Start the counter.
        virtual void begin() = 0;
        // Stop the counter. Can be called more than once without failing.
        virtual void end() = 0;
        // Determine whether this query has been completed and has valid result to gather.
        virtual bool hasResult() = 0;
        // Get the query result time (in milliseconds) it took to execute GPU commands between Begin/End calls.
        virtual float getResult() = 0;

        GfxResType getType() const final override
        {
            return GfxResType::kQuery;
        }
    };

}