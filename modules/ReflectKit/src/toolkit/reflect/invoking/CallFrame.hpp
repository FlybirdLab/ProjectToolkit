/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit
{
    class CallFrame
    {
        NON_COPYABLE(CallFrame);

    public:
        // CallFrame owns temporary argument values, pointer cells, and value-return storage for one call.
        // Any ReturnValue::data pointing into the frame is valid until the frame is reset/destroyed.
        CallFrame();
        ~CallFrame();

        void* allocate(psize size, psize align);

        template <class T, class... Args>
        T* emplace(Args&&... args)
        {
            T* storage = static_cast<T*>(allocate(sizeof(T), alignof(T)));
            new (storage) T(toolkit::forward<Args>(args)...);
            if constexpr (!TTriviallyDestructibleType<T>)
            {
                addDestructor(storage, [](void* ptr)
                {
                    static_cast<T*>(ptr)->~T();
                });
            }
            return storage;
        }

        void addDestructor(void* ptr, void (*destroy)(void*));
        void reset();

    private:
        struct DestructorRecord
        {
            void* ptr = nullptr;
            void (*destroy)(void*) = nullptr;
        };

        struct HeapBlock
        {
            void* ptr = nullptr;
        };

        static constexpr psize kInlineCapacity = 1024;

    private:
        alignas(16) byte m_inlineBuffer[kInlineCapacity];
        psize m_inlineOffset = 0;
        TkArray<DestructorRecord> m_destructors;
        TkArray<HeapBlock> m_heapBlocks;
    };
}
