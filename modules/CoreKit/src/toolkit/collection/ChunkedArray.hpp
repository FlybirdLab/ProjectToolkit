// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {

	// Template for dynamic array with variable capacity that uses fixed size memory chunks for data storage rather than linear allocation.
	template<typename T, i32 ChunkSize>
	class ChunkedArray
	{
		friend ChunkedArray;

	private:
		// TODO: don't use Array but small struct and don't InlinedArray or Chunk* but Chunk (less dynamic allocations)
		typedef CArray<T> Chunk;

		i32 m_count = 0;
		CArray<Chunk*, InlinedAllocation<32>> m_chunks;

	public:
		ChunkedArray() = default;
		~ChunkedArray()
		{
			m_chunks.clearDelete();
		}

	public:
		// Gets the amount of the elements in the collection.
		CB_FORCE_INLINE i32 count() const
		{
			return m_count;
		}

		// Gets the amount of the elements that can be hold by collection without resizing.
		CB_FORCE_INLINE i32 capacity() const
		{
			return m_chunks.count() * ChunkSize;
		}

		// Returns true if array isn't empty.
		CB_FORCE_INLINE bool hasItems() const
		{
			return m_count != 0;
		}

		// Returns true if collection is empty.
		CB_FORCE_INLINE bool isEmpty() const
		{
			return m_count == 0;
		}

	public:
		// Gets element by index
		CB_FORCE_INLINE T& at(i32 index) const
		{
			CB_ASSERT(index >= 0 && index < m_count);
			return m_chunks[index / ChunkSize]->at(index % ChunkSize);
		}

		// Gets/Sets element by index
		CB_FORCE_INLINE T& operator[](i32 index)
		{
			CB_ASSERT(index >= 0 && index < m_count);
			return m_chunks[index / ChunkSize]->at(index % ChunkSize);
		}

		// Gets/Sets element by index
		CB_FORCE_INLINE const T& operator[](i32 index) const
		{
			CB_ASSERT(index >= 0 && index < m_count);
			return m_chunks[index / ChunkSize]->at(index % ChunkSize);
		}

	public:
		// Chunked array iterator.
		struct Iterator
		{
			friend ChunkedArray;
		private:
			ChunkedArray* m_collection;
			i32 m_chunkIndex;
			i32 m_index;

			Iterator(const ChunkedArray* collection, const i32 index)
				: m_collection(const_cast<ChunkedArray*>(collection))
				, m_chunkIndex(index / ChunkSize)
				, m_index(index % ChunkSize)
			{
			}

		public:
			Iterator()
				: m_collection(nullptr)
				, m_chunkIndex(INVALID_INDEX)
				, m_index(INVALID_INDEX)
			{
			}

			Iterator(const Iterator& i)
				: m_collection(i.m_collection)
				, m_chunkIndex(i.m_chunkIndex)
				, m_index(i.m_index)
			{
			}

			Iterator(Iterator&& i)
				: m_collection(i.m_collection)
				, m_chunkIndex(i.m_chunkIndex)
				, m_index(i.m_index)
			{
			}

		public:
			CB_FORCE_INLINE i32 index() const
			{
				return m_chunkIndex * ChunkSize + m_index;
			}

			CB_FORCE_INLINE bool isEnd() const
			{
				return (m_chunkIndex * ChunkSize + m_index) == m_collection->m_count;
			}

			CB_FORCE_INLINE bool isNotEnd() const
			{
				return (m_chunkIndex * ChunkSize + m_index) != m_collection->m_count;
			}

			CB_FORCE_INLINE T& operator*() const
			{
				return m_collection->m_chunks[m_chunkIndex]->at(m_index);
			}

			CB_FORCE_INLINE T* operator->() const
			{
				return &m_collection->m_chunks[m_chunkIndex]->at(m_index);
			}

			CB_FORCE_INLINE bool operator==(const Iterator& v) const
			{
				return m_collection == v.m_collection && m_chunkIndex == v.m_chunkIndex && m_index == v.m_index;
			}

			CB_FORCE_INLINE bool operator!=(const Iterator& v) const
			{
				return m_collection != v.m_collection || m_chunkIndex != v.m_chunkIndex || m_index != v.m_index;
			}

			Iterator& operator=(const Iterator& v)
			{
				m_collection = v.m_collection;
				m_chunkIndex = v.m_chunkIndex;
				m_index = v.m_index;
				return *this;
			}

			Iterator& operator=(Iterator&& v)
			{
				m_collection = v.m_collection;
				m_chunkIndex = v.m_chunkIndex;
				m_index = v.m_index;
				return *this;
			}

			Iterator& operator++()
			{
				// Check if it is not at end
				if ((m_chunkIndex * ChunkSize + m_index) != m_collection->m_count)
				{
					// Move forward within chunk
					m_index++;

					if (m_index == ChunkSize && m_chunkIndex < m_collection->m_chunks.count() - 1)
					{
						// Move to next chunk
						m_chunkIndex++;
						m_index = 0;
					}
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
				// Check if it's not at beginning
				if (m_index != 0 || m_chunkIndex != 0)
				{
					if (m_index == 0)
					{
						// Move to previous chunk
						m_chunkIndex--;
						m_index = ChunkSize - 1;
					}
					else
					{
						// Move backward within chunk
						m_index--;
					}
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
		// Adds the specified item to the collection.
		T* add(const T& item)
		{
			// Find first chunk with some space
			Chunk* chunk = nullptr;
			for (i32 i = 0; i < m_chunks.count(); i++)
			{
				if (m_chunks[i]->count() < ChunkSize)
				{
					chunk = m_chunks[i];
					break;
				}
			}

			// Allocate chunk if missing
			if (chunk == nullptr)
			{
				chunk = New<Chunk>();
				chunk->setCapacity(ChunkSize);
				m_chunks.add(chunk);
			}

			// Add item
			m_count++;
			chunk->add(item);
			return &chunk->at(chunk->count() - 1);
		}

		// Adds the one item to the collection and returns the reference to it.
		T& addOne()
		{
			// Find first chunk with some space
			Chunk* chunk = nullptr;
			for (i32 i = 0; i < m_chunks.count(); i++)
			{
				if (m_chunks[i]->count() < ChunkSize)
				{
					chunk = m_chunks[i];
					break;
				}
			}

			// Allocate chunk if missing
			if (chunk == nullptr)
			{
				chunk = New<Chunk>();
				chunk->setCapacity(ChunkSize);
				m_chunks.add(chunk);
			}

			// Add item
			m_count++;
			return chunk->addOne();
		}

		// Removes the element at specified iterator position.
		void remove(const Iterator& i)
		{
			if (isEmpty())
				return;
			CB_ASSERT(i.m_collection == this);
			CB_ASSERT(i.m_chunkIndex < m_chunks.count() && i.m_index < ChunkSize);
			CB_ASSERT(i.index() < count());

			auto lastChunkIndex = (m_count - 1) / ChunkSize;
			auto lastIndex = (m_count - 1) % ChunkSize;
			auto& lastChunk = *m_chunks[lastChunkIndex];

			// Check if remove element from the last chunk
			if (i.m_chunkIndex == lastChunkIndex)
			{
				// Remove that item
				lastChunk.removeAt(i.m_index);
			}
			else
			{
				// Swap that item with the last item from the last chunk
				(*m_chunks[i.m_chunkIndex])[i.m_index] = lastChunk[lastIndex];
				lastChunk.removeLast();
			}

			m_count--;
		}

		// Clears the collection but without changing its capacity.
		void clear()
		{
			m_count = 0;
			for (i32 i = 0; i < m_chunks.count(); i++)
			{
				m_chunks[i]->clear();
			}
		}

		// Clears the collection and releases the dynamic memory allocated within the container.
		void release()
		{
			clear();
			m_chunks.clearDelete();
			m_chunks.resize(0);
		}

		// Ensures that collection has a given capacity. It does not preserve collection contents.
		void ensureCapacity(i32 minCapacity)
		{
			i32 minChunks = minCapacity / ChunkSize;
			if (minCapacity % ChunkSize != 0)
				++minChunks;
			while (m_chunks.count() < minChunks)
			{
				auto chunk = New<Chunk>();
				chunk->setCapacity(ChunkSize);
				m_chunks.add(chunk);
			}
		}


		// Resizes that collection to the specified new size. It may not preserve collection contents in case of shrinking.
		void resize(i32 newSize)
		{
			while (newSize < count())
			{
				auto& chunk = m_chunks.last();
				i32 itemsLeft = count() - newSize;
				i32 itemsToRemove = math::min(chunk->count(), itemsLeft);
				chunk->resize(chunk->count() - itemsToRemove);
				m_count -= itemsToRemove;
				if (chunk->count() == 0)
				{
					deletePtr(chunk);
					m_chunks.removeLast();
				}
			}
			if (newSize > count())
			{
				ensureCapacity(newSize);

				// Add more items until reach the new size
				i32 chunkIndex = 0;
				i32 itemsReaming = newSize - count();
				while (itemsReaming != 0)
				{
					CB_ASSERT(chunkIndex != m_chunks.count());
					auto& chunk = m_chunks[chunkIndex];

					// Insert some items to this chunk if can
					const i32 spaceLeft = chunk->capacity() - chunk->count();
					i32 itemsToAdd = math::min(itemsReaming, spaceLeft);
					chunk->resize(chunk->count() + itemsToAdd);
					m_count += itemsToAdd;

					// Update counter
					itemsReaming = newSize - count();

					// Move to the next chunk to fill it
					chunkIndex++;
				}
			}
			CB_ASSERT(newSize == count());
		}


		// Searches for the specified object and returns the zero-based index of the first occurrence within the entire collection.
		i32 find(const T& item) const
		{
			i32 startIndex = 0;
			for (i32 chunkIndex = 0; chunkIndex < m_chunks.count(); chunkIndex++)
			{
				const i32 index = m_chunks[chunkIndex]->Find(item);
				if (index != INVALID_INDEX)
					return startIndex + index;
				startIndex += ChunkSize;
				if (startIndex > m_count)
					break;
			}
			return INVALID_INDEX;
		}

	public:
		Iterator Begin() const
		{
			return Iterator(this, 0);
		}

		Iterator End() const
		{
			return Iterator(this, m_count);
		}

		Iterator iteratorAt(i32 index) const
		{
			return Iterator(this, index);
		}

		CB_FORCE_INLINE Iterator begin()
		{
			return Iterator(this, 0);
		}

		CB_FORCE_INLINE Iterator end()
		{
			return Iterator(this, m_count);
		}

		CB_FORCE_INLINE const Iterator begin() const
		{
			return Iterator(this, 0);
		}

		CB_FORCE_INLINE const Iterator end() const
		{
			return Iterator(this, m_count);
		}
	};
}