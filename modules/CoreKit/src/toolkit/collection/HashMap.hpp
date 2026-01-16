/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References:
 *    http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */

#pragma once

#include "toolkit/memory/Memory.hpp"
#include <toolkit/memory/ContainerMemory.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/base/Hash.hpp>

namespace toolkit
{

    // Template for unordered dictionary with mapped key with value pairs
    template <typename KeyType, typename ValueType>
    class TkHashMap
    {
    public:
        // Describes single portion of space for the key and value pair in a hash map.
        struct Bucket
        {
            friend TkHashMap;
            KeyType Key;
            ValueType Value;

        private:
            BucketState m_state;

            TK_FORCE_INLINE void free()
            {
                if (m_state == BucketState::kOccupied)
                {
                    TkMemory::destructItem(&Key);
                    TkMemory::destructItem(&Value);
                }
                m_state = BucketState::kEmpty;
            }

            TK_FORCE_INLINE void destroy()
            {
                if (m_state == BucketState::kOccupied)
                {
                    TkMemory::destructItem(&Key);
                    TkMemory::destructItem(&Value);
                    m_state = BucketState::kDeleted;
                }
            }

            template <typename KeyComparableType>
            TK_FORCE_INLINE void occupy(const KeyComparableType &key)
            {
                TkMemory::constructItems(&Key, &key, 1);
                TkMemory::constructItem(&Value);
                m_state = BucketState::kOccupied;
            }

            template <typename KeyComparableType>
            TK_FORCE_INLINE void occupy(const KeyComparableType &key, const ValueType &value)
            {
                TkMemory::constructItems(&Key, &key, 1);
                TkMemory::constructItems(&Value, &value, 1);
                m_state = BucketState::kOccupied;
            }

            template <typename KeyComparableType>
            TK_FORCE_INLINE void occupy(const KeyComparableType &key, ValueType &&value)
            {
                TkMemory::constructItems(&Key, &key, 1);
                TkMemory::moveItems(&Value, &value, 1);
                m_state = BucketState::kOccupied;
            }

            TK_FORCE_INLINE bool isEmpty() const
            {
                return m_state == BucketState::kEmpty;
            }

            TK_FORCE_INLINE bool isDeleted() const
            {
                return m_state == BucketState::kDeleted;
            }

            TK_FORCE_INLINE bool isOccupied() const
            {
                return m_state == BucketState::kOccupied;
            }

            TK_FORCE_INLINE bool isNotOccupied() const
            {
                return m_state != BucketState::kOccupied;
            }
        };

    public:
        TkHashMap(IAllocator &alloc = TkMemory::getDefault())
            : m_alloc{alloc}
        {
        }
        explicit TkHashMap(const i32 capacity, IAllocator &alloc = TkMemory::getDefault())
            : m_alloc{alloc}
        {
            setCapacity(capacity);
        }
        TkHashMap(TkHashMap &&other) noexcept
            : m_alloc{other.m_alloc}
        {
            m_elementsCount = other.m_elementsCount;
            m_deletedCount = other.m_deletedCount;
            m_size = other.m_size;
            other.m_elementsCount = 0;
            other.m_deletedCount = 0;
            other.m_size = 0;
            moveToEmpty(m_alloc, m_data, other.m_data, m_size);
        }
        TkHashMap(const TkHashMap &other)
            : m_alloc{other.m_alloc}
        {
            clone(other);
        }

        TkHashMap &operator=(const TkHashMap &other)
        {
            if (this != &other)
            {
                if (&m_alloc != &other.m_alloc)
                {
                    clear();
                    TkMemory::free(m_alloc, m_data);
                    m_data = nullptr;
                    m_size = m_elementsCount = m_deletedCount = 0;
                }
                clone(other);
            }
            return *this;
        }
        TkHashMap &operator=(TkHashMap &&other) noexcept
        {
            if (this != &other)
            {
                clear();
                if (m_data)
                    TkMemory::free(m_alloc, m_data);

                if (&m_alloc == &other.m_alloc)
                {
                    TK_ASSERT(m_alloc.getTypeId() != AllocatorType::kStack && "Cannot steal from stack allocator");
                    stealFrom(other);
                }
                else
                {
                    clone(other);
                    other.clear();
                }
            }
            return *this;
        }

