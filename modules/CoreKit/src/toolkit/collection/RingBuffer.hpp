/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by Flybim_rdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/memory/Memory.hpp>
#include <toolkit/base/Math.hpp>
#include <toolkit/memory/ContainerMemory.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/types/Atomic.hpp>
#include <toolkit/threading/Sync.hpp>

namespace toolkit
{

    // Template for ring buffer with variable capacity.
    template <typename T>
    class TkRingBuffer
    {
    private:
        i32 m_front = 0, m_back = 0, m_count = 0, m_capacity = 0;
        T *m_data = nullptr;
        IAllocator &m_alloc;

    public:
        TkRingBuffer(IAllocator &alloc = TkMemory::getDefault())
            : m_alloc{alloc}
        {
        }
        ~TkRingBuffer()
        {
            destructAll();
        }

        TK_FORCE_INLINE T *get()
        {
            return m_data;
        }

        TK_FORCE_INLINE i32 count() const
        {
            return m_count;
        }

        TK_FORCE_INLINE i32 capacity() const
        {
            return m_capacity;
        }

        void pushBack(const T &data)
        {
            if (m_capacity == 0 || m_capacity == m_count)
            {
                const i32 capacity = calCapacityGrow(m_alloc, m_capacity, m_count + 1);
                T *alloc = (T*)TkMemory::allocate(m_alloc, capacity * sizeof(T));
                const i32 frontCount = TkMath::min(m_capacity - m_front, m_count);
                TkMemory::moveItems(alloc, m_data + m_front, frontCount);
                TkMemory::destructItems(m_data + m_front, frontCount);
                const i32 backCount = m_count - frontCount;
                TkMemory::moveItems(alloc + frontCount, m_data, backCount);
                TkMemory::destructItems(m_data, backCount);
                toolkit::swap(m_data, alloc);
                m_front = 0;
                m_back = m_count;
                m_capacity = capacity;

                if (alloc)
                    TkMemory::free(m_alloc, alloc); // TODO check it
            }
            TkMemory::constructItems(m_data + m_back, &data, 1);
            m_back = (m_back + 1) % m_capacity;
            ++m_count;
        }

        TK_FORCE_INLINE T &peekFront()
        {
            return m_data[m_front];
        }

        TK_FORCE_INLINE const T &peekFront() const
        {
            return m_data[m_front];
        }

        TK_FORCE_INLINE T &operator[](const i32 index)
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[(m_front + index) % m_capacity];
        }

        TK_FORCE_INLINE const T &operator[](const i32 index) const
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[(m_front + index) % m_capacity];
        }

        void popFront()
        { 
            TK_ASSERT(m_count > 0);
            TkMemory::destructItems(m_data + m_front, 1);
            m_front = (m_front + 1) % m_capacity;
            --m_count;
        }

        void clear()
        {
            destructAll();
            m_front = m_back = m_count = 0;
        }

        void release()
        {
            destructAll();
            TkMemory::free(m_alloc, m_data);
            m_data = nullptr;
            m_capacity = 0;
        }

        void destructAll()
        {
            if (m_count == 0)
                return;

            const i32 frontCount = TkMath::min(m_capacity - m_front, m_count);
            TkMemory::destructItems(m_data + m_front, frontCount);

            const i32 backCount = m_count - frontCount;
            if (backCount > 0)
                TkMemory::destructItems(m_data, backCount);
        }
    };


    
    template <typename T, u32 CAPACITY>
    class TkSafeRingBuffer 
    {
    public:
        TkSafeRingBuffer()
            : m_fallback()
        {
            static_assert(CAPACITY > 2);
            for (u32 i = 0; i < CAPACITY; ++i) {
                m_objects[i].seq = i;
            }
            TSync::memoryBarrier();
        }


        TK_FORCE_INLINE bool pop(T& obj) 
        {
            i32 pos = m_rd.load();
            Item* j;
            for (;;) {
                j = &m_objects[pos % CAPACITY];
                const i32 seq = j->seq;
                if (seq < pos + 1) {
                    return false;
                }
                else if (seq == pos + 1) {
                    auto to = pos + 1;
                    if (m_rd.compareExchange(to, pos)) break;
                }
                else {
                    pos = m_rd.load();
                }
            }
            obj = j->value;
            j->seq = pos + CAPACITY;
            return true;
        }


        TK_FORCE_INLINE void push(const T& obj, TkMutex* mutex) 
        {
            i32 pos = m_wr.load();
            Item* j;
            for (;;) {
                j = &m_objects[pos % CAPACITY];
                const i32 seq = j->seq;
                if (seq < pos) {
                    // buffer full
                    if (mutex) mutex->lock();
                    m_fallback.push(obj);
                    if (mutex) mutex->unlock();
                    return;
                }
                else if (seq == pos) {
                    // we can try to push
                    auto to = pos + 1;
                    if (m_wr.compareExchange(to, pos)) break;
                }
                else {
                    // somebody pushed before us, try again
                    pos = m_wr.load();
                }
            }
            j->value = obj;
            j->seq = pos + 1;
        }


        bool popSecondary(T& obj) 
        {
            if (m_fallback.isEmpty()) return false;
            obj = m_fallback.back();
            m_fallback.pop();
            return true;
        }
        
    private:
        struct Item {
            T value;
            volatile i32 seq;
        };

        Item m_objects[CAPACITY];
        TkAtomic<i32> m_rd {0};
        TkAtomic<i32> m_wr {0};
        TkArray<T> m_fallback;
    };

}