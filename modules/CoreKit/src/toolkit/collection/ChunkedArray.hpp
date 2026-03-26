/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target: ChunkedArrayTest.cpp
 */

#pragma once

#include "toolkit/memory/Memory.hpp"
#include <toolkit/memory/ContainerMemory.hpp>
#include <toolkit/collection/StaticArray.hpp>
#include <toolkit/base/Math.hpp>

namespace toolkit
{
	namespace detail
	{
		template <typename T>
		struct TChunkedArrayChunk final : protected TLinearContainerBase<T>
		{
			using ChunkBase = TLinearContainerBase<T>;
			using ChunkBase::m_capacity;
			using ChunkBase::m_alloc;
			using ChunkBase::m_data;
			using ChunkBase::allocateStorage;
			using ChunkBase::freeStorage;
			using ChunkBase::growCapacity;

			explicit TChunkedArrayChunk(IAllocator &alloc = TkMemory::getDefault())
				: ChunkBase{alloc}
			{
			}

			explicit TChunkedArrayChunk(i32 capacity, IAllocator &alloc = TkMemory::getDefault())
				: ChunkBase{alloc}
			{
				setCapacity(capacity, false);
			}

			~TChunkedArrayChunk()
			{
				TkMemory::destructItems(m_data, m_count);
				freeStorage(m_data);
			}

			i32 count() const
			{
				return m_count;
			}

			i32 capacity() const
			{
				return m_capacity;
			}

			T &at(i32 index)
			{
				TK_ASSERT(index >= 0 && index < m_count);
				return m_data[index];
			}

			const T &at(i32 index) const
			{
				TK_ASSERT(index >= 0 && index < m_count);
				return m_data[index];
			}

			T &operator[](i32 index)
			{
				return at(index);
			}

			const T &operator[](i32 index) const
			{
				return at(index);
			}

			void setCapacity(i32 capacity, bool preserveContents = true)
			{
				if (capacity == m_capacity)
					return;

				TK_ASSERT(capacity >= 0);
				const i32 newCount = preserveContents ? TkMath::min(m_count, capacity) : 0;
				T *newData = allocateStorage(capacity);

				if (m_count > 0)
				{
					if (newCount > 0)
						TkMemory::moveItems(newData, m_data, newCount);
					TkMemory::destructItems(m_data, m_count);
				}

				freeStorage(m_data);
				m_data = newData;
				m_capacity = capacity;
				m_count = newCount;
			}

			void ensureCapacity(i32 minCapacity)
			{
				if (m_capacity < minCapacity)
					setCapacity(growCapacity(minCapacity));
			}

			void clear()
			{
				TkMemory::destructItems(m_data, m_count);
				m_count = 0;
			}

			void resize(i32 size)
			{
				TK_ASSERT(size >= 0);
				if (size < m_count)
				{
					TkMemory::destructItems(m_data + size, m_count - size);
				}
				else if (size > m_count)
				{
					ensureCapacity(size);
					TkMemory::constructItems(m_data + m_count, size - m_count);
				}
				m_count = size;
			}

			void add(const T &item)
			{
				TK_ASSERT(m_count < m_capacity);
				TkMemory::constructItems(m_data + m_count, &item, 1);
				++m_count;
			}

			void add(const TChunkedArrayChunk &other)
			{
				TK_ASSERT(m_count + other.m_count <= m_capacity);
				TkMemory::constructItems(m_data + m_count, other.m_data, other.m_count);
				m_count += other.m_count;
			}

			T &addOne()
			{
				TK_ASSERT(m_count < m_capacity);
				TkMemory::constructItems(m_data + m_count, 1);
				++m_count;
				return m_data[m_count - 1];
			}

			void removeAt(i32 index)
			{
				TK_ASSERT(index >= 0 && index < m_count);
				--m_count;
				if (index < m_count)
					m_data[index] = moveTemp(m_data[m_count]);
				TkMemory::destructItems(m_data + m_count, 1);
			}

			void removeLast()
			{
				TK_ASSERT(m_count > 0);
				--m_count;
				TkMemory::destructItems(m_data + m_count, 1);
			}

