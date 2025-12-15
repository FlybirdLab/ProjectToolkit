// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/ContainerAllocation.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit {

	// Template for unordered set of values (without duplicates with O(1) lookup access)
	template<typename T, typename AllocationType = HeapAllocation>
	class THashSet
	{
		friend THashSet;
	public:
		// Describes single portion of space for the item in a hash map.
		struct Bucket
		{
			friend THashSet;
			T Item;

		private:
			BucketState m_state;

			TK_FORCE_INLINE void free()
			{
				if (m_state == BucketState::kOccupied)
					TkMemory::destructItem(&Item);
				m_state = BucketState::kEmpty;
			}

			TK_FORCE_INLINE void Delete()
			{
				m_state = BucketState::kDeleted;
				TkMemory::destructItem(&Item);
			}

			template<typename ItemType>
			TK_FORCE_INLINE void occupy(const ItemType& item)
			{
				TkMemory::constructItems(&Item, &item, 1);
				m_state = BucketState::kOccupied;
			}

			template<typename ItemType>
			TK_FORCE_INLINE void occupy(ItemType&& item)
			{
				TkMemory::moveItems(&Item, &item, 1);
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

		using AllocationData = typename AllocationType::template Data<Bucket>;

	private:
		i32 m_elementsCount = 0;
		i32 m_deletedCount = 0;
		i32 m_size = 0;
		AllocationData m_allocation;

		TK_FORCE_INLINE static void moveToEmpty(AllocationData& to, AllocationData& from, const i32 fromSize)
		{
			if constexpr (AllocationType::HasSwap)
				to.swap(from);
			else
			{
				to.allocate(fromSize);
				Bucket* toData = to.get();
				Bucket* fromData = from.get();
				for (i32 i = 0; i < fromSize; ++i)
				{
					Bucket& fromBucket = fromData[i];
					if (fromBucket.isOccupied())
					{
						Bucket& toBucket = toData[i];
						TkMemory::moveItems(&toBucket.Item, &fromBucket.Item, 1);
						toBucket.m_state = BucketState::kOccupied;
						TkMemory::destructItem(&fromBucket.Item);
						fromBucket.m_state = BucketState::kEmpty;
					}
				}
				from.free();
			}
		}

	public:
		THashSet() = default;
		explicit THashSet(const i32 capacity)
		{
			setCapacity(capacity);
		}
		THashSet(THashSet&& other) noexcept
		{
			m_elementsCount = other.m_elementsCount;
			m_deletedCount = other.m_deletedCount;
			m_size = other.m_size;
			other.m_elementsCount = 0;
			other.m_deletedCount = 0;
			other.m_size = 0;
			moveToEmpty(m_allocation, other.m_allocation, m_size);
		}

		// <param name="other">Other collection to copy</param>
		THashSet(const THashSet& other)
		{
			Clone(other);
		}

		// Clones the data from the other collection.
		THashSet& operator=(const THashSet& other)
		{
			if (this != &other)
				Clone(other);
			return *this;
		}

		// Moves the data from the other collection.
		THashSet& operator=(THashSet&& other) noexcept
		{
			if (this != &other)
			{
				clear();
				m_allocation.free();
				m_elementsCount = other.m_elementsCount;
				m_deletedCount = other.m_deletedCount;
				m_size = other.m_size;
				other.m_elementsCount = 0;
				other.m_deletedCount = 0;
				other.m_size = 0;
				moveToEmpty(m_allocation, other.m_allocation, m_size);
			}
			return *this;
		}

		~THashSet()
		{
			clear();
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
			friend THashSet;
		private:
			THashSet* m_collection;
			i32 m_index;

		public:
			Iterator(THashSet* collection, const i32 index)
				: m_collection(collection)
				, m_index(index)
			{
			}

			Iterator(const THashSet* collection, const i32 index)
				: m_collection(const_cast<THashSet*>(collection))
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
				return m_collection->m_allocation.get()[m_index];
			}

			TK_FORCE_INLINE Bucket* operator->() const
			{
				return &m_collection->m_allocation.get()[m_index];
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
					const Bucket* data = m_collection->m_allocation.get();
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
					const Bucket* data = m_collection->m_allocation.get();
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
				Bucket* data = m_allocation.get();
				for (i32 i = 0; i < m_size; i++)
					data[i].free();
				m_elementsCount = m_deletedCount = 0;
			}
		}

		// Clears the collection and delete value objects.
		// Note: collection must contain pointers to the objects that have public destructor and be allocated using New method.
#if defined(_MSC_VER)
		template<typename = typename TEnableIf<TIsPointer<T>::Value>::Type>
#endif
		void clearDelete()
		{
			for (Iterator i = Begin(); i.isNotEnd(); ++i)
			{
				if (i->Item)
					Delete(i->Item);
			}
			clear();
		}

		// Changes capacity of the collection
		void setCapacity(i32 capacity, const bool preserveContents = true)
		{
			if (capacity == this->capacity())
				return;
			TK_ASSERT(capacity >= 0);
			AllocationData oldAllocation;
			moveToEmpty(oldAllocation, m_allocation, m_size);
			const i32 oldSize = m_size;
			const i32 oldElementsCount = m_elementsCount;
			m_deletedCount = m_elementsCount = 0;
			if (capacity != 0 && (capacity & (capacity - 1)) != 0)
			{
				// Align capacity value to the next power of two (http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2)
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
				m_allocation.allocate(capacity);
				Bucket* data = m_allocation.get();
				for (i32 i = 0; i < capacity; i++)
					data[i].m_state = BucketState::kEmpty;
			}
			m_size = capacity;
			Bucket* oldData = oldAllocation.get();
			if (oldElementsCount != 0 && capacity != 0 && preserveContents)
			{
				FindPositionResult pos;
				for (i32 i = 0; i < oldSize; i++)
				{
					Bucket& oldBucket = oldData[i];
					if (oldBucket.isOccupied())
					{
						findPosition(oldBucket.Item, pos);
						TK_ASSERT(pos.FreeSlotIndex != -1);
						Bucket* bucket = &m_allocation.get()[pos.FreeSlotIndex];
						TkMemory::moveItems(&bucket->Item, &oldBucket.Item, 1);
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
			i32 capacity = m_allocation.calculateCapacityGrow(m_size, minCapacity);
			if (capacity < DICTIONARY_DEFAULT_CAPACITY)
				capacity = DICTIONARY_DEFAULT_CAPACITY;
			setCapacity(capacity, preserveContents);
		}

		// Swaps the contents of collection with the other object without copy operation. Performs fast internal data exchange.
		void swap(THashSet& other)
		{
			if constexpr (AllocationType::HasSwap)
			{
				swap(m_elementsCount, other.m_elementsCount);
				swap(m_deletedCount, other.m_deletedCount);
				swap(m_size, other.m_size);
				m_allocation.swap(other.m_allocation);
			}
			else
			{
				THashSet tmp = moveTemp(other);
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
			if (pos.ObjectIndex != -1)
			{
				m_allocation.get()[pos.ObjectIndex].Delete();
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
				TK_ASSERT(m_allocation.get()[i.m_index].isOccupied());
				m_allocation.get()[i.m_index].Delete();
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
			return pos.ObjectIndex != -1 ? Iterator(this, pos.ObjectIndex) : End();
		}

		// Determines whether a collection contains the specified element.
		template<typename ItemType>
		bool contains(const ItemType& item) const
		{
			if (isEmpty())
				return false;
			FindPositionResult pos;
			findPosition(item, pos);
			return pos.ObjectIndex != -1;
		}

	public:
		// Clones other collection into this
		void clone(const THashSet& other)
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
			i32 ObjectIndex;
			i32 FreeSlotIndex;
		};

		// Returns a pair of positions: 1st where the object is, 2nd where
		// it would go if you wanted to insert it. 1st is -1
		// if object is not found; 2nd is -1 if it is.
		template<typename ItemType>
		void findPosition(const ItemType& item, FindPositionResult& result) const
		{
			TK_ASSERT(m_size);
			const i32 tableSizeMinusOne = m_size - 1;
			i32 bucketIndex = getHash(item) & tableSizeMinusOne;
			i32 insertPos = -1;
			i32 numChecks = 0;
			const Bucket* data = m_allocation.get();
			result.FreeSlotIndex = -1;
			while (numChecks < m_size)
			{
				// Empty bucket
				const Bucket& bucket = data[bucketIndex];
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
					// Occupied bucket by target item
				else if (bucket.Item == item)
				{
					// Found item
					result.ObjectIndex = bucketIndex;
					return;
				}

				numChecks++;
				bucketIndex = (bucketIndex + DICTIONARY_PROB_FUNC(m_size, numChecks)) & tableSizeMinusOne;
			}
			result.ObjectIndex = -1;
			result.FreeSlotIndex = insertPos;
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
			if (pos.ObjectIndex != -1)
				return nullptr;

			// Insert
			TK_ASSERT(pos.FreeSlotIndex != -1);
			++m_elementsCount;
			return &m_allocation.get()[pos.FreeSlotIndex];
		}

		void compact()
		{
			if (m_elementsCount == 0)
			{
				// Fast path if it's empty
				Bucket* data = m_allocation.get();
				for (i32 i = 0; i < m_size; ++i)
					data[i].m_state = BucketState::kEmpty;
			}
			else
			{
				// Rebuild entire table completely
				AllocationData oldAllocation;
				moveToEmpty(oldAllocation, m_allocation, m_size);
				m_allocation.allocate(m_size);
				Bucket* data = m_allocation.get();
				for (i32 i = 0; i < m_size; ++i)
					data[i].m_state = BucketState::kEmpty;
				Bucket* oldData = oldAllocation.get();
				FindPositionResult pos;
				for (i32 i = 0; i < m_size; ++i)
				{
					Bucket& oldBucket = oldData[i];
					if (oldBucket.isOccupied())
					{
						findPosition(oldBucket.Item, pos);
						TK_ASSERT(pos.FreeSlotIndex != -1);
						Bucket* bucket = &m_allocation.get()[pos.FreeSlotIndex];
						TkMemory::moveItems(&bucket->Item, &oldBucket.Item, 1);
						bucket->m_state = BucketState::kOccupied;
					}
				}
				for (i32 i = 0; i < m_size; ++i)
					oldData[i].free();
			}
			m_deletedCount = 0;
		}
	};
}