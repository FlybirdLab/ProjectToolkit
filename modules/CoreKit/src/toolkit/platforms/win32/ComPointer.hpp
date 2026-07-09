/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_WINDOWS

#define UNICODE 1
#include <windows.h>
#include "toolkit/base/TypeTraits.hpp"

namespace toolkit
{
    // Helper object that makes IUnknown methods private.
    template <typename T>
    class RemoveIUnknownBase : public T
    {
    public:
        ~RemoveIUnknownBase() = default;

        // STDMETHOD macro implies virtual.
        // ComPtr can be used with any class that implements the 3 methods of IUnknown.
        // ComPtr does not require these methods to be virtual.
        // When ComPtr is used with a class without a virtual table, marking the functions
        // as virtual in this class adds unnecessary overhead.
        HRESULT __stdcall queryInterface(REFIID riid, _COM_Outptr_ void **ppvObject);
        ULONG __stdcall addRef();
        ULONG __stdcall release();

        template <class Q>
        HRESULT __stdcall queryInterface(_COM_Outptr_ Q **pp)
        {
            return queryInterface(__uuidof(Q), (void **)pp);
        }
    };

    template <typename T>
    struct RemoveIUnknown
    {
        typedef RemoveIUnknownBase<T> ReturnType;
    };

    template <typename T>
    struct RemoveIUnknown<const T>
    {
        typedef const RemoveIUnknownBase<T> ReturnType;
    };

    template <typename T>
    class ComPtr
    {
    public:
        typedef T InterfaceType;

    protected:
        InterfaceType *m_ptr;
        template <class U>
        friend class ComPtr;

        void internalAddRef() const
        {
            if (m_ptr != nullptr)
            {
                m_ptr->AddRef();
            }
        }

        unsigned long internalRelease()
        {
            unsigned long ref = 0;
            T *temp = m_ptr;

            if (temp != nullptr)
            {
                m_ptr = nullptr;
                ref = temp->Release();
            }

            return ref;
        }

    public:
        ComPtr()
            : m_ptr(nullptr)
        {
        }

        ComPtr(decltype(__nullptr))
            : m_ptr(nullptr)
        {
        }

        template <class U>
        ComPtr(U *other)
            : m_ptr(other)
        {
            internalAddRef();
        }

        ComPtr(const ComPtr &other)
            : m_ptr(other.m_ptr)
        {
            internalAddRef();
        }

        // copy constructor that allows to instantiate class when U* is convertible to T*
        template <class U>
        ComPtr(const ComPtr<U> &other, typename TEnableIf<__is_convertible_to(U *, T *), void *>::Type * = 0)
            : m_ptr(other.m_ptr)
        {
            internalAddRef();
        }

        ComPtr(ComPtr &&other) noexcept
            : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<ComPtr *>(&reinterpret_cast<unsigned char &>(other)))
            {
                swap(other);
            }
        }

        // Move constructor that allows instantiation of a class when U* is convertible to T*
        template <class U>
        ComPtr(ComPtr<U> &&other, typename TEnableIf<__is_convertible_to(U *, T *), void *>::Type * = 0)
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ~ComPtr()
        {
            internalRelease();
        }

    public:
        ComPtr &operator=(decltype(__nullptr))
        {
            internalRelease();
            return *this;
        }

        ComPtr &operator=(T *other)
        {
            if (m_ptr != other)
            {
                ComPtr(other).swap(*this);
            }
            return *this;
        }

        template <typename U>
        ComPtr &operator=(U *other)
        {
            ComPtr(other).swap(*this);
            return *this;
        }

        ComPtr &operator=(const ComPtr &other)
        {
            if (m_ptr != other.m_ptr)
            {
                ComPtr(other).swap(*this);
            }
            return *this;
        }

        template <class U>
        ComPtr &operator=(const ComPtr<U> &other)
        {
            ComPtr(other).swap(*this);
            return *this;
        }

        ComPtr &operator=(ComPtr &&other) noexcept
        {
            ComPtr(static_cast<ComPtr &&>(other)).swap(*this);
            return *this;
        }

        template <class U>
        ComPtr &operator=(ComPtr<U> &&other)
        {
            ComPtr(static_cast<ComPtr<U> &&>(other)).swap(*this);
            return *this;
        }

    public:
        void swap(ComPtr &&r)
        {
            T *tmp = m_ptr;
            m_ptr = r.m_ptr;
            r.m_ptr = tmp;
        }

        void swap(ComPtr &r)
        {
            T *tmp = m_ptr;
            m_ptr = r.m_ptr;
            r.m_ptr = tmp;
        }

        operator bool() const
        {
            return get() != nullptr;
        }

        operator T *() const
        {
            return m_ptr;
        }

        T *get() const
        {
            return m_ptr;
        }

        typename InterfaceType *operator->() const
        {
            return m_ptr;
        }

        T **operator&()
        {
            return &m_ptr;
        }

        const T **operator&() const
        {
            return &m_ptr;
        }

        T *const *getAddressOf() const
        {
            return &m_ptr;
        }

        T **getAddressOf()
        {
            return &m_ptr;
        }

        T **releaseAndGetAddressOf()
        {
            internalRelease();
            return &m_ptr;
        }

        T *detach()
        {
            T *ptr = m_ptr;
            m_ptr = nullptr;
            return ptr;
        }

        void attach(InterfaceType *other)
        {
            if (m_ptr != nullptr)
            {
                auto ref = m_ptr->Release();
            }

            m_ptr = other;
        }

        unsigned long reset()
        {
            return internalRelease();
        }

        // query for U interface
        template <typename U>
        HRESULT as(ComPtr<U> *p) const
        {
            return m_ptr->QueryInterface(__uuidof(U), reinterpret_cast<void **>(p->releaseAndGetAddressOf()));
        }
    };

    // Comparison operators - don't compare COM object identity
    template <class T, class U>
    bool operator==(const ComPtr<T> &a, const ComPtr<U> &b)
    {
        static_assert(__is_base_of(T, U) || __is_base_of(U, T), "'T' and 'U' pointers must be comparable");
        return a.get() == b.get();
    }

    template <class T>
    bool operator==(const ComPtr<T> &a, decltype(__nullptr))
    {
        return a.get() == nullptr;
    }

    template <class T>
    bool operator==(decltype(__nullptr), const ComPtr<T> &a)
    {
        return a.get() == nullptr;
    }

    template <class T, class U>
    bool operator!=(const ComPtr<T> &a, const ComPtr<U> &b)
    {
        static_assert(__is_base_of(T, U) || __is_base_of(U, T), "'T' and 'U' pointers must be comparable");
        return a.get() != b.get();
    }

    template <class T>
    bool operator!=(const ComPtr<T> &a, decltype(__nullptr))
    {
        return a.get() != nullptr;
    }

    template <class T>
    bool operator!=(decltype(__nullptr), const ComPtr<T> &a)
    {
        return a.get() != nullptr;
    }

    template <class T, class U>
    bool operator<(const ComPtr<T> &a, const ComPtr<U> &b)
    {
        static_assert(__is_base_of(T, U) || __is_base_of(U, T), "'T' and 'U' pointers must be comparable");
        return a.get() < b.get();
    }

}

#endif
