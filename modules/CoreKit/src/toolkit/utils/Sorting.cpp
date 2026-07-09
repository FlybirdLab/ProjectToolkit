/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Sorting.hpp"

#include "toolkit/threading/ThreadLocal.hpp"

namespace toolkit {
    // Use a cached storage for the sorting (one per thread to reduce locking)
    ThreadLocal<TkSorting::Stack*> s_stacks {};


    TkSorting::Stack& TkSorting::Stack::get()
    {
        Stack*& stack = s_stacks.get();
        if (!stack)
            stack = TkMemory::newObject<Stack>(TkMemory::getDefault());
        return *stack;
    }


    TkSorting::Stack::Stack() = default;
    TkSorting::Stack::~Stack()
    {
        TkMemory::free(TkMemory::getDefault(), data);
    }


    void TkSorting::Stack::setCapacity(const i32 capacity)
    {
        TK_ASSERT(capacity >= 0);
        if (capacity == this->capacity)
            return;
        i32* newData = nullptr;
        if (capacity > 0)
            newData = (i32*)TkMemory::allocate(TkMemory::getDefault(), capacity * sizeof(i32));
        const i32 newCount = count < capacity ? count : capacity;
        if (data)
        {
            if (newData && newCount)
                memcpy(newData, data, newCount * sizeof(i32));
            TkMemory::free(TkMemory::getDefault(), data);
        }
        data = newData;
        this->capacity = capacity;
        count = newCount;
    }


    void TkSorting::Stack::ensureCapacity(i32 minCapacity)
    {
        if (capacity >= minCapacity)
            return;
        i32 num = capacity == 0 ? 64 : capacity * 2;
        if (num < minCapacity)
            num = minCapacity;
        setCapacity(num);
    }
}