        ~TkHashMap()
        {
            clear();
            if (m_data)
                TkMemory::free(m_alloc, m_data);
        }

    public:
        TK_FORCE_INLINE i32 count() const
        {
            return m_elementsCount;
        }

        TK_FORCE_INLINE i32 capacity() const
        {
            return m_size;
        }

        TK_FORCE_INLINE bool isEmpty() const
        {
            return m_elementsCount == 0;
        }

        TK_FORCE_INLINE bool hasItems() const
        {
            return m_elementsCount != 0;
        }

    public:
        struct Iterator
        {
            friend TkHashMap;

        private:
            TkHashMap *m_collection;
            i32 m_index;

        public:
            Iterator(TkHashMap *collection, const i32 index)
                : m_collection(collection), m_index(index)
            {
            }

            Iterator(TkHashMap const *collection, const i32 index)
                : m_collection(const_cast<TkHashMap *>(collection)), m_index(index)
            {
            }

            Iterator()
                : m_collection(nullptr), m_index(-1)
            {
            }

            Iterator(const Iterator &i)
                : m_collection(i.m_collection), m_index(i.m_index)
            {
            }

            Iterator(Iterator &&i) noexcept
                : m_collection(i.m_collection), m_index(i.m_index)
            {
            }

        public:
            TK_FORCE_INLINE i32 index() const
            {
                return m_index;
            }

            TK_FORCE_INLINE bool isEnd() const
            {
                return m_index == m_collection->m_size;
            }

            TK_FORCE_INLINE bool isNotEnd() const
            {
                return m_index != m_collection->m_size;
            }

            TK_FORCE_INLINE Bucket &operator*() const
            {
                return m_collection->m_data[m_index];
            }

            TK_FORCE_INLINE Bucket *operator->() const
            {
                return &m_collection->m_data[m_index];
            }

            TK_FORCE_INLINE explicit operator bool() const
            {
                return m_index >= 0 && m_index < m_collection->m_size;
            }

            TK_FORCE_INLINE bool operator!() const
            {
                return !(bool)*this;
            }

            TK_FORCE_INLINE bool operator==(const Iterator &v) const
            {
                return m_index == v.m_index && m_collection == v.m_collection;
            }

            TK_FORCE_INLINE bool operator!=(const Iterator &v) const
            {
                return m_index != v.m_index || m_collection != v.m_collection;
            }

            Iterator &operator=(const Iterator &v)
            {
                m_collection = v.m_collection;
                m_index = v.m_index;
                return *this;
            }

            Iterator &operator=(Iterator &&v) noexcept
            {
                m_collection = v.m_collection;
                m_index = v.m_index;
                return *this;
            }

            Iterator &operator++()
            {
                const i32 capacity = m_collection->m_size;
                if (m_index != capacity)
                {
                    const Bucket *data = m_collection->m_data;
                    do
                    {
                        ++m_index;
                    } while (m_index != capacity && data[m_index].isNotOccupied());
                }
                return *this;
            }

            Iterator operator++(int) const
            {
                Iterator i = *this;
                ++i;
                return i;
            }

            Iterator &operator--()
            {
                if (m_index > 0)
                {
                    const Bucket *data = m_collection->m_data;
                    do
                    {
                        --m_index;
                    } while (m_index > 0 && data[m_index].isNotOccupied());
                }
                return *this;
            }

            Iterator operator--(int) const
            {
                Iterator i = *this;
                --i;
                return i;
            }
        };