			i32 find(const T &item) const
			{
				for (i32 i = 0; i < m_count; ++i)
				{
					if (m_data[i] == item)
						return i;
				}
				return kInvalidIndex;
			}

		private:
			i32 m_count = 0;
		};
	}

	/// @brief Dynamic array that stores items across fixed-size heap chunks.
	template <typename T, i32 ChunkSize>
	class TkChunkedArray : protected TChunkedContainerBase<detail::TChunkedArrayChunk<T>, 32>
	{
		using Chunk = detail::TChunkedArrayChunk<T>;
		using Base = TChunkedContainerBase<Chunk, 32>;
		using Base::m_alloc;
		using Base::m_chunks;
		using Base::m_count;
		using Base::appendChunk;
		using Base::destroyChunks;
		using Base::findOrAppendChunkWithSpace;

		friend TkChunkedArray;

	public:
		/// @brief Creates an empty chunked array.
		TkChunkedArray(IAllocator &alloc = TkMemory::getDefault())
			: Base{alloc}
		{
		}
		~TkChunkedArray()
		{
			destroyChunks();
		}
		TkChunkedArray(TkChunkedArray &&other) noexcept
			: Base(moveTemp(other))
		{
		}
		TkChunkedArray &operator=(TkChunkedArray &&other) noexcept
		{
			if (this == &other)
				return *this;

			TK_ASSERT(&m_alloc == &other.m_alloc);
			destroyChunks();
			m_count = other.m_count;
			m_chunks = moveTemp(other.m_chunks);
			other.m_count = 0;
			other.m_chunks.clear();

			return *this;
		}
		TkChunkedArray(const TkChunkedArray &other)
			: Base{other.m_alloc}
		{
			for (i32 i = 0; i < other.m_chunks.count(); ++i)
			{
				auto *src = other.m_chunks[i];
				auto *dst = appendChunk(ChunkSize);
				dst->add(*src);
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
				auto *dst = appendChunk(ChunkSize);
				dst->add(*src);
				m_count += src->count();
			}

			return *this;
		}

	public:
		/// @brief Returns the total number of stored items.
		TK_FORCE_INLINE i32 count() const
		{
			return m_count;
		}

		/// @brief Returns the number of items that fit without allocating another chunk.
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
		/// @brief Returns the item at `index`.
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
		/// @brief Appends one item and returns its stored address.
		T *add(const T &item)
		{
			Chunk *chunk = findOrAppendChunkWithSpace(ChunkSize);

			m_count++;
			chunk->add(item);
			return &chunk->at(chunk->count() - 1);
		}

		/// @brief Appends one default-constructed item and returns it.
		T &addOne()
		{
			Chunk *chunk = findOrAppendChunkWithSpace(ChunkSize);

			m_count++;
			return chunk->addOne();
		}

		/// @brief Removes the item pointed to by the iterator using swap-remove semantics.
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

		/// @brief Clears all items but keeps allocated chunks for reuse.
		void clear()
		{
			m_count = 0;
			for (i32 i = 0; i < m_chunks.count(); i++)
			{
				m_chunks[i]->clear();
			}
		}

		/// @brief Clears the collection and frees all allocated chunks.
		void release()
		{
			destroyChunks();
		}

		/// @brief Ensures enough chunks exist to store `minCapacity` items.
		void ensureCapacity(i32 minCapacity)
		{
			i32 minChunks = minCapacity / ChunkSize;
			if (minCapacity % ChunkSize != 0)
				++minChunks;
			while (m_chunks.count() < minChunks)
			{
				appendChunk(ChunkSize);
			}
		}

		/// @brief Resizes the collection by removing or default-constructing items.
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

		/// @brief Returns the first matching item index or `kInvalidIndex`.
		i32 find(const T &item) const
		{
			i32 startIndex = 0;
			for (i32 chunkIndex = 0; chunkIndex < m_chunks.count(); chunkIndex++)
			{
				const i32 index = m_chunks[chunkIndex]->find(item);
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

	};
}
