/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 * Test target: DataCollectionTest.cpp
 */

#pragma once

#include "toolkit/memory/Memory.hpp"
#include <toolkit/types/Span.hpp>

namespace toolkit {

    /// @brief Small owned-or-linked data view that can either reference or allocate storage.
    template<typename T>
    class TkDataCollection : public TkSpan<T>
    {
    public:
        typedef TkSpan<T> Base;

    public:
        /// @brief Creates an empty collection.
        TkDataCollection(IAllocator& alloc = TkMemory::getDefault())
            : Base {}
            , m_alloc {alloc} 
        { }
        TkDataCollection(const T* data, i32 length, IAllocator& alloc = TkMemory::getDefault())
            : Base {data, length}
            , m_alloc {alloc} 
        {}
        TkDataCollection(const TkArray<T>& data, IAllocator& alloc = TkMemory::getDefault())
            : Base {data.get(), data.count()}
            , m_alloc {alloc} 
        {}
        TkDataCollection(const TkSpan<T>& data, IAllocator& alloc = TkMemory::getDefault())
            : Base {data.get(), data.length()}
            , m_alloc {alloc} 
        {}
        TkDataCollection(const TkDataCollection& other)
            : TkDataCollection {other.m_alloc}
        {
            if (other.isAllocated())
                copy(other);
            else
                link(other);
        }


        TkDataCollection(TkDataCollection&& other) noexcept
            : m_alloc(other.m_alloc)
        {
            Base::m_data = other.m_data;
            Base::m_length = other.m_length;
            m_isAllocated = other.m_isAllocated;
            other.m_data = nullptr;
            other.m_length = 0;
            other.m_isAllocated = false;
        }


        // The move assignment operator that deletes the current container of data and the moves items from the other container.
        TkDataCollection& operator=(TkDataCollection&& other) noexcept
        {
            if (this != &other)
            {
                TK_ASSERT(&m_alloc == &other.m_alloc);
                release();
                Base::m_data = other.m_data;
                Base::m_length = other.m_length;
                m_isAllocated = other.m_isAllocated;
                other.m_data = nullptr;
                other.m_length = 0;
                other.m_isAllocated = false;
            }
            return *this;
        }

        // Assignment operator
        TkDataCollection& operator=(const TkDataCollection& other)
        {
            if (this != &other)
            {
                TK_ASSERT(&m_alloc == &other.m_alloc);
                if (other.isAllocated())
                    copy(other);
                else
                    link(other);
            }
            return *this;
        }


        // Releases any allocated memory.
        ~TkDataCollection()
        {
            if (m_isAllocated)
                TkMemory::free(m_alloc, Base::m_data);
        }

    public:
        /// @brief Returns true when this object owns its current storage.
        TK_FORCE_INLINE bool isAllocated() const
        {
            return m_isAllocated;
        }

        /// @brief Links external storage without taking ownership.
        void link(const TkArray<T>& data)
        {
            link(data.get(), data.count());
        }

        // Links the external data.
        TK_FORCE_INLINE void link(const TkSpan<T>& data)
        {
            link(data.get(), data.length());
        }

        // Links the external data.
        TK_FORCE_INLINE void link(const TkDataCollection& data)
        {
            link(data.get(), data.length());
        }

        // Links external data.
        template<typename U>
        TK_FORCE_INLINE void link(const U* data)
        {
            link(static_cast<T*>((void*)data), sizeof(U));
        }

        // Links the external data.
        void link(const T* data, i32 length)
        {
            release();
            m_isAllocated = false;
            Base::m_length = length;
            Base::m_data = (T*)data;
        }

        /// @brief Allocates owned storage for `length` items.
        void allocate(const i32 length)
        {
            if (m_isAllocated && Base::m_length == length)
                return;
            release();
            if (length > 0)
            {
                m_isAllocated = true;
                Base::m_length = length;
                Base::m_data = (T*)TkMemory::allocate(m_alloc, length * sizeof(T));
            }
        }

        /// @brief Copies `data` into newly owned storage.
        TK_FORCE_INLINE void copy(const TkArray<T>& data)
        {
            if (data.hasItems())
                copy(data.get(), data.count());
            else
                release();
        }

        // Copies the data to a new allocated chunk.
        void copy(const TkDataCollection& data)
        {
            if (data.length() != 0)
                copy(data.get(), data.length());
            else
                release();
        }

        // Copies the data to a new allocated chunk.
        void copy(const TkSpan<T>& data)
        {
            if (data.length() != 0)
                copy(data.get(), data.length());
            else
                release();
        }

        // Copies the data to a new allocated chunk.
        template<typename U>
        TK_FORCE_INLINE void copy(const U* data)
        {
            copy(static_cast<T*>((void*)data), sizeof(U));
        }

        // Copies the data to a new allocated chunk.
        void copy(const T* data, i32 length)
        {
            TK_ASSERT(data && length > 0);
            allocate(length);
            TK_ASSERT(Base::m_data);
            memcpy(Base::m_data, data, length * sizeof(T));
        }

        /// @brief Exchanges contents with another collection using the same allocator.
        void swap(TkDataCollection& data)
        {
            TK_ASSERT(&m_alloc == &data.m_alloc);
            toolkit::swap(m_isAllocated, data.m_isAllocated);
            toolkit::swap(Base::m_length, data.m_length);
            toolkit::swap(Base::m_data, data.m_data);
        }

        /// @brief Releases owned storage and resets the collection to empty.
        void release()
        {
            if (m_isAllocated)
                TkMemory::free(m_alloc, Base::m_data);
            m_isAllocated = false;
            Base::m_length = 0;
            Base::m_data = nullptr;
        }

        // Unlinks the buffer and clears the length to zero. Use with caution because it's not safe.
        void unlink()
        {
            m_isAllocated = false;
            Base::m_length = 0;
            Base::m_data = nullptr;
        }

        // Sets the length of the internal allocated buffer. Can be used to `trim` length allocated data without performing reallocation. Use with caution because it's not safe.
        void setLength(i32 length)
        {
            TK_ASSERT(length >= 0);
            Base::m_length = length;
        }

        /// @brief Appends a copy of `length` items to the owned buffer.
        void append(const T* data, i32 length)
        {
            if (length <= 0)
                return;
            auto prev = Base::m_data;
            const auto prevLength = Base::m_length;
            if (prevLength == 0 || prev == nullptr)
            {
                copy(data, length);
                return;
            }

            Base::m_length = prevLength + length;
            Base::m_data = (T*)TkMemory::allocate(m_alloc, Base::m_length * sizeof(T));

            TkMemory::memoryCopy(Base::m_data, prev, prevLength * sizeof(T));
            TkMemory::memoryCopy(Base::m_data + prevLength, data, length * sizeof(T));

            if (m_isAllocated)
                TkMemory::free(m_alloc, prev);
            m_isAllocated = true;
        }

    public:
        template<typename ReadStream>
        void read(ReadStream* stream, i32 length)
        {
            TK_ASSERT(stream != nullptr);
            allocate(length);
            if (length > 0)
            {
                stream->readBytes((void*)Base::m_data, length * sizeof(T));
            }
        }

        template<typename WriteStream>
        void write(WriteStream* stream) const
        {
            TK_ASSERT(stream != nullptr);
            if (Base::m_length > 0)
            {
                stream->writeBytes((void*)Base::m_data, Base::m_length * sizeof(T));
            }
        }
        
    private:
        bool m_isAllocated = false;
        IAllocator& m_alloc;
    };

    typedef TkDataCollection<byte> ByteCollection;
}