    public:
        // Gets element by the key (will add default ValueType element if key not found).
        template <typename KeyComparableType>
        ValueType &at(const KeyComparableType &key)
        {
            // Check if need to rehash elements (prevent many deleted elements that use too much of capacity)
            if (m_deletedCount > m_size / DICTIONARY_DEFAULT_SLACK_SCALE)
                compact();

            // Ensure to have enough memory for the next item (in case of new element insertion)
            ensureCapacity((m_elementsCount + 1) * DICTIONARY_DEFAULT_SLACK_SCALE + m_deletedCount);

            // Find location of the item or place to insert it
            FindPositionResult pos;
            findPosition(key, pos);

            // Check if that key has been already added
            if (pos.ObjectIndex != -1)
                return m_data[pos.ObjectIndex].Value;

            // Insert
            TK_ASSERT(pos.FreeSlotIndex != -1);
            ++m_elementsCount;
            Bucket &bucket = m_data[pos.FreeSlotIndex];
            bucket.occupy(key);
            return bucket.Value;
        }

        // Gets the element by the key.
        template <typename KeyComparableType>
        const ValueType &at(const KeyComparableType &key) const
        {
            FindPositionResult pos;
            findPosition(key, pos);
            TK_ASSERT(pos.ObjectIndex != -1);
            return m_data[pos.ObjectIndex].Value;
        }

        // Gets or sets the element by the key.
        template <typename KeyComparableType>
        TK_FORCE_INLINE ValueType &operator[](const KeyComparableType &key)
        {
            return at(key);
        }

        // Gets or sets the element by the key.
        template <typename KeyComparableType>
        TK_FORCE_INLINE const ValueType &operator[](const KeyComparableType &key) const
        {
            return at(key);
        }

        // Tries to get element with given key.
        template <typename KeyComparableType>
        bool tryGet(const KeyComparableType &key, ValueType &result) const
        {
            if (isEmpty())
                return false;
            FindPositionResult pos;
            findPosition(key, pos);
            if (pos.ObjectIndex == -1)
                return false;
            result = m_data[pos.ObjectIndex].Value;
            return true;
        }

    public:
        // Clears the collection but without changing its capacity (all inserted elements: keys and values will be removed).
        void clear()
        {
            if (!m_data)
                return;
            if (m_elementsCount + m_deletedCount != 0)
            {
                Bucket *data = m_data;
                for (i32 i = 0; i < m_size; i++)
                    data[i].free();
                m_elementsCount = m_deletedCount = 0;
            }
        }

        void clearDelete() requires(TIsPointer<ValueType>::Value)
        {
            for (Iterator i = Begin(); i.isNotEnd(); ++i)
            {
                if (i->Value)
                    TkMemory::free(m_alloc, i->Value);
            }
            clear();
        }

        void clearDelete() requires(!TIsPointer<ValueType>::Value)
        {
            clear();
        }

        // Changes the capacity of the collection.
        void setCapacity(i32 capacity, const bool preserveContents = true)
        {
            if (capacity == this->capacity())
                return;
            TK_ASSERT(capacity >= 0);

            Bucket* oldAllocation = nullptr;
            auto needFree = moveToEmpty(m_alloc, oldAllocation, m_data, m_size);
            const i32 oldSize = m_size;
            const i32 oldElementsCount = m_elementsCount;
            m_deletedCount = m_elementsCount = 0;
            if (capacity != 0 && (capacity & (capacity - 1)) != 0)
            {
                capacity--;
                capacity |= capacity >> 1;
                capacity |= capacity >> 2;
                capacity |= capacity >> 4;
                capacity |= capacity >> 8;
                capacity |= capacity >> 16;
                capacity++;
            }
            if (capacity)
            {
                m_data = (Bucket*)TkMemory::allocate(m_alloc, capacity * sizeof(Bucket));
                Bucket *data = m_data;
                for (i32 i = 0; i < capacity; i++)
                    data[i].m_state = BucketState::kEmpty;
            }
            m_size = capacity;
            Bucket *oldData = oldAllocation;
            if (oldElementsCount != 0 && capacity != 0 && preserveContents)
            {
                FindPositionResult pos;
                for (i32 i = 0; i < oldSize; i++)
                {
                    Bucket &oldBucket = oldData[i];
                    if (oldBucket.isOccupied())
                    {
                        findPosition(oldBucket.Key, pos);
                        TK_ASSERT(pos.FreeSlotIndex != -1);
                        Bucket *bucket = &m_data[pos.FreeSlotIndex];
                        TkMemory::moveItems(&bucket->Key, &oldBucket.Key, 1);
                        TkMemory::moveItems(&bucket->Value, &oldBucket.Value, 1);
                        bucket->m_state = BucketState::kOccupied;
                        ++m_elementsCount;
                    }
                }
            }
            if (oldElementsCount != 0)
            {
                for (i32 i = 0; i < oldSize; i++)
                    oldData[i].free();
            }
        }

