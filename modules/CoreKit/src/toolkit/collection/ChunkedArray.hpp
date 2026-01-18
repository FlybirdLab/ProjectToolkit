/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/memory/Memory.hpp"
#include <toolkit/collection/StaticArray.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/base/Math.hpp>

namespace toolkit
{

	// Template for dynamic array with variable capacity that uses fixed size memory chunks for data storage rather than linear allocation.
	template <typename T, i32 ChunkSize>
	class TkChunkedArray
	{
		friend TkChunkedArray;

	public:
		TkChunkedArray(IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
		}
		~TkChunkedArray()
		{
			for (auto *ptr : m_chunks)
			{
				if (ptr)
				{
					TkMemory::destructItem(ptr);
					TkMemory::deletePtr(m_alloc, ptr);
				}
			}
			// FIX: Manually delete pointer instead of stack-array's clear. m_chunks.clear();
		}
		TkChunkedArray(TkChunkedArray &&other) noexcept
			: m_count(other.m_count), m_alloc(other.m_alloc), m_chunks(std::move(other.m_chunks))
		{
			other.m_count = 0;
			other.m_chunks.clear();
		}
		TkChunkedArray &operator=(TkChunkedArray &&other) noexcept
		{
			if (this == &other)
				return *this;

			for (auto *ptr : m_chunks)
			{
				if (ptr)
				{
					TkMemory::destructItem(ptr);
					TkMemory::deletePtr(m_alloc, ptr);
				}
			}
			m_chunks.clear();

			// steal
			m_count = other.m_count;
			m_chunks = std::move(other.m_chunks);

			other.m_count = 0;
			other.m_chunks.clear();

			return *this;
		}
		TkChunkedArray(const TkChunkedArray &other)
			: m_count(0), m_alloc(other.m_alloc)
		{
			for (i32 i = 0; i < other.m_chunks.count(); ++i)
			{
				auto *src = other.m_chunks[i];
				auto *dst = TkMemory::newObject<Chunk>(m_alloc);
				dst->setCapacity(ChunkSize);

				for (i32 j = 0; j < src->count(); ++j)
					dst->add((*src)[j]);

				m_chunks.add(dst);
				m_count += src->count();
			}
		}
		TkChunkedArray &operator=(const TkChunkedArray &other)
		{
			if (this == &other)
				return *this;

			release();

			for (i32 i = 0; i < other.m_chunks.count(); ++i)
			{
				auto *src = other.m_chunks[i];
				auto *dst = TkMemory::newObject<Chunk>(m_alloc);
				dst->setCapacity(ChunkSize);

				for (i32 j = 0; j < src->count(); ++j)
					dst->add((*src)[j]);

				m_chunks.add(dst);
				m_count += src->count();
			}

			return *this;
		}

	public:
		// Gets the amount of the elements in the collection.
		TK_FORCE_INLINE i32 count() const
		{
			return m_count;
		}

		// Gets the amount of the elements that can be hold by collection without resizing.
		TK_FORCE_INLINE i32 capacity() const
		{
			return m_chunks.count() * ChunkSize;
		}

		// Returns true if array isn't empty.
		TK_FORCE_INLINE bool hasItems() const
		{
			return m_count != 0;
		}

		// Returns true if collection is empty.
		TK_FORCE_INLINE bool isEmpty() const
		{
			return m_count == 0;
		}

	public:
		// Gets element by index
		TK_FORCE_INLINE T &at(i32 index) const
		{
			TK_ASSERT(index >= 0 && index < m_count);
			return m_chunks[index / ChunkSize]->at(index % ChunkSize);
		}

		// Gets/Sets element by index
		TK_FORCE_INLINE T &operator[](i32 index)
		{
			TK_ASSERT(index >= 0 && index < m_count);
			return m_chunks[index / ChunkSize]->at(index % ChunkSize);
		}

		// Gets/Sets element by index
		TK_FORCE_INLINE const T &operator[](i32 index) const
		{
			TK_ASSERT(index >= 0 && index < m_count);
			return m_chunks[index / ChunkSize]->at(index % ChunkSize);
		}

	public:
		// Chunked array iterator.
		struct Iterator
		{
			friend TkChunkedArray;

		private:
			TkChunkedArray *m_collection;
			i32 m_chunkIndex;
			i32 m_index;

			Iterator(const TkChunkedArray *collection, const i32 index)
				: m_collection(const_cast<TkChunkedArray *>(collection)), m_chunkIndex(index / ChunkSize), m_index(index % ChunkSize)
			{
			}

		public:
			Iterator()
				: m_collection(nullptr), m_chunkIndex(kInvalidIndex), m_index(kInvalidIndex)
			{
			}

			Iterator(const Iterator &i)
				: m_collection(i.m_collection), m_chunkIndex(i.m_chunkIndex), m_index(i.m_index)
			{
			}

			Iterator(Iterator &&i)
				: m_collection(i.m_collection), m_chunkIndex(i.m_chunkIndex), m_index(i.m_index)
			{
			}

		public:
			TK_FORCE_INLINE i32 index() const
			{
				return m_chunkIndex * ChunkSize + m_index;
			}

			TK_FORCE_INLINE bool isEnd() const
			{
				return (m_chunkIndex * ChunkSize + m_index) == m_collection->m_count;
			}

