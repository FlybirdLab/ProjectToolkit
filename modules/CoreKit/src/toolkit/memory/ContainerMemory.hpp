// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/Memory.hpp>
#include <toolkit/collection/StaticArray.hpp>

namespace toolkit
{
	i32 calCapacityGrow(IAllocator &alloc, i32 capacity, const i32 minCapacity);

	template <typename T>
	class TLinearContainerBase
	{
	protected:
		explicit TLinearContainerBase(IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
		}

		TLinearContainerBase(TLinearContainerBase &&other) noexcept
			: m_capacity(other.m_capacity), m_alloc(other.m_alloc), m_data(other.m_data)
		{
			other.m_capacity = 0;
			other.m_data = nullptr;
		}

		TK_FORCE_INLINE T *allocateStorage(i32 itemCount)
		{
			if (itemCount <= 0)
				return nullptr;
			auto *data = static_cast<T *>(TkMemory::allocate(m_alloc, static_cast<psize>(itemCount) * sizeof(T), alignof(T)));
			TK_ASSERT(data != nullptr && "OutOfMemory Error");
			return data;
		}

		TK_FORCE_INLINE void freeStorage(T *ptr)
		{
			if (ptr)
				TkMemory::free(m_alloc, ptr);
		}

		TK_FORCE_INLINE void releaseStorage()
		{
			freeStorage(m_data);
			m_data = nullptr;
			m_capacity = 0;
		}

		TK_FORCE_INLINE i32 growCapacity(i32 minCapacity) const
		{
			return calCapacityGrow(m_alloc, m_capacity, minCapacity);
		}

		TK_FORCE_INLINE void stealStorageFrom(TLinearContainerBase &other)
		{
			m_capacity = other.m_capacity;
			m_data = other.m_data;
			other.m_capacity = 0;
			other.m_data = nullptr;
		}

	protected:
		i32 m_capacity = 0;
		IAllocator &m_alloc;
		T *m_data = nullptr;
	};

	template <typename ChunkType, i32 InlineCapacity>
	class TChunkedContainerBase
	{
	protected:
		explicit TChunkedContainerBase(IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
		}

		TChunkedContainerBase(TChunkedContainerBase &&other) noexcept
			: m_count(other.m_count), m_alloc(other.m_alloc), m_chunks(moveTemp(other.m_chunks))
		{
			other.m_count = 0;
			other.m_chunks.clear();
		}

		TK_FORCE_INLINE ChunkType *newChunk()
		{
			return TkMemory::newObject<ChunkType>(m_alloc, m_alloc);
		}

		TK_FORCE_INLINE ChunkType *newChunk(i32 capacity)
		{
			return TkMemory::newObject<ChunkType>(m_alloc, capacity, m_alloc);
		}

		TK_FORCE_INLINE ChunkType *appendChunk()
		{
			auto *chunk = newChunk();
			m_chunks.add(chunk);
			return chunk;
		}

		TK_FORCE_INLINE ChunkType *appendChunk(i32 capacity)
		{
			auto *chunk = newChunk(capacity);
			m_chunks.add(chunk);
			return chunk;
		}

		TK_FORCE_INLINE void destroyChunks()
		{
			for (auto *ptr : m_chunks)
			{
				if (ptr)
					TkMemory::deletePtr(m_alloc, ptr);
			}
			m_chunks.clear();
			m_count = 0;
		}

		TK_FORCE_INLINE ChunkType *findChunkWithSpace(i32 chunkSize) const
		{
			for (i32 i = 0; i < m_chunks.count(); ++i)
			{
				if (m_chunks[i] != nullptr && m_chunks[i]->count() < chunkSize)
					return m_chunks[i];
			}
			return nullptr;
		}

		TK_FORCE_INLINE ChunkType *findOrAppendChunkWithSpace(i32 chunkSize)
		{
			if (auto *chunk = findChunkWithSpace(chunkSize))
				return chunk;
			return appendChunk(chunkSize);
		}

	protected:
		i32 m_count = 0;
		IAllocator &m_alloc;
		TkStaticArray<ChunkType *, InlineCapacity> m_chunks;
	};
}
