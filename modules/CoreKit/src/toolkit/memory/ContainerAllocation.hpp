// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/base/Assert.hpp>
#include <toolkit/memory/Memory.hpp>

namespace toolkit {

	// The memory allocation policy that uses inlined memory of the fixed size (no resize support, does not use heap allocations at all).
	template<int Capacity>
	class StackAllocation
	{
	public:
		enum { HasSwap = false };

		template<typename T>
		class alignas(sizeof(void*)) Data
		{
		private:
			byte m_data[Capacity * sizeof(T)];

		public:
			TK_FORCE_INLINE Data() = default;
			TK_FORCE_INLINE ~Data() = default;


			TK_FORCE_INLINE T* get()
			{
				return reinterpret_cast<T*>(m_data);
			}

			TK_FORCE_INLINE const T* get() const
			{
				return reinterpret_cast<const T*>(m_data);
			}

			TK_FORCE_INLINE i32 calculateCapacityGrow(i32 capacity, const i32 minCapacity) const
			{
				TK_ASSERT(minCapacity <= Capacity);
				return Capacity;
			}

			TK_FORCE_INLINE void allocate(const i32 capacity)
			{
#if ENABLE_ASSERTION_LOW_LAYERS
				TK_ASSERT(capacity <= Capacity);
#endif
			}

			TK_FORCE_INLINE void relocate(const i32 capacity, i32 oldCount, i32 newCount)
			{
#if ENABLE_ASSERTION_LOW_LAYERS
				TK_ASSERT(capacity <= Capacity);
#endif
			}

			TK_FORCE_INLINE void free()
			{
			}

			void swap(Data& other)
			{
				// Not supported
			}
		};
	};


	class HeapAllocation
	{
	public:
		enum { HasSwap = true };

		template<typename T>
		class Data
		{
		private:
			T* m_data = nullptr;

		public:
			TK_FORCE_INLINE Data() = default;

			TK_FORCE_INLINE ~Data()
			{
				TkMemory::free(TkMemory::getDefault(), m_data);
			}

			TK_FORCE_INLINE T* get()
			{
				return m_data;
			}

			TK_FORCE_INLINE const T* get() const
			{
				return m_data;
			}

			TK_FORCE_INLINE i32 calculateCapacityGrow(i32 capacity, const i32 minCapacity) const
			{
				if (capacity < minCapacity)
					capacity = minCapacity;
				if (capacity < 8)
				{
					capacity = 8;
				}
				else
				{
					// Round up to the next power of 2 and multiply by 2 (http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2)
					capacity--;
					capacity |= capacity >> 1;
					capacity |= capacity >> 2;
					capacity |= capacity >> 4;
					capacity |= capacity >> 8;
					capacity |= capacity >> 16;
					u64 capacity64 = (u64)(capacity + 1) * 2;
					if (capacity64 > kMaxI32)
						capacity64 = kMaxI32;
					capacity = (i32)capacity64;
				}
				return capacity;
			}

			TK_FORCE_INLINE void allocate(const i32 capacity)
			{
#if  ENABLE_ASSERTION_LOW_LAYERS
				TK_ASSERT(!m_data);
#endif
				m_data = static_cast<T*>(TkMemory::allocate(TkMemory::getDefault(), capacity * sizeof(T)));
#if !CB_RELEASE
				if (!m_data)
					TK_ASSERT(false && "OOM");
#endif
			}

			TK_FORCE_INLINE void relocate(const i32 capacity, i32 oldCount, i32 newCount)
			{
				T* newData = capacity != 0 ? static_cast<T*>(TkMemory::allocate(TkMemory::getDefault(), capacity * sizeof(T))) : nullptr;
#if !CB_RELEASE
				if (!newData && capacity != 0)
					TK_ASSERT(false && "OOM");
#endif

				if (oldCount)
				{
					if (newCount > 0)
						TkMemory::moveItems(newData, m_data, newCount);
					TkMemory::destructItems(m_data, oldCount);
				}

				TkMemory::free(TkMemory::getDefault(), m_data);
				m_data = newData;
			}

			TK_FORCE_INLINE void free()
			{
				TkMemory::free(TkMemory::getDefault(), m_data);
				m_data = nullptr;
			}

			TK_FORCE_INLINE void swap(Data& other)
			{
				toolkit::swap(m_data, other.m_data);
			}
		};
	};


	// The memory allocation policy that uses inlined memory of the fixed size and supports using additional allocation to increase its capacity (eg. via heap allocation).
	template<int Capacity, typename OtherAllocator = HeapAllocation>
	class InlinedAllocation
	{
	public:
		enum { HasSwap = false };

		template<typename T>
		class alignas(sizeof(void*)) Data
		{
		private:
			typedef typename OtherAllocator::template Data<T> OtherData;

			bool m_useOther = false;
			byte m_data[Capacity * sizeof(T)];
			OtherData m_other;

		public:
			TK_FORCE_INLINE Data() = default;
			TK_FORCE_INLINE ~Data() = default;


			TK_FORCE_INLINE T* get()
			{
				return m_useOther ? m_other.get() : reinterpret_cast<T*>(m_data);
			}

			TK_FORCE_INLINE const T* get() const
			{
				return m_useOther ? m_other.get() : reinterpret_cast<const T*>(m_data);
			}

			TK_FORCE_INLINE i32 calculateCapacityGrow(i32 capacity, i32 minCapacity) const
			{
				return minCapacity <= Capacity ? Capacity : m_other.calculateCapacityGrow(capacity, minCapacity);
			}

			TK_FORCE_INLINE void allocate(i32 capacity)
			{
				if (capacity > Capacity)
				{
					m_useOther = true;
					m_other.allocate(capacity);
				}
			}

			TK_FORCE_INLINE void relocate(i32 capacity, i32 oldCount, i32 newCount)
			{
				T* data = reinterpret_cast<T*>(m_data);

				// Check if the new allocation will fit into inlined storage
				if (capacity <= Capacity)
				{
					if (m_useOther)
					{
						// Move the items from other allocation to the inlined storage
						TkMemory::moveItems(data, m_other.get(), newCount);

						// Free the other allocation
						TkMemory::destructItems(m_other.get(), oldCount);
						m_other.free();
						m_useOther = false;
					}
				}
				else
				{
					if (m_useOther)
					{
						// Resize other allocation
						m_other.relocate(capacity, oldCount, newCount);
					}
					else
					{
						// Allocate other allocation
						m_other.allocate(capacity);
						m_useOther = true;

						// Move the items from the inlined storage to the other allocation
						TkMemory::moveItems(m_other.get(), data, newCount);
						TkMemory::destructItems(data, oldCount);
					}
				}
			}

			TK_FORCE_INLINE void free()
			{
				if (m_useOther)
				{
					m_useOther = false;
					m_other.free();
				}
			}

			void swap(Data& other)
			{
				// Not supported
			}
		};
	};

	using DefaultAllocation = HeapAllocation;
}