			TK_FORCE_INLINE bool isNotEnd() const
			{
				return (m_chunkIndex * ChunkSize + m_index) != m_collection->m_count;
			}

			TK_FORCE_INLINE T &operator*() const
			{
				return m_collection->m_chunks[m_chunkIndex]->at(m_index);
			}

			TK_FORCE_INLINE T *operator->() const
			{
				return &m_collection->m_chunks[m_chunkIndex]->at(m_index);
			}

			TK_FORCE_INLINE bool operator==(const Iterator &v) const
			{
				return m_collection == v.m_collection && m_chunkIndex == v.m_chunkIndex && m_index == v.m_index;
			}

			TK_FORCE_INLINE bool operator!=(const Iterator &v) const
			{
				return m_collection != v.m_collection || m_chunkIndex != v.m_chunkIndex || m_index != v.m_index;
			}

			Iterator &operator=(const Iterator &v)
			{
				m_collection = v.m_collection;
				m_chunkIndex = v.m_chunkIndex;
				m_index = v.m_index;
				return *this;
			}

			Iterator &operator=(Iterator &&v)
			{
				m_collection = v.m_collection;
				m_chunkIndex = v.m_chunkIndex;
				m_index = v.m_index;
				return *this;
			}

			Iterator &operator++()
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

			Iterator &operator--()
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
		T *add(const T &item)
		{
			// Find first chunk with some space
			Chunk *chunk = nullptr;
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
				chunk = TkMemory::newObject<Chunk>(m_alloc);
				chunk->setCapacity(ChunkSize);
				m_chunks.add(chunk);
			}

			// Add item
			m_count++;
			chunk->add(item);
			return &chunk->at(chunk->count() - 1);
		}

		// Adds the one item to the collection and returns the reference to it.
		T &addOne()
		{
			// Find first chunk with some space
			Chunk *chunk = nullptr;
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
				chunk = TkMemory::newObject<Chunk>();
				chunk->setCapacity(ChunkSize);
				m_chunks.add(chunk);
			}

			// Add item
			m_count++;
			return chunk->addOne();
		}

		// Removes the element at specified iterator position.
		void remove(const Iterator &i)
		{
			if (isEmpty())
				return;
			TK_ASSERT(i.m_collection == this);
			TK_ASSERT(i.m_chunkIndex < m_chunks.count() && i.m_index < ChunkSize);
			TK_ASSERT(i.index() < count());

			auto lastChunkIndex = (m_count - 1) / ChunkSize;
			auto lastIndex = (m_count - 1) % ChunkSize;
			auto &lastChunk = *m_chunks[lastChunkIndex];

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
				auto chunk = TkMemory::newObject<Chunk>(m_alloc);
				chunk->setCapacity(ChunkSize);
				m_chunks.add(chunk);
			}
		}

		// Resizes that collection to the specified new size. It may not preserve collection contents in case of shrinking.
		void resize(i32 newSize)
		{
			while (newSize < count())
			{
				auto &chunk = m_chunks.last();
				i32 itemsLeft = count() - newSize;
				i32 itemsToRemove = TkMath::min(chunk->count(), itemsLeft);
				chunk->resize(chunk->count() - itemsToRemove);
				m_count -= itemsToRemove;
				if (chunk->count() == 0)
				{
					TkMemory::deletePtr(m_alloc, chunk);
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
					TK_ASSERT(chunkIndex != m_chunks.count());
					auto &chunk = m_chunks[chunkIndex];

					// Insert some items to this chunk if can
					const i32 spaceLeft = chunk->capacity() - chunk->count();
					i32 itemsToAdd = TkMath::min(itemsReaming, spaceLeft);
					chunk->resize(chunk->count() + itemsToAdd);
					m_count += itemsToAdd;

					// Update counter
					itemsReaming = newSize - count();

					// Move to the next chunk to fill it
					chunkIndex++;
				}
			}
			TK_ASSERT(newSize == count());
		}

		// Searches for the specified object and returns the zero-based index of the first occurrence within the entire collection.
		i32 find(const T &item) const
		{
			i32 startIndex = 0;
			for (i32 chunkIndex = 0; chunkIndex < m_chunks.count(); chunkIndex++)
			{
				const i32 index = m_chunks[chunkIndex]->Find(item);
				if (index != kInvalidIndex)
					return startIndex + index;
				startIndex += ChunkSize;
				if (startIndex > m_count)
					break;
			}
			return kInvalidIndex;
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

		TK_FORCE_INLINE Iterator begin()
		{
			return Iterator(this, 0);
		}

		TK_FORCE_INLINE Iterator end()
		{
			return Iterator(this, m_count);
		}

		TK_FORCE_INLINE const Iterator begin() const
		{
			return Iterator(this, 0);
		}

		TK_FORCE_INLINE const Iterator end() const
		{
			return Iterator(this, m_count);
		}

	private:
		// TODO: don't use Array but small struct and don't InlinedArray or Chunk* but Chunk (less dynamic allocations)
		typedef TkArray<T> Chunk;

		i32 m_count = 0;
		IAllocator &m_alloc;
		TkStaticArray<Chunk *, 32> m_chunks;
	};
}