/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/collection/StaticArray.hpp"
#include "toolkit/types/String.hpp"
#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/base/Math.hpp>

#if TK_TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace toolkit
{

#if TK_DEBUG

    // Represents single CPU profiling event data.
    struct DebugEvent
    {
        // The start time (in milliseconds).
        f64 start;
        // The end time (in milliseconds).
        f64 end;
        // The event depth. Value 0 is used for the root event.
        i32 depth;
        // The native dynamic memory allocation size during this event (excluding the child events). Given value is in bytes.
        i32 heapMemAlloc;
        char name[100];
    };

    // Implements simple profiling events ring-buffer.
    class DebugEventBuffer
    {
    private:
        DebugEvent *m_data;
        i32 m_capacity;
        i32 m_capacityMask;
        i32 m_head;
        i32 m_count;

    public:
        DebugEventBuffer();
        ~DebugEventBuffer();

    public:
        // Gets the amount of the events in the buffer.
        TK_FORCE_INLINE i32 getCount() const
        {
            return m_count;
        }

        // Gets the event at the specified index.
        DebugEvent &get(i32 index) const
        {
            TK_ASSERT(index >= 0 && index < m_capacity);
            return m_data[index];
        }

        // Adds new event to the buffer.
        i32 add()
        {
            const i32 index = m_head;
            m_head = (m_head + 1) & m_capacityMask;
            m_count = TkMath::min(m_count + 1, m_capacity);
            return index;
        }

        // Extracts the buffer data (only ended events starting from the root level with depth=0).
        void extract(TkArray<DebugEvent> &data, bool withRemoval);

    public:
        // Ring buffer iterator
        struct Iterator
        {
            friend class DebugEventBuffer;

        private:
            DebugEventBuffer *m_buffer;
            i32 m_index;

            TK_FORCE_INLINE Iterator(DebugEventBuffer *buffer, const i32 index)
                : m_buffer(buffer), m_index(index)
            {
            }

        public:
            TK_FORCE_INLINE Iterator(const Iterator &other)
                : m_buffer(other.m_buffer), m_index(other.m_index)
            {
            }

            TK_FORCE_INLINE Iterator(Iterator &&other) noexcept
                : m_buffer(other.m_buffer), m_index(other.m_index)
            {
            }

            TK_FORCE_INLINE Iterator &operator=(Iterator &&other)
            {
                m_buffer = other.m_buffer;
                m_index = other.m_index;
                return *this;
            }

            TK_FORCE_INLINE Iterator &operator=(const Iterator &other)
            {
                m_buffer = other.m_buffer;
                m_index = other.m_index;
                return *this;
            }

            TK_FORCE_INLINE i32 index() const
            {
                return m_index;
            }

            TK_FORCE_INLINE DebugEvent &event() const
            {
                TK_ASSERT(m_buffer && m_index >= 0 && m_index < m_buffer->m_capacity);
                return m_buffer->get(m_index);
            }

            TK_FORCE_INLINE bool isEnd() const
            {
                return m_index == m_buffer->m_head;
            }

            TK_FORCE_INLINE bool isNotEnd() const
            {
                return m_index != m_buffer->m_head;
            }

            TK_FORCE_INLINE bool operator==(const Iterator &v) const
            {
                return m_buffer == v.m_buffer && m_index == v.m_index;
            }

            TK_FORCE_INLINE bool operator!=(const Iterator &v) const
            {
                return m_buffer != v.m_buffer || m_index != v.m_index;
            }

        public:
            TK_FORCE_INLINE Iterator &operator++()
            {
                m_index = (m_index + 1) & m_buffer->m_capacityMask;
                return *this;
            }

            TK_FORCE_INLINE Iterator operator++(int)
            {
                Iterator temp = *this;
                m_index = (m_index + 1) & m_buffer->m_capacityMask;
                return temp;
            }

            TK_FORCE_INLINE Iterator &operator--()
            {
                m_index = (m_index - 1) & m_buffer->m_capacityMask;
                return *this;
            }

            TK_FORCE_INLINE Iterator operator--(int)
            {
                Iterator temp = *this;
                m_index = (m_index - 1) & m_buffer->m_capacityMask;
                return temp;
            }
        };

    public:
        TK_FORCE_INLINE Iterator begin()
        {
            return Iterator(this, (m_head - m_count) & m_capacityMask);
        }

        TK_FORCE_INLINE Iterator last()
        {
            TK_ASSERT(m_count > 0);
            return Iterator(this, (m_head - 1) & m_capacityMask);
        }

        TK_FORCE_INLINE Iterator end()
        {
            return Iterator(this, m_head);
        }
    };


    // Thread registered for profiling. Holds events data with read/write support.
    class DebugThread
    {
    private:
        TkString m_name;
        i32 m_depth = 0;

    public:
        DebugThread(const char *name)
        {
            m_name = name;
        }

        DebugThread(const TkString &name)
        {
            m_name = name;
        }

    public:
        // The current thread.
        static TK_THREAD_LOCAL DebugThread *Current;

    public:
        // Gets the name.
        TK_FORCE_INLINE const TkString &getName() const
        {
            return m_name;
        }

        // The events buffer.
        DebugEventBuffer Buffer;

    public:
        // Begins the event running on a this thread. Call EndEvent with index parameter equal to the returned value by BeginEvent function.
        i32 beginEvent();
        // Ends the event running on a this thread.
        void endEvent(i32 index);
        // Ends the last event running on a this thread.
        void endEvent();
    };

    // Provides CPU performance measuring methods.
    class TkProfiler
    {
    public:
        // The registered threads.
        static TkStaticArray<DebugThread*, 64> Threads;
        // The profiling tools usage flag. Can be used to disable profiler. Engine turns it down before the exit and before platform startup.
        static bool Enabled;

    public:
        // Determines whether the current (calling) thread is being profiled by the service (it may has no active profile block but is registered).
        static bool isProfilingCurrentThread();
        // Gets the current thread (profiler service shadow object).
        static DebugThread *getCurrentThread();
        // Begins the event. Call EndEvent with index parameter equal to the returned value by BeginEvent function.
        static i32 beginEvent();
        // Begins the event. Call EndEvent with index parameter equal to the returned value by BeginEvent function.
        static i32 beginEvent(const char *name);
        // Ends the event.
        static void endEvent(i32 index);
        // Ends the last event.
        static void endEvent();
        // Releases resources. Calls to the profiling API after Dispose are not valid.
        static void dispose();
    };

    // Helper structure used to call BeginEvent/EndEvent within single code block.
    struct ScopeProfileBlockCPU
    {
        i32 Index;

        TK_FORCE_INLINE ScopeProfileBlockCPU(const char *name)
        {
            Index = TkProfiler::beginEvent(name);
        }

        TK_FORCE_INLINE ~ScopeProfileBlockCPU()
        {
            TkProfiler::endEvent(Index);
        }
    };

    template <>
    struct TIsPODType<DebugEvent>
    {
        enum
        {
            Value = true
        };
    };
    
    struct TK_API SrcLocData
    {
        const char* name;
        const char* function;
        const char* file;
        u32 line;
        u32 color;
    };

    // Shortcut macros for profiling a single code block execution on CPU
    // Use ZoneTransient for Tracy for code that can be hot-reloaded (eg. in Editor) or if name can be a variable

#endif
}

