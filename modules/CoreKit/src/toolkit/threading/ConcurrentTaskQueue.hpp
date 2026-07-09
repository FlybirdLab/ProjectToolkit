/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/ConcurrentQueue.hpp>
#include <toolkit/threading/Task.hpp>

namespace toolkit
{
    // Lock-free implementation of thread-safe tasks queue.
    template <typename T = TkTask>
    class ConcurrentTaskQueue : public TkConcurrentQueue<T *>
    {
    public:
        // Add item to the collection (thread-safe).
        TK_FORCE_INLINE void Add(T *item)
        {
            TkConcurrentQueue<T *>::enqueue(item);
        }

        // Cancels all the tasks from the queue and removes them.
        void cancelAll()
        {
            T *tasks[16];
            std::size_t count;
            while ((count = TkConcurrentQueue<T *>::try_dequeue_bulk(tasks, lengthOf(tasks))) != 0)
            {
                for (std::size_t i = 0; i != count; i++)
                {
                    tasks[i]->cancel();
                }
            }
        }
    };

}