        // Ensures that collection has given capacity.
        void ensureCapacity(i32 minCapacity, const bool preserveContents = true)
        {
            if (m_size >= minCapacity)
                return;
            i32 capacity = calCapacityGrow(m_alloc, m_size, minCapacity);
            if (capacity < DICTIONARY_DEFAULT_CAPACITY)
                capacity = DICTIONARY_DEFAULT_CAPACITY;
            setCapacity(capacity, preserveContents);
        }

        // Swaps the contents of collection with the other object without copy operation. Performs fast internal data exchange.
        void swap(TkHashMap &other)
        {
            if (m_alloc.getTypeId() != AllocatorType::kStack)
            {
                toolkit::swap(m_elementsCount, other.m_elementsCount);
                toolkit::swap(m_deletedCount, other.m_deletedCount);
                toolkit::swap(m_size, other.m_size);
                toolkit::swap(m_data, other.m_data);
            }
            else
            {
                toolkit::swap(other, *this);
            }
        }

    public:
        // Add pair element to the collection.
        template <typename KeyComparableType>
        TK_FORCE_INLINE Bucket *add(const KeyComparableType &key, const ValueType &value)
        {
            Bucket *bucket = onAdd(key);
            bucket->occupy(key, value);
            return bucket;
        }

        // Add pair element to the collection.
        template <typename KeyComparableType>
        TK_FORCE_INLINE Bucket *add(const KeyComparableType &key, ValueType &&value)
        {
            Bucket *bucket = onAdd(key);
            bucket->occupy(key, moveTemp(value));
            return bucket;
        }

        // Add pair element to the collection.
        void add(const Iterator &i)
        {
            TK_ASSERT(i.m_collection != this && i);
            const Bucket &bucket = *i;
            add(bucket.Key, bucket.Value);
        }

        // Removes element with a specified key.
        template <typename KeyComparableType>
        bool remove(const KeyComparableType &key)
        {
            if (isEmpty())
                return false;
            FindPositionResult pos;
            findPosition(key, pos);
            if (pos.ObjectIndex != -1)
            {
                m_data[pos.ObjectIndex].destroy();
                --m_elementsCount;
                ++m_deletedCount;
                return true;
            }
            return false;
        }

        // Removes element at specified iterator.
        bool remove(const Iterator &i)
        {
            TK_ASSERT(i.m_collection == this);
            if (i)
            {
                TK_ASSERT(m_data[i.m_index].isOccupied());
                m_data[i.m_index].destroy();
                --m_elementsCount;
                ++m_deletedCount;
                return true;
            }
            return false;
        }

        // Removes elements with a specified value
        i32 removeValue(const ValueType &value)
        {
            i32 result = 0;
            for (Iterator i = Begin(); i.isNotEnd(); ++i)
            {
                if (i->Value == value)
                {
                    remove(i);
                    ++result;
                }
            }
            return result;
        }

    public:
        // Finds the element with given key in the collection.
        template <typename KeyComparableType>
        Iterator find(const KeyComparableType &key) const
        {
            if (isEmpty())
                return End();
            FindPositionResult pos;
            findPosition(key, pos);
            return pos.ObjectIndex != -1 ? Iterator(this, pos.ObjectIndex) : End();
        }

        // Checks if given key is in a collection.
        template <typename KeyComparableType>
        bool containsKey(const KeyComparableType &key) const
        {
            if (isEmpty())
                return false;
            FindPositionResult pos;
            findPosition(key, pos);
            return pos.ObjectIndex != -1;
        }

