// Created by cubevlmu on 2025/8/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/ContainerMemory.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit
{

	// Template for dynamic array with variable capacity.
	template <typename T>
	class TkArray final
	{
		friend class TkArray;

	public:
		TK_FORCE_INLINE TkArray(IAllocator &alloc = TkMemory::getDefault())
			: m_count{0}, m_capacity{0}, m_data{nullptr}, m_alloc{alloc}
		{
		}

		explicit TkArray(const i32 capacity, IAllocator &alloc = TkMemory::getDefault())
			: m_count{0}, m_capacity{capacity}, m_alloc{alloc}
		{
			if (capacity > 0)
				m_data = (T*)TkMemory::allocate(alloc, capacity * sizeof(T), alignof(T));
			else
				m_data = nullptr;
		}

		TkArray(std::initializer_list<T> initList, IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
			m_count = m_capacity = static_cast<i32>(initList.size());
			if (m_count > 0)
			{
				m_data = TkMemory::allocate(m_alloc, m_count * sizeof(T), alignof(T));
				TkMemory::constructItems(m_data, initList.begin(), m_count);
			}
		}

		TkArray(const T *data, const i32 length, IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
			TK_ASSERT(length >= 0);
			m_count = m_capacity = length;
			if (length > 0)
			{
				m_data = TkMemory::allocate(alloc, length * sizeof(T), alignof(T));
				TkMemory::constructItems(m_data, data, length);
			}
		}

		// Copy constructor
		TkArray(const TkArray &other)
			: m_alloc{other.m_alloc}
		{
			m_count = m_capacity = other.m_count;
			if (m_capacity > 0)
			{
				m_data = TkMemory::allocate(m_alloc, m_capacity * sizeof(T), alignof(T));
				TkMemory::constructItems(m_data, other.get(), other.m_count);
			}
		}

		TkArray(const TkArray &other, i32 extraSize, IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
			TK_ASSERT(extraSize >= 0);
			m_count = m_capacity = other.m_count + extraSize;
			if (m_capacity > 0)
			{
				m_data = TkMemory::allocate(m_alloc, m_capacity * sizeof(T), alignof(T));
				TkMemory::constructItems(m_data, other.get(), other.m_count);
				TkMemory::constructItems(m_data + other.m_count, extraSize);
			}
		}

		template <typename OtherT = T>
		explicit TkArray(const TkArray<OtherT> &other) noexcept
		{
			m_capacity = other.capacity();
			m_count = other.count();
			//			m_alloc = other.m_alloc;
			if (m_capacity > 0)
			{
				m_data = TkMemory::allocate(m_alloc, m_capacity * sizeof(T), alignof(T));
				TkMemory::constructItems(m_data, other.get(), m_count);
			}
		}

		// Move constructor
		TkArray(TkArray &&other) noexcept
			: m_alloc{other.m_alloc}
		{
			m_count = other.m_count;
			m_capacity = other.m_capacity;
			other.m_count = 0;
			other.m_capacity = 0;
			moveToEmpty(m_alloc, m_data, other.m_data, m_count, m_capacity);
		}

		TkArray &operator=(std::initializer_list<T> initList) noexcept
		{
			clear();
			if (initList.size() > 0)
			{
				ensureCapacity(static_cast<i32>(initList.size()));
				m_count = static_cast<i32>(initList.size());
				TkMemory::constructItems(m_data, initList.begin(), m_count);
			}
			return *this;
		}

		TkArray &operator=(const TkArray &other) noexcept
		{
			if (this != &other)
			{
				TkMemory::destructItems(m_data, m_count);
				if (m_capacity < other.count())
				{
					TkMemory::free(m_alloc, m_data);
					m_capacity = other.count();
					m_data = (T *)TkMemory::allocate(m_alloc, m_capacity * sizeof(T), alignof(T));
				}
				m_count = other.count();
				TkMemory::constructItems(m_data, other.get(), m_count);
			}
			return *this;
		}

		TkArray &operator=(TkArray &&other) noexcept
		{
			if (this != &other)
			{
				TkMemory::destructItems(m_data, m_count);
				TkMemory::free(m_alloc, m_data);
				m_count = other.m_count;
				m_capacity = other.m_capacity;
				m_alloc = other.m_alloc;
				other.m_count = 0;
				other.m_capacity = 0;
				moveToEmpty(m_data, other.m_data, m_count, m_capacity);
			}
			return *this;
		}

		~TkArray()
		{
			TkMemory::destructItems(m_data, m_count);
			TkMemory::free(m_alloc, m_data);
		}

	public:
		TK_FORCE_INLINE i32 count() const
		{
			return m_count;
		}
		TK_FORCE_INLINE i32 capacity() const
		{
			return m_capacity;
		}
		TK_FORCE_INLINE bool hasItems() const
		{
			return m_count != 0;
		}
		TK_FORCE_INLINE bool isEmpty() const
		{
			return m_count == 0;
		}

		bool isValidIndex(const i32 index) const
		{
			return index < m_count && index >= 0;
		}

		TK_FORCE_INLINE T *get()
		{
			return m_data;
		}

		TK_FORCE_INLINE const T *get() const
		{
			return m_data;
		}

		TK_FORCE_INLINE T &at(const i32 index)
		{
			TK_ASSERT(index >= 0 && index < m_count);
			return m_data[index];
		}
		TK_FORCE_INLINE const T &at(const i32 index) const
		{
			TK_ASSERT(index >= 0 && index < m_count);
			return m_data[index];
		}
		TK_FORCE_INLINE T &operator[](const i32 index)
		{
			TK_ASSERT(index >= 0 && index < m_count);
			return m_data[index];
		}
		TK_FORCE_INLINE const T &operator[](const i32 index) const
		{
			TK_ASSERT(index >= 0 && index < m_count);
			return m_data[index];
		}

		TK_FORCE_INLINE T &last()
		{
			TK_ASSERT(m_count > 0);
			return m_data[m_count - 1];
		}
		TK_FORCE_INLINE const T &last() const
		{
			TK_ASSERT(m_count > 0);
			return m_data[m_count - 1];
		}

		TK_FORCE_INLINE T &first()
		{
			TK_ASSERT(m_count > 0);
			return m_data[0];
		}
		TK_FORCE_INLINE const T &first() const
		{
			TK_ASSERT(m_count > 0);
			return m_data[0];
		}

		TK_FORCE_INLINE const T &back() const
		{
			return last();
		}
		TK_FORCE_INLINE T &back()
		{
			return last();
		}

	public:
		TK_FORCE_INLINE T *begin()
		{
			return m_data;
		}
		TK_FORCE_INLINE T *end()
		{
			return m_data + m_count;
		}
		TK_FORCE_INLINE const T *begin() const
		{
			return m_data;
		}
		TK_FORCE_INLINE const T *end() const
		{
			return m_data + m_count;
		}

	public:
		TK_FORCE_INLINE void clear()
		{
			TkMemory::destructItems(m_data, m_count);
			m_count = 0;
		}

		void clearDelete()
			requires(TIsPointer<T>::Value)
		{
			T *data = get();
			for (i32 i = 0; i < m_count; i++)
			{
				if (data[i])
					TkMemory::deletePtr(m_alloc, data[i]);
			}
			clear();
		}

		void clearDelete()
			requires(!TIsPointer<T>::Value)
		{
			clear();
		}

		void setCapacity(const i32 capacity, const bool preserveContents = true)
		{
			if (capacity == m_capacity)
				return;
			TK_ASSERT(capacity >= 0);
			const i32 count = preserveContents ? (m_count < capacity ? m_count : capacity) : 0;

			// Data allocation
			T *newData = capacity != 0 ? static_cast<T *>(TkMemory::allocate(m_alloc, capacity * sizeof(T))) : nullptr;
			if (!newData && capacity != 0)
				TK_ASSERT(false && "OutOfMemory Error");

			if (m_count)
			{
				if (count > 0)
					TkMemory::moveItems(newData, m_data, count);
				TkMemory::destructItems(m_data, m_count);
			}

			TkMemory::free(m_alloc, m_data);
			m_data = newData;
			//

			m_capacity = capacity;
			m_count = count;
		}

		void resize(const i32 size, const bool preserveContents = true)
		{
			if (m_count > size)
				TkMemory::destructItems(m_data + size, m_count - size);
			else
			{
				ensureCapacity(size, preserveContents);
				TkMemory::constructItems(m_data + m_count, size - m_count);
			}
			m_count = size;
		}

		void ensureCapacity(const i32 minCapacity, const bool preserveContents = true)
		{
			if (m_capacity < minCapacity)
			{
				const i32 capacity = toolkit::calCapacityGrow(m_alloc, m_capacity, minCapacity);
				setCapacity(capacity, preserveContents);
			}
		}

		TK_FORCE_INLINE void reserve(u32 capacity)
		{
			ensureCapacity(capacity);
		}

		void setAll(const T &value)
		{
			T *data = m_data;
			for (i32 i = 0; i < m_count; i++)
				data[i] = value;
		}

		void set(const T *data, const i32 count)
		{
			ensureCapacity(count, false);
			TkMemory::destructItems(m_data, m_count);
			m_count = count;
			TkMemory::constructItems(m_data, data, m_count);
		}

		void add(const T &item)
		{
			ensureCapacity(m_count + 1);
			TkMemory::constructItems(m_data + m_count, &item, 1);
			++m_count;
		}

		void add(T &&item)
		{
			ensureCapacity(m_count + 1);
			TkMemory::moveItems(m_data + m_count, &item, 1);
			++m_count;
		}

		void add(const T *items, const i32 count)
		{
			ensureCapacity(m_count + count);
			TkMemory::constructItems(m_data + m_count, items, count);
			m_count += count;
		}

		template <typename OtherT>
		TK_FORCE_INLINE void add(const TkArray<OtherT> &other)
		{
			add(other.get(), other.count());
		}

		TK_FORCE_INLINE void addUnique(const T &item)
		{
			if (!contains(item))
				add(item);
		}

		TK_FORCE_INLINE void addDefault(const i32 count = 1)
		{
			ensureCapacity(m_count + count);
			TkMemory::constructItems(m_data + m_count, count);
			m_count += count;
		}

		TK_FORCE_INLINE void addUninitialized(const i32 count = 1)
		{
			ensureCapacity(m_count + count);
			m_count += count;
		}

		TK_FORCE_INLINE T &addOne()
		{
			ensureCapacity(m_count + 1);
			TkMemory::constructItems(m_data + m_count, 1);
			++m_count;
			return m_data[m_count - 1];
		}

		void addZeroed(const i32 count = 1)
		{
			static_assert(toolkit::TIsTriviallyConstructible<T>(), "addZeroed requires trivial type");
			ensureCapacity(m_count + count);
			memset(m_data + m_count, 0, count * sizeof(T));
			m_count += count;
		}

		void insert(const i32 index, const T &item)
		{
			TK_ASSERT(index >= 0 && index <= m_count);
			ensureCapacity(m_count + 1);
			T *data = m_data;
			TkMemory::constructItems(data + m_count, 1);
			for (i32 i = m_count - 1; i >= index; i--)
				data[i + 1] = data[i];
			m_count++;
			data[index] = item;
		}

		void insert(const i32 index, T &&item)
		{
			TK_ASSERT(index >= 0 && index <= m_count);
			ensureCapacity(m_count + 1);
			T *data = m_data;
			TkMemory::constructItems(data + m_count, 1);
			for (i32 i = m_count - 1; i >= index; i--)
				data[i + 1] = moveTemp(data[i]);
			++m_count;
			data[index] = moveTemp(item);
		}

		void insert(const i32 index)
		{
			TK_ASSERT(index >= 0 && index <= m_count);
			ensureCapacity(m_count + 1);
			T *data = m_data;
			TkMemory::constructItems(data + m_count, 1);
			for (i32 i = m_count - 1; i >= index; i--)
				data[i + 1] = data[i];
			++m_count;
		}

		template <typename TComparableType>
		bool contains(const TComparableType &item) const
		{
			const T *data = m_data;
			for (i32 i = 0; i < m_count; i++)
			{
				if (data[i] == item)
					return true;
			}
			return false;
		}

		bool removeKeepOrder(const T &item)
		{
			const i32 index = find(item);
			if (index == -1)
				return false;
			removeAtKeepOrder(index);
			return true;
		}

		void removeAllKeepOrder(const T &item)
		{
			for (i32 i = count() - 1; i >= 0; --i)
			{
				if (m_data[i] == item)
				{
					removeAtKeepOrder(i);
					if (isEmpty())
						break;
				}
			}
		}

		void removeAtKeepOrder(const i32 index)
		{
			TK_ASSERT(index < m_count && index >= 0);
			--m_count;
			T *data = m_data;
			if (index < m_count)
			{
				T *dst = data + index;
				T *src = data + (index + 1);
				const i32 count = m_count - index;
				for (i32 i = 0; i < count; ++i)
					dst[i] = moveTemp(src[i]);
			}
			TkMemory::destructItems(data + m_count, 1);
		}

		bool remove(const T &item)
		{
			const i32 index = find(item);
			if (index == -1)
				return false;
			removeAt(index);
			return true;
		}

		void removeAll(const T &item)
		{
			for (i32 i = count() - 1; i >= 0; --i)
			{
				if (m_data[i] == item)
				{
					removeAt(i);
					if (isEmpty())
						break;
				}
			}
		}

		void removeAt(const i32 index)
		{
			TK_ASSERT(index < m_count && index >= 0);
			--m_count;
			T *data = m_data;
			if (m_count)
				data[index] = data[m_count];
			TkMemory::destructItems(data + m_count, 1);
		}

		void removeLast()
		{
			TK_ASSERT(m_count > 0);
			--m_count;
			TkMemory::destructItems(m_data + m_count, 1);
		}

		void swap(TkArray &other)
		{
			toolkit::swap(m_data, other.m_data);
			toolkit::swap(m_count, other.m_count);
			toolkit::swap(m_capacity, other.m_capacity);
		}

		void reverse()
		{
			T *data = m_data;
			const i32 count = m_count / 2;
			for (i32 i = 0; i < count; ++i)
				::toolkit::swap(data[i], data[m_count - i - 1]);
		}

	public:
		TK_FORCE_INLINE void push(const T &item)
		{
			add(item);
		}

		TK_FORCE_INLINE T pop()
		{
			T item = moveTemp(last());
			removeLast();
			return item;
		}

		TK_FORCE_INLINE T &peek()
		{
			TK_ASSERT(m_count > 0);
			return m_data[m_count - 1];
		}

		TK_FORCE_INLINE const T &peek() const
		{
			TK_ASSERT(m_count > 0);
			return m_data[m_count - 1];
		}

	public:
		void enqueue(const T &item)
		{
			add(item);
		}

		void enqueue(T &&item)
		{
			add(moveTemp(item));
		}

		T dequeue()
		{
			TK_ASSERT(hasItems());
			T item = moveTemp(m_data[0]);
			removeAtKeepOrder(0);
			return item;
		}

	public:
		template <typename ComparableType>
		TK_FORCE_INLINE bool find(const ComparableType &item, i32 &index) const
		{
			index = find(item);
			return index != -1;
		}

		template <typename ComparableType>
		i32 find(const ComparableType &item) const
		{
			if (m_count > 0)
			{
				const T *TK_RESTRICT start = m_data;
				for (const T *TK_RESTRICT data = start, *TK_RESTRICT dataEnd = data + m_count; data != dataEnd; ++data)
				{
					if (*data == item)
						return static_cast<i32>(data - start);
				}
			}
			return -1;
		}

		template <typename ComparableType>
		TK_FORCE_INLINE bool findLast(const ComparableType &item, int &index) const
		{
			index = findLast(item);
			return index != -1;
		}

		template <typename ComparableType>
		i32 findLast(const ComparableType &item) const
		{
			if (m_count > 0)
			{
				const T *TK_RESTRICT end = m_data + m_count;
				for (const T *TK_RESTRICT data = end, *TK_RESTRICT dataStart = data - m_count; data != dataStart;)
				{
					--data;
					if (*data == item)
						return static_cast<i32>(data - dataStart);
				}
			}
			return -1;
		}

	public:
		template <typename OtherT = T>
		bool operator==(const TkArray<OtherT> &other) const
		{
			if (m_count == other.count())
			{
				const T *data = m_data;
				const T *otherData = other.get();
				for (i32 i = 0; i < m_count; i++)
				{
					if (!(data[i] == otherData[i]))
						return false;
				}
				return true;
			}
			return false;
		}

		template <typename OtherT = T>
		bool operator!=(const TkArray<OtherT> &other) const
		{
			return !operator==(other);
		}

	public:
		// The collection iterator
		struct Iterator
		{
			friend TkArray;

		private:
			TkArray *m_array;
			i32 m_index;

			Iterator(TkArray *array, const i32 index)
				: m_array(array), m_index(index)
			{
			}

			Iterator(const TkArray *array, const i32 index)
				: m_array(const_cast<TkArray *>(array)), m_index(index)
			{
			}

		public:
			Iterator()
				: m_array(nullptr), m_index(-1)
			{
			}

			Iterator(const Iterator &i)
				: m_array(i.m_array), m_index(i.m_index)
			{
			}

			Iterator(Iterator &&i)
				: m_array(i.m_array), m_index(i.m_index)
			{
			}

		public:
			TK_FORCE_INLINE TkArray *getTArray() const
			{
				return m_array;
			}

			TK_FORCE_INLINE i32 getIndex() const
			{
				return m_index;
			}

			TK_FORCE_INLINE bool isEnd() const
			{
				return m_index == m_array->m_count;
			}

			TK_FORCE_INLINE bool isNotEnd() const
			{
				return m_index != m_array->m_count;
			}

			TK_FORCE_INLINE T &operator*() const
			{
				return m_array->get()[m_index];
			}

			TK_FORCE_INLINE T *operator->() const
			{
				return &m_array->get()[m_index];
			}

			TK_FORCE_INLINE bool operator==(const Iterator &v) const
			{
				return m_array == v.m_array && m_index == v.m_index;
			}

			TK_FORCE_INLINE bool operator!=(const Iterator &v) const
			{
				return m_array != v.m_array || m_index != v.m_index;
			}

			Iterator &operator=(const Iterator &v)
			{
				m_array = v.m_array;
				m_index = v.m_index;
				return *this;
			}

			Iterator &operator=(Iterator &&v)
			{
				m_array = v.m_array;
				m_index = v.m_index;
				return *this;
			}

			Iterator &operator++()
			{
				if (m_index != m_array->m_count)
					m_index++;
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator temp = *this;
				if (m_index != m_array->m_count)
					m_index++;
				return temp;
			}

			Iterator &operator--()
			{
				if (m_index > 0)
					--m_index;
				return *this;
			}

			Iterator operator--(int)
			{
				Iterator temp = *this;
				if (m_index > 0)
					--m_index;
				return temp;
			}
		};

	public:
		TK_FORCE_INLINE Iterator Begin() const
		{
			return Iterator(this, 0);
		}
		TK_FORCE_INLINE Iterator End() const
		{
			return Iterator(this, m_count);
		}

	private:
		i32 m_count;
		i32 m_capacity;
		T *m_data;
		IAllocator &m_alloc;

		TK_FORCE_INLINE static void moveToEmpty(IAllocator &alloc, void *&to, void *&from, const i32 fromCount, const i32 fromCapacity)
		{
			toolkit::swap(to, from);
		}
	};

}

template <typename T>
void *operator new(const size_t size, toolkit::TkArray<T> &array)
{
	using namespace toolkit;
	TK_ASSERT(size == sizeof(T));
	const i32 index = array.count();
	array.addUninitialized(1);
	return &array[index];
}

template <typename T>
void *operator new(const size_t size, toolkit::TkArray<T> &array, const toolkit::i32 index)
{
	using namespace toolkit;
	TK_ASSERT(size == sizeof(T));
	array.insert(index);
	return &array[index];
}