namespace toolkit {
#if TK_DEBUG && TK_TRACY_ENABLE

#define PROFILE_USE_TRANSIENT_DATA 0

#if PROFILE_USE_TRANSIENT_DATA
#define PROFILE()                              \
    ZoneTransient(___tracy_scoped_zone, true); \
    ScopeProfileBlockCPU ProfileBlockCPU(__FUNCTION__)
#define PROFILE_NAMED(name)                           \
    ZoneTransientN(___tracy_scoped_zone, name, true); \
    ScopeProfileBlockCPU ProfileBlockCPU(name)
#else
#define PROFILE()                          \
    ZoneNamed(___tracy_scoped_zone, true); \
    ScopeProfileBlockCPU ProfileBlockCPU(__FUNCTION__)
#define PROFILE_NAMED(name)                       \
    ZoneNamedN(___tracy_scoped_zone, name, true); \
    ScopeProfileBlockCPU ProfileBlockCPU(name)
#endif

#if TK_TRACY_ENABLE
#define PROFILE_SRC_LOC(srcLoc)                                                     \
    tracy::ScopedZone ___tracy_scoped_zone((tracy::SourceLocationData *)&(srcLoc)); \
    ScopeProfileBlockCPU ProfileBlockCPU((srcLoc).name)
#define PROFILE_ASSET(asset)                                  \
    ZoneScoped;                                               \
    const StringView __tracy_assetm_name((asset)->GetPath()); \
    ZoneName(*__tracy_assetm_name, __tracy_assetm_name.Length())
#define PROFILE_ACTOR(actor)                                  \
    ZoneScoped;                                               \
    const StringView __tracy_actorm_name((actor)->GetName()); \
    ZoneName(*__tracy_actorm_name, __tracy_actorm_name.Length())
#else
#define PROFILE_SRC_LOC(srcLoc) ScopeProfileBlockCPU ProfileBlockCPU((srcLoc).name)
#define PROFILE_ASSET(asset)
#define PROFILE_ACTOR(actor)
#endif

#else
    // Empty macros for disabled profiler
#define PROFILE()
#define PROFILE_NAMED(name)
#define PROFILE_SRC_LOC(srcLoc)
#define PROFILE_ASSET(asset)
#define PROFILE_ACTOR(actor)
#endif

}