        // Checks if given value is in a collection.
        bool containsValue(const ValueType &value) const
        {
            if (hasItems())
            {
                const Bucket *data = m_data;
                for (i32 i = 0; i < m_size; ++i)
                {
                    if (data[i].isOccupied() && data[i].Value == value)
                        return true;
                }
            }
            return false;
        }

        // Searches for the specified object and returns the zero-based index of the first occurrence within the entire dictionary.
        bool keyOf(const ValueType &value, KeyType *key) const
        {
            if (hasItems())
            {
                const Bucket *data = m_data;
                for (i32 i = 0; i < m_size; ++i)
                {
                    if (data[i].isOccupied() && data[i].Value == value)
                    {
                        if (key)
                            *key = data[i].Key;
                        return true;
                    }
                }
            }
            return false;
        }

    public:
        // Clones other collection into this.
        void clone(const TkHashMap &other)
        {
            // TODO: if both key and value are POD types then use raw memory copy for buckets
            clear();
            ensureCapacity(other.capacity(), false);
            for (Iterator i = other.Begin(); i != other.End(); ++i)
                add(i);
        }

        // Gets the keys collection to the output array (will contain unique items).
        void getKeys(TkArray<KeyType> &result) const
        {
            for (Iterator i = Begin(); i.isNotEnd(); ++i)
                result.add(i->Key);
        }

        // Gets the values collection to the output array (may contain duplicates).
        void getValues(TkArray<ValueType> &result) const
        {
            for (Iterator i = Begin(); i.isNotEnd(); ++i)
                result.add(i->Value);
        }

    public:
        Iterator Begin() const
        {
            Iterator i(this, -1);
            ++i;
            return i;
        }

        Iterator End() const
        {
            return Iterator(this, m_size);
        }

        Iterator begin()
        {
            Iterator i(this, -1);
            ++i;
            return i;
        }

        TK_FORCE_INLINE Iterator end()
        {
            return Iterator(this, m_size);
        }

        Iterator begin() const
        {
            Iterator i(this, -1);
            ++i;
            return i;
        }

        TK_FORCE_INLINE Iterator end() const
        {
            return Iterator(this, m_size);
        }

    private:
        // The result container of the dictionary item lookup searching.
        struct FindPositionResult
        {
            i32 ObjectIndex;
            i32 FreeSlotIndex;
        };

        // Returns a pair of positions: 1st where the object is, 2nd where
        // it would go if you wanted to insert it. 1st is -1
        // if object is not found; 2nd is -1 if it is.
        template <typename KeyComparableType>
        void findPosition(const KeyComparableType &key, FindPositionResult &result) const
        {
            TK_ASSERT(m_size);
            const i32 tableSizeMinusOne = m_size - 1;
            i32 bucketIndex = HashFunc<KeyComparableType>::get(key) & tableSizeMinusOne;
            i32 insertPos = -1;
            i32 checksCount = 0;
            const Bucket *data = m_data;
            result.FreeSlotIndex = -1;
            while (checksCount < m_size)
            {
                // Empty bucket
                const Bucket &bucket = data[bucketIndex];
                if (bucket.isEmpty())
                {
                    // Found place to insert
                    result.ObjectIndex = -1;
                    result.FreeSlotIndex = insertPos == -1 ? bucketIndex : insertPos;
                    return;
                }
                // Deleted bucket
                if (bucket.isDeleted())
                {
                    // Keep searching but mark to insert
                    if (insertPos == -1)
                        insertPos = bucketIndex;
                }
                // Occupied bucket by target key
                else if (bucket.Key == key)
                {
                    // Found key
                    result.ObjectIndex = bucketIndex;
                    return;
                }
                ++checksCount;
                bucketIndex = (bucketIndex + DICTIONARY_PROB_FUNC(m_size, checksCount)) & tableSizeMinusOne;
            }
            result.ObjectIndex = -1;
            result.FreeSlotIndex = insertPos;
        }

