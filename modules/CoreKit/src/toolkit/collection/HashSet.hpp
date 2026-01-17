/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 * Reference:
 *     http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */


#pragma once

#include <toolkit/base/Assert.hpp>
#include <toolkit/base/Hash.hpp>
#include <toolkit/memory/ContainerMemory.hpp>
#include <toolkit/memory/Memory.hpp>

namespace toolkit {

	// Template for unordered set of values (without duplicates with O(1) lookup access)
	template<typename T>
	class TkHashSet
	{
		friend TkHashSet;
	public:
		// Describes single portion of space for the item in a hash map.
		struct Bucket
		{
			friend TkHashSet;
			T item;

		private:
			BucketState m_state;

			TK_FORCE_INLINE void free()
			{
				if (m_state == BucketState::kOccupied)
					TkMemory::destructItem(&item);
				m_state = BucketState::kEmpty;
			}

			TK_FORCE_INLINE void destroy()
			{
				m_state = BucketState::kDeleted;
				TkMemory::destructItem(&item);
			}

			template<typename ItemType>
			TK_FORCE_INLINE void occupy(const ItemType& item)
			{
				TkMemory::constructItems(&this->item, &item, 1);
				m_state = BucketState::kOccupied;
			}

			template<typename ItemType>
			TK_FORCE_INLINE void occupy(ItemType&& item)
			{
				TkMemory::moveItems(&this->item, &item, 1);
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
		TkHashSet(IAllocator& alloc = TkMemory::getDefault()) 
		    : m_alloc {alloc}
		{ }
		explicit TkHashSet(const i32 capacity, IAllocator& alloc = TkMemory::getDefault())
		    : m_alloc {alloc}
		{
			setCapacity(capacity);
		}
		TkHashSet(TkHashSet&& other) noexcept
		    : m_alloc {other.m_alloc}
		{
			m_elementsCount = other.m_elementsCount;
			m_deletedCount = other.m_deletedCount;
			m_size = other.m_size;
			other.m_elementsCount = 0;
			other.m_deletedCount = 0;
			other.m_size = 0;
			moveToEmpty(m_data, other.m_data, m_size);
		}

		TkHashSet(const TkHashSet& other)
            : m_alloc{other.m_alloc}
		{
			clone(other);
		}

		// Clones the data from the other collection.
		TkHashSet& operator=(const TkHashSet& other)
		{
			if (this != &other)
				clone(other);
			return *this;
		}

		// Moves the data from the other collection.
		TkHashSet& operator=(TkHashSet&& other) noexcept
		{
			if (this != &other)
			{
				clear();
				TkMemory::free(m_alloc, m_data);
				m_elementsCount = other.m_elementsCount;
				m_deletedCount = other.m_deletedCount;
				m_size = other.m_size;
				other.m_elementsCount = 0;
				other.m_deletedCount = 0;
				other.m_size = 0;
				moveToEmpty(m_data, other.m_data, m_size);
			}
			return *this;
		}

		~TkHashSet()
		{
			clear();
			if (m_data)
			    TkMemory::free(m_alloc, m_data);
		}

	public:
		// Gets the amount of the elements in the collection.
		TK_FORCE_INLINE i32 count() const
		{
			return m_elementsCount;
		}

		// Gets the amount of the elements that can be contained by the collection.
		TK_FORCE_INLINE i32 capacity() const
		{
			return m_size;
		}

		// Returns true if collection is empty.
		TK_FORCE_INLINE bool isEmpty() const
		{
			return m_elementsCount == 0;
		}

		// Returns true if collection has one or more elements.
		TK_FORCE_INLINE bool hasItems() const
		{
			return m_elementsCount != 0;
		}

	public:
		// The hash set collection iterator.
		struct Iterator
		{
			friend TkHashSet;
		private:
			TkHashSet* m_collection;
			i32 m_index;

		public:
			Iterator(TkHashSet* collection, const i32 index)
				: m_collection(collection)
				, m_index(index)
			{
			}

			Iterator(const TkHashSet* collection, const i32 index)
				: m_collection(const_cast<TkHashSet*>(collection))
				, m_index(index)
			{
			}

			Iterator()
				: m_collection(nullptr)
				, m_index(-1)
			{
			}

			Iterator(const Iterator& i)
				: m_collection(i.m_collection)
				, m_index(i.m_index)
			{
			}

			Iterator(Iterator&& i) noexcept
				: m_collection(i.m_collection)
				, m_index(i.m_index)
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

			TK_FORCE_INLINE Bucket& operator*() const
			{
				return m_collection->m_data[m_index];
			}

			TK_FORCE_INLINE Bucket* operator->() const
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

			TK_FORCE_INLINE bool operator==(const Iterator& v) const
			{
				return m_index == v.m_index && m_collection == v.m_collection;
			}

			TK_FORCE_INLINE bool operator!=(const Iterator& v) const
			{
				return m_index != v.m_index || m_collection != v.m_collection;
			}

			Iterator& operator=(const Iterator& v)
			{
				m_collection = v.m_collection;
				m_index = v.m_index;
				return *this;
			}

			Iterator& operator=(Iterator&& v) noexcept
			{
				m_collection = v.m_collection;
				m_index = v.m_index;
				return *this;
			}

			Iterator& operator++()
			{
				const i32 capacity = m_collection->m_size;
				if (m_index != capacity)
				{
					const Bucket* data = m_collection->m_data;
					do
					{
						++m_index;
					} while (m_index != capacity && data[m_index].isNotOccupied());
				}
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator i = *this;
				++i;
				return i;
			}

			Iterator& operator--()
			{
				if (m_index > 0)
				{
					const Bucket* data = m_collection->m_data;
					do
					{
						--m_index;
					} while (m_index > 0 && data[m_index].isNotOccupied());
				}
				return *this;
			}

			Iterator operator--(int)
			{
				Iterator i = *this;
				--i;
				return i;
			}
		};

	public:
		// Removes all elements from the collection.
		void clear()
		{
			if (m_elementsCount + m_deletedCount != 0)
			{
				Bucket* data = m_data;
				for (i32 i = 0; i < m_size; i++)
					data[i].free();
				m_elementsCount = m_deletedCount = 0;
			}
		}


		void clearDelete() 
#if TK_COMPILER_MSVC
		    requires(TIsPointer<T>::Value)
#endif
		{
			for (Iterator i = Begin(); i.isNotEnd(); ++i)
			{
				if (i->Item)
					TkMemory::free(m_alloc, i->Item);
			}
			clear();
		}

#if TK_COMPILER_MSVC
		void clearDelete() requires(!TIsPointer<T>::Value) {
			clear();
		}
#endif

		// Changes capacity of the collection
		void setCapacity(i32 capacity, const bool preserveContents = true)
		{
			if (capacity == this->capacity())
				return;
			TK_ASSERT(capacity >= 0);
			Bucket* oldAllocation = nullptr;
			moveToEmpty(m_alloc, oldAllocation, m_data, m_size);
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
				Bucket* data = m_data;
				for (i32 i = 0; i < capacity; i++)
					data[i].m_state = BucketState::kEmpty;
			}
			m_size = capacity;
			Bucket* oldData = oldAllocation;
			if (oldElementsCount != 0 && capacity != 0 && preserveContents)
			{
				FindPositionResult pos;
				for (i32 i = 0; i < oldSize; i++)
				{
					Bucket& oldBucket = oldData[i];
					if (oldBucket.isOccupied())
					{
						findPosition(oldBucket.item, pos);
						TK_ASSERT(pos.freeSlotIdx != -1);
						Bucket* bucket = &m_data[pos.freeSlotIdx];
						TkMemory::moveItems(&bucket->item, &oldBucket.item, 1);
						bucket->m_state = BucketState::kOccupied;
						m_elementsCount++;
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
		void ensureCapacity(const i32 minCapacity, const bool preserveContents = true)
		{
			if (m_size >= minCapacity)
				return;
			i32 capacity = calCapacityGrow(m_alloc, m_size, minCapacity);
			if (capacity < DICTIONARY_DEFAULT_CAPACITY)
				capacity = DICTIONARY_DEFAULT_CAPACITY;
			setCapacity(capacity, preserveContents);
		}

		// Swaps the contents of collection with the other object without copy operation. Performs fast internal data exchange.
		void swap(TkHashSet& other)
		{
			if (m_alloc.getTypeId() != AllocatorType::kStack)
			{
				swap(m_elementsCount, other.m_elementsCount);
				swap(m_deletedCount, other.m_deletedCount);
				swap(m_size, other.m_size);
				toolkit::swap(m_data, other.m_data);
			}
			else
			{
				TkHashSet tmp = moveTemp(other);
				other = *this;
				*this = moveTemp(tmp);
			}
		}

	public:
		// Add element to the collection.
		template<typename ItemType>
		bool add(const ItemType& item)
		{
			Bucket* bucket = anAdd(item);
			if (bucket)
				bucket->occupy(item);
			return bucket != nullptr;
		}

		// Add element to the collection.
		bool add(T&& item)
		{
			Bucket* bucket = anAdd(item);
			if (bucket)
				bucket->occupy(moveTemp(item));
			return bucket != nullptr;
		}

		// Add element at iterator to the collection
		void add(const Iterator& i)
		{
			TK_ASSERT(i.m_collection != this && i);
			const Bucket& bucket = *i;
			Add(bucket.Item);
		}

		// Removes the specified element from the collection.
		template<typename ItemType>
		bool remove(const ItemType& item)
		{
			if (isEmpty())
				return false;
			FindPositionResult pos;
			findPosition(item, pos);
			if (pos.objIdx != -1)
			{
				m_data[pos.objIdx].destroy();
				--m_elementsCount;
				++m_deletedCount;
				return true;
			}
			return false;
		}

		// Removes an element at specified iterator position.
		bool remove(const Iterator& i)
		{
			TK_ASSERT(i.m_collection == this);
			if (i)
			{
				TK_ASSERT(m_data[i.m_index].isOccupied());
				m_data[i.m_index].Delete();
				--m_elementsCount;
				++m_deletedCount;
				return true;
			}
			return false;
		}

	public:
		// Find element with given item in the collection
		template<typename ItemType>
		Iterator find(const ItemType& item) const
		{
			if (isEmpty())
				return End();
			FindPositionResult pos;
			findPosition(item, pos);
			return pos.objIdx != -1 ? Iterator(this, pos.objIdx) : End();
		}

		// Determines whether a collection contains the specified element.
		template<typename ItemType>
		bool contains(const ItemType& item) const
		{
			if (isEmpty())
				return false;
			FindPositionResult pos;
			findPosition(item, pos);
			return pos.objIdx != -1;
		}

	public:
		// Clones other collection into this
		void clone(const TkHashSet& other)
		{
			clear();
			setCapacity(other.capacity(), false);
			for (Iterator i = other.Begin(); i != other.End(); ++i)
				Add(i);
			TK_ASSERT(count() == other.count());
			TK_ASSERT(capacity() == other.capacity());
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
		// The result container of the set item lookup searching.
		struct FindPositionResult
		{
			i32 objIdx;
			i32 freeSlotIdx;
		};

		// Returns a pair of positions: 1st where the object is, 2nd where
		// it would go if you wanted to insert it. 1st is -1
		// if object is not found; 2nd is -1 if it is.
		template<typename ItemType>
		void findPosition(const ItemType& item, FindPositionResult& result) const
		{
			TK_ASSERT(m_size);
			const i32 tableSizeMinusOne = m_size - 1;
			i32 bucketIndex = HashFunc<ItemType>::get(item) & tableSizeMinusOne;
			i32 insertPos = -1;
			i32 numChecks = 0;
			const Bucket* data = m_data;
			result.freeSlotIdx = -1;
			while (numChecks < m_size)
			{
				// Empty bucket
				const Bucket& bucket = data[bucketIndex];
				if (bucket.isEmpty())
				{
					// Found place to insert
					result.objIdx = -1;
					result.freeSlotIdx = insertPos == -1 ? bucketIndex : insertPos;
					return;
				}
				// Deleted bucket
				if (bucket.isDeleted())
				{
					// Keep searching but mark to insert
					if (insertPos == -1)
						insertPos = bucketIndex;
				}
					// Occupied bucket by target item
				else if (bucket.item == item)
				{
					// Found item
					result.objIdx = bucketIndex;
					return;
				}

				numChecks++;
				bucketIndex = (bucketIndex + DICTIONARY_PROB_FUNC(m_size, numChecks)) & tableSizeMinusOne;
			}
			result.objIdx = -1;
			result.freeSlotIdx = insertPos;
		}

		template<typename ItemType>
		Bucket* anAdd(const ItemType& key)
		{
			// Check if need to rehash elements (prevent many deleted elements that use too much of capacity)
			if (m_deletedCount > m_size / DICTIONARY_DEFAULT_SLACK_SCALE)
				compact();

			// Ensure to have enough memory for the next item (in case of new element insertion)
			ensureCapacity((m_elementsCount + 1) * DICTIONARY_DEFAULT_SLACK_SCALE + m_deletedCount);

			// Find location of the item or place to insert it
			FindPositionResult pos;
			findPosition(key, pos);

			// Check if object has been already added
			if (pos.objIdx != -1)
				return nullptr;

			// Insert
			TK_ASSERT(pos.freeSlotIdx != -1);
			++m_elementsCount;
			return &m_data[pos.freeSlotIdx];
		}

		void compact()
		{
			if (m_elementsCount == 0)
			{
				// Fast path if it's empty
				Bucket* data = m_data;
				for (i32 i = 0; i < m_size; ++i)
					data[i].m_state = BucketState::kEmpty;
			}
			else
			{
				// Rebuild entire table completely
				Bucket* oldAllocation = nullptr;
				moveToEmpty(m_alloc, oldAllocation, m_data, m_size);
				m_data = (Bucket*)TkMemory::allocate(m_alloc, m_size * sizeof(Bucket));
				Bucket* data = m_data;
				for (i32 i = 0; i < m_size; ++i)
					data[i].m_state = BucketState::kEmpty;
				Bucket* oldData = oldAllocation;
				FindPositionResult pos;
				for (i32 i = 0; i < m_size; ++i)
				{
					Bucket& oldBucket = oldData[i];
					if (oldBucket.isOccupied())
					{
						findPosition(oldBucket.item, pos);
						TK_ASSERT(pos.freeSlotIdx != -1);
						Bucket* bucket = &m_data[pos.freeSlotIdx];
						TkMemory::moveItems(&bucket->item, &oldBucket.item, 1);
						bucket->m_state = BucketState::kOccupied;
					}
				}
				for (i32 i = 0; i < m_size; ++i)
					oldData[i].free();
			}
			m_deletedCount = 0;
		}

	private:
		TK_FORCE_INLINE static void moveToEmpty(IAllocator& alloc, Bucket*& to, Bucket*& from, const i32 fromSize)
		{
			if (alloc.getTypeId() != AllocatorType::kStack)
				toolkit::swap(to, from);
			else
			{
				to = (Bucket*)TkMemory::allocate(alloc, fromSize);
				Bucket* toData = to;
				Bucket* fromData = from;
				for (i32 i = 0; i < fromSize; ++i)
				{
					Bucket& fromBucket = fromData[i];
					if (fromBucket.isOccupied())
					{
						Bucket& toBucket = toData[i];
						TkMemory::moveItems(&toBucket.item, &fromBucket.item, 1);
						toBucket.m_state = BucketState::kOccupied;
						TkMemory::destructItem(&fromBucket.item);
						fromBucket.m_state = BucketState::kEmpty;
					}
				}
				TkMemory::free(alloc, from);
			}
		}
		
	private:
		i32 m_elementsCount = 0;
		i32 m_deletedCount = 0;
		i32 m_size = 0;
		IAllocator& m_alloc;
		Bucket* m_data = nullptr;

	};
}