/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Reference:
 *    https://github.com/cameron314/concurrentqueue
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/memory/Memory.hpp>
#define MOODYCAMEL_EXCEPTIONS_ENABLED 0
#include <concurrentqueue.h>

namespace toolkit
{

    // The default engine configuration for concurrentqueue.
    struct ConcurrentQueueSettings : public moodycamel::ConcurrentQueueDefaultTraits
    {
        // Use bigger blocks
        static const size_t BLOCK_SIZE = 256;

        // Use default engine memory allocator
        static inline void *malloc(size_t size)
        {
            return TkMemory::allocate(TkMemory::getDefault(), (u64)size);
        }

        // Use default engine memory allocator
        static inline void free(void *ptr)
        {
            return TkMemory::free(TkMemory::getDefault(), ptr);
        }
    };

    // Lock-free implementation of thread-safe queue.
    template <typename T>
    class TkConcurrentQueue : public moodycamel::ConcurrentQueue<T, ConcurrentQueueSettings>
    {
    public:
        typedef moodycamel::ConcurrentQueue<T, ConcurrentQueueSettings> Base;

    public:
        // Get an estimate of the total number of elements currently in the queue.
        TK_FORCE_INLINE i32 count() const
        {
            return static_cast<i32>(Base::size_approx());
        }

        // Add item to the collection.
        TK_FORCE_INLINE void add(const T &item)
        {
            Base::enqueue(item);
        }

        // Add item to the collection.
        TK_FORCE_INLINE void add(T &&item)
        {
            Base::enqueue(moveTemp(item));
        }
    };

}