        template <typename KeyComparableType>
        Bucket *onAdd(const KeyComparableType &key)
        {
            // Check if need to rehash elements (prevent many deleted elements that use too much of capacity)
            if (m_deletedCount > m_size / DICTIONARY_DEFAULT_SLACK_SCALE)
                compact();

            // Ensure to have enough memory for the next item (in case of new element insertion)
            ensureCapacity((m_elementsCount + 1) * DICTIONARY_DEFAULT_SLACK_SCALE + m_deletedCount);

            // Find location of the item or place to insert it
            FindPositionResult pos;
            findPosition(key, pos);

            // Ensure key is unknown
            TK_ASSERT(pos.ObjectIndex == -1 && "That key has been already added to the dictionary.");

            // Insert
            TK_ASSERT(pos.FreeSlotIndex != -1);
            m_elementsCount++;
            return &m_data[pos.FreeSlotIndex];
        }

    public:
        void compact()
        {
            if (m_elementsCount == 0)
            {
                // Fast path if it's empty
                Bucket *data = m_data;
                for (i32 i = 0; i < m_size; i++)
                    data[i].m_state = BucketState::kEmpty;
            }
            else
            {
                // Rebuild entire table completely
                // AllocationData oldAllocation;
                Bucket *oldAllocation;
                moveToEmpty(m_alloc, oldAllocation, m_data, m_size);
                m_data = (Bucket*)TkMemory::allocate(m_alloc, m_size * sizeof(Bucket));
                Bucket *data = m_data;
                for (i32 i = 0; i < m_size; i++)
                    data[i].m_state = BucketState::kEmpty;
                Bucket *oldData = oldAllocation;
                FindPositionResult pos;
                for (i32 i = 0; i < m_size; i++)
                {
                    Bucket &oldBucket = oldData[i];
                    if (oldBucket.isOccupied())
                    {
                        findPosition(oldBucket.Key, pos);
                        TK_ASSERT(pos.FreeSlotIndex != -1);
                        Bucket *bucket = &m_data[pos.FreeSlotIndex];
                        TkMemory::moveItems(&bucket->Key, &oldBucket.Key, 1);
                        TkMemory::moveItems(&bucket->Value, &oldBucket.Value, 1);
                        bucket->m_state = BucketState::kOccupied;
                    }
                }
                for (i32 i = 0; i < m_size; i++)
                    oldData[i].free();

                if (oldAllocation)
                    TkMemory::free(m_alloc, oldAllocation); // QUEST Check if this true
            }
            m_deletedCount = 0;
        }

    private:
        void stealFrom(TkHashMap &other)
        {
            m_data = other.m_data;
            m_size = other.m_size;
            m_elementsCount = other.m_elementsCount;
            m_deletedCount = other.m_deletedCount;

            other.m_data = nullptr;
            other.m_size = 0;
            other.m_elementsCount = 0;
            other.m_deletedCount = 0;
        }

        TK_FORCE_INLINE static bool moveToEmpty(IAllocator &alloc, Bucket *&to, Bucket *&from, const i32 fromSize)
        {
            if (alloc.getTypeId() != AllocatorType::kStack)
            {
                toolkit::swap(to, from);
                return true;
            }
            else
            {
                to = (Bucket*)TkMemory::allocate(alloc, fromSize * sizeof(Bucket));
                Bucket *toData = to;
                Bucket *fromData = from;
                for (i32 i = 0; i < fromSize; i++)
                {
                    Bucket &fromBucket = fromData[i];
                    if (fromBucket.isOccupied())
                    {
                        Bucket &toBucket = toData[i];
                        TkMemory::moveItems(&toBucket.Key, &fromBucket.Key, 1);
                        TkMemory::moveItems(&toBucket.Value, &fromBucket.Value, 1);
                        toBucket.m_state = BucketState::kOccupied;
                        TkMemory::destructItem(&fromBucket.Key);
                        TkMemory::destructItem(&fromBucket.Value);
                        fromBucket.m_state = BucketState::kEmpty;
                    }
                }
                TkMemory::free(alloc, from);
                return false;
            }
        }

    private:
        i32 m_elementsCount = 0;
        i32 m_deletedCount = 0;
        i32 m_size = 0;
        IAllocator &m_alloc;
        Bucket* m_data = nullptr;

    };

}