// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/base/Template.hpp>

#include <cstring>
#include <new>

namespace toolkit {

//    typedef CRTMemoryPool DefaultPool;
//
//    namespace allocator_ext
//    {
//        inline void* realloc(void* ptr, u64 newSize)
//        {
//            if (newSize == 0)
//            {
//                DefaultPool::free(ptr);
//                return nullptr;
//            }
//            if (!ptr)
//                return DefaultPool::allocate(newSize);
//            void* result = DefaultPool::allocate(newSize);
//            if (result)
//            {
//                memcpy(result, ptr, newSize);
//                DefaultPool::free(ptr);
//            }
//            return result;
//        }
//
//
//        // Reallocates block of the memory.
//        inline void* reallocAligned(void* ptr, u64 newSize, u64 alignment)
//        {
//            if (newSize == 0)
//            {
//                DefaultPool::free(ptr);
//                return nullptr;
//            }
//            if (!ptr)
//                return DefaultPool::allocate(newSize, alignment);
//            void* result = DefaultPool::allocate(newSize, alignment);
//            if (result)
//            {
//                memcpy(result, ptr, newSize);
//                DefaultPool::free(ptr);
//            }
//            return result;
//        }
//
//
//        // Reallocates block of the memory.
//        inline void* realloc(void* ptr, u64 oldSize, u64 newSize)
//        {
//            if (newSize == 0)
//            {
//                DefaultPool::free(ptr);
//                return nullptr;
//            }
//            if (!ptr)
//                return DefaultPool::allocate(newSize);
//            if (newSize <= oldSize)
//                return ptr;
//            void* result = DefaultPool::allocate(newSize);
//            if (result)
//            {
//                memcpy(result, ptr, oldSize);
//                DefaultPool::free(ptr);
//            }
//            return result;
//        }
//    }
//
//    namespace memory
//    {
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<!TIsTriviallyConstructible<T>::Value>::Type constructItem(T* dst)
//        {
//            new(dst) T();
//        }
//
//        // Constructs the item in the memory.
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<TIsTriviallyConstructible<T>::Value>::Type constructItem(T* dst)
//        {
//        }
//
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<!TIsTriviallyConstructible<T>::Value>::Type constructItems(T* dst, i32 count)
//        {
//            while (count--)
//            {
//                new(dst) T();
//                ++(T*&)dst;
//            }
//        }
//
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<TIsTriviallyConstructible<T>::Value>::Type constructItems(T* dst, i32 count)
//        {
//            // More undefined behavior! No more clean memory! More performance! Yay!
//            //ZERO_MEMORY(dst, count * sizeof(T));
//        }
//
//        template<typename T, typename U>
//        TK_FORCE_INLINE typename TEnableIf<!TIsBitwiseConstructible<T, U>::Value>::Type constructItems(T* dst, const U* src, i32 count)
//        {
//            while (count--)
//            {
//                new(dst) T(*src);
//                ++(T*&)dst;
//                ++src;
//            }
//        }
//
//        template<typename T, typename U>
//        TK_FORCE_INLINE typename TEnableIf<TIsBitwiseConstructible<T, U>::Value>::Type constructItems(T* dst, const U* src, i32 count)
//        {
//            memcpy(dst, src, count * sizeof(U));
//        }
//
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<!TIsTriviallyDestructible<T>::Value>::Type destructItem(T* dst)
//        {
//            dst->~T();
//        }
//
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<TIsTriviallyDestructible<T>::Value>::Type destructItem(T* dst)
//        {
//        }
//
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<!TIsTriviallyDestructible<T>::Value>::Type destructItems(T* dst, i32 count)
//        {
//            while (count--)
//            {
//                dst->~T();
//                ++dst;
//            }
//        }
//
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<TIsTriviallyDestructible<T>::Value>::Type destructItems(T* dst, i32 count)
//        {
//        }
//
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<!TIsTriviallyCopyAssignable<T>::Value>::Type copyItems(T* dst, const T* src, i32 count)
//        {
//            while (count--)
//            {
//                *dst = *src;
//                ++dst;
//                ++src;
//            }
//        }
//
//        template<typename T>
//        TK_FORCE_INLINE typename TEnableIf<TIsTriviallyCopyAssignable<T>::Value>::Type copyItems(T* dst, const T* src, i32 count)
//        {
//            memcpy(dst, src, count * sizeof(T));
//        }
//
//        template<typename T, typename U>
//        TK_FORCE_INLINE typename TEnableIf<!TIsBitwiseConstructible<T, U>::Value>::Type moveItems(T* dst, const U* src, i32 count)
//        {
//            while (count--)
//            {
//                new(dst) T((T&&)*src);
//                ++(T*&)dst;
//                ++src;
//            }
//        }
//
//        template<typename T, typename U>
//        TK_FORCE_INLINE typename TEnableIf<TIsBitwiseConstructible<T, U>::Value>::Type moveItems(T* dst, const U* src, i32 count)
//        {
//            memcpy(dst, src, count * sizeof(U));
//        }
//    }
//
//    // Creates a new object of the given type.
//    template<class T, class MemoryPool = DefaultPool>
//    inline T* newObject()
//    {
//        T* ptr = (T*)MemoryPool::allocate(sizeof(T));
//        memory::constructItem(ptr);
//        return ptr;
//    }
//    template<class T>
//    inline T* newObject(IAllocator& alloc)
//    {
//        T* ptr = (T*)alloc.allocate(sizeof(T));
//        memory::constructItem(ptr);
//        return ptr;
//    }
//
//    // Creates a new object of the given type with the specified parameters.
//    template<class T, class MemoryPool = DefaultPool, class... Args>
//    inline T* newObject(Args&&...args)
//    {
//        T* ptr = (T*)MemoryPool::allocate(sizeof(T));
//        new(ptr) T(forward<Args>(args)...);
//        return ptr;
//    }
//    template<class T, class... Args>
//    inline T* newObject(IAllocator& alloc, Args&&...args)
//    {
//        T* ptr = (T*)alloc.allocate(sizeof(T));
//        new(ptr) T(forward<Args>(args)...);
//        return ptr;
//    }
//
//    // Creates and constructs an array of given elements count.
//    template<class T, class MemoryPool = DefaultPool>
//    inline T* newArray(u32 count)
//    {
//        T* ptr = (T*)MemoryPool::allocate(sizeof(T) * count);
//        memory::constructItems(ptr, count);
//        return ptr;
//    }
//    template<class T>
//    inline T* newArray(IAllocator& alloc, u32 count)
//    {
//        T* ptr = (T*)alloc.allocate(sizeof(T) * count);
//        memory::constructItems(ptr, count);
//        return ptr;
//    }
//
//    // Destructs and frees the specified object.
//    template<class T, class MemoryPool = DefaultPool>
//    inline void deletePtr(T* ptr)
//    {
//        memory::destructItem(ptr);
//        MemoryPool::free(ptr);
//    }
//    template<class T>
//    inline void deletePtr(IAllocator& alloc, T* ptr)
//    {
//        memory::destructItem(ptr);
//        alloc.deallocate(ptr);
//    }
//
//    // Destructs and frees the specified array of objects.
//    template<class T, class MemoryPool = DefaultPool>
//    inline void deleteArray(T* ptr, u32 count)
//    {
//        memory::destructItems(ptr, count);
//        MemoryPool::free(ptr);
//    }
//    template<class T>
//    inline void deleteArray(IAllocator& alloc, T* ptr, u32 count)
//    {
//        memory::destructItems(ptr, count);
//        alloc.deallocate(ptr);
//    }
//
//
//    template <typename T, typename Allocator = DefaultPool>
//    struct CLocal
//    {
//        ~CLocal() {
//            if (obj) obj->~T();
//        }
//
//        void operator =(const CLocal&) = delete;
//
//        template <typename... Args>
//        void create(Args&&... args) {
//            CB_ASSERT(!obj);
//            obj = newObject<T, Allocator>(static_cast<Args&&>(args)...);
//        }
//
//        void destroy() {
//            CB_ASSERT(obj);
//            obj->~T();
//            obj = nullptr;
//        }
//
//        T& operator*() { CB_ASSERT(obj); return *obj; }
//        T* operator->() const { CB_ASSERT(obj); return obj; }
//        T* get() const { return obj; }
//
//    private:
//        alignas(T) u8 mem[sizeof(T)];
//        T* obj = nullptr;
//    };
//
//
//    template <typename T, typename Allocator = DefaultPool>
//    struct CUniquePtr
//    {
//        CUniquePtr()
//            : m_ptr(nullptr)
//        {
//        }
//
//        CUniquePtr(T* obj)
//            : m_ptr(obj)
//        {
//        }
//
//        template <typename T2>
//        CUniquePtr(CUniquePtr<T2>&& rhs)
//        {
//            *this = static_cast<CUniquePtr<T2>&&>(rhs);
//        }
//
//        ~CUniquePtr() {
//            if (m_ptr) {
//                deletePtr<T, Allocator>(m_ptr);
//            }
//        }
//
//        CUniquePtr(const CUniquePtr& rhs) = delete;
//        void operator=(const CUniquePtr& rhs) = delete;
//
//        template <typename T2>
//        void operator=(CUniquePtr<T2>&& rhs) {
//            if (m_ptr) {
//                deletePtr<T, Allocator>(m_ptr);
//            }
//            // m_allocator = rhs.getAllocator();
//            m_ptr = static_cast<T*>(rhs.detach());
//        }
//
//        template <typename... Args> static CUniquePtr<T> create(CUniquePtr& allocator, Args&&... args) {
//            return CUniquePtr<T>(newObject<T, Allocator>(forward(args)...));
//        }
//
//        T* detach() {
//            T* res = m_ptr;
//            m_ptr = nullptr;
//            return res;
//        }
//
//        CUniquePtr&& move() { return static_cast<CUniquePtr&&>(*this); }
//
//        void reset() {
//            if (m_ptr) {
//                deletePtr<T, Allocator>(m_ptr);
//            }
//            m_ptr = nullptr;
//        }
//
//        T* get() const { return m_ptr; }
//        T& operator *() const { CB_ASSERT(m_ptr); return *m_ptr; }
//        T* operator ->() const { CB_ASSERT(m_ptr); return m_ptr; }
//        operator bool() const { return m_ptr; }
//
//    private:
//        T* m_ptr = nullptr;
//    };

}