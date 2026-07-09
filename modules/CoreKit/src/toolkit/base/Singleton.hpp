/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit {
    
#if TK_DEBUG
    extern i32 g_singletonsAllocated;
#endif

    // If class inherits that it will become a singleton.
    template<typename T>
    class MakeSingleton
    {
    public:
        TK_FORCE_INLINE static T& getSingleton()
        {
            TK_ASSERT(m_initialized);
            return *reinterpret_cast<T*>(m_global);
        }

        template<typename... TArgs>
        static T& allocateSingleton(TArgs... args)
        {
            TK_ASSERT(!m_initialized);
            ::new(m_global) T(args...);
            m_initialized = true;
#if TK_DEBUG
            ++g_singletonsAllocated;
#endif
            return *reinterpret_cast<T*>(m_global);
        }

        static void freeSingleton()
        {
            if (m_initialized)
            {
                reinterpret_cast<T*>(m_global)->~T();
                m_initialized = false;
#if TK_DEBUG
                --g_singletonsAllocated;
#endif
            }
        }

        static bool isAllocated()
        {
            return m_initialized;
        }

    private:
        static u8 m_global[];
        static inline bool m_initialized = false;
    };


    template<typename T>
    alignas(TK_SAFE_ALIGNMENT) u8 MakeSingleton<T>::m_global[sizeof(T)];


    // If class inherits that it will become a singleton.
    template<typename T>
    class MakeSingletonPtr
    {
    public:
        TK_FORCE_INLINE static T& getSingleton()
        {
            return *m_global;
        }

        template<typename... TArgs>
        static T& allocateSingleton(TArgs... args)
        {
            TK_ASSERT(m_global == nullptr);
            m_global = new T(args...);

#if TK_DEBUG
            ++g_singletonsAllocated;
#endif

            return *m_global;
        }

        static void freeSingleton()
        {
            if (m_global)
            {
                delete m_global;
                m_global = nullptr;
#if TK_DEBUG
                --g_singletonsAllocated;
#endif
            }
        }

        static bool isAllocated()
        {
            return m_global != nullptr;
        }

    private:
        static inline T* m_global = nullptr;
    };


    // If class inherits that it will become a singleton. This is a simple version without a need for init.
    template<typename T>
    class MakeSingletonSimple
    {
    public:
        TK_FORCE_INLINE static T& getSingleton()
        {
            return m_global;
        }

    private:
        static T m_global;
    };

    template<typename T>
    T MakeSingletonSimple<T>::m_global;


    // If class inherits that it will become a singleton. This is a simple version with implicit init.
    template<typename T>
    class MakeSingletonLazyInit
    {
    public:
        TK_FORCE_INLINE static T& getSingleton()
        {
            if (m_global == nullptr)
            {
                m_global = new T;
            }
            return *m_global;
        }

    private:
        static inline T* m_global = nullptr;
    };

}