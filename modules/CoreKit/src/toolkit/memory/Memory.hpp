/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/base/TypeTraits.hpp>
#if TK_DEBUG
#include <string_view>
#endif
#if TK_POSIX
#include <string.h>
#endif

#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace toolkit
{
#define TK_SAFE_DELETE(x)                                            \
	if (x)                                                           \
	{                                                                \
		::toolkit::TkMemory::deletePtr(TkMemory::getDefault(), (x)); \
		(x) = nullptr;                                               \
	}

	struct AllocatorBase
	{
#if TK_DEBUG
		AllocatorBase();
		AllocatorBase(const AllocatorBase &) = delete;
		AllocatorBase &operator=(const AllocatorBase &) = delete;
		~AllocatorBase();
#else
		AllocatorBase() = default;
		AllocatorBase(const AllocatorBase &) = delete;
		AllocatorBase &operator=(const AllocatorBase &) = delete;
		~AllocatorBase() = default;
#endif
	};
	using AllocFn = void *(*)(AllocatorBase * ctx, psize size, psize align);
	using FreeFn = void (*)(AllocatorBase *ctx, void *ptr);
	using ReallocFn = void *(*)(AllocatorBase * ctx, void *ptr, psize newSize, psize oldSize, psize align);

	enum class AllocatorType : u8
	{
		kUnkown,
		kTag,
		kArena,
		kStd,
		kStack,
		kPage
	};

	struct IAllocator
	{
		friend class TkMemory;

	public:
		IAllocator(AllocFn, ReallocFn, FreeFn, AllocatorBase *, AllocatorType);
		~IAllocator();

		operator AllocatorBase *()
		{
			return ctx;
		}
		operator AllocatorType()
		{
			return typeId;
		}

		AllocatorType getTypeId() const
		{
			return typeId;
		}

	private:
		AllocatorBase *ctx;
		AllocFn alloc;
		ReallocFn realloc;
		FreeFn free;
		AllocatorType typeId;
#if TK_DEBUG
		u64 debugGeneration = 0;
#endif
	};

	struct AllocatorMeta
	{
#if TK_DEBUG
		const std::string_view tag{};
#endif
		const IAllocator *parent = nullptr;
		u32 typeId = 0;
	};

	/// memory management system
	class TkMemory
	{
	public:
		// Rounds up the input value to the next power of 2 to be used as bigger memory allocation block. Handles overflow.
		// Reference: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		inline static i32 roundUpToPod2(i32 capacity)
		{
			capacity--;
			capacity |= capacity >> 1;
			capacity |= capacity >> 2;
			capacity |= capacity >> 4;
			capacity |= capacity >> 8;
			capacity |= capacity >> 16;
			u64 capacity64 = (u64)(capacity + 1) * 2;
			if (capacity64 > kMaxI32)
				capacity64 = kMaxI32;
			return (i32)capacity64;
		}

		// Aligns the input value to the next power of 2 to be used as bigger memory allocation block.
		// Reference: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		inline static i32 alignToPod2(i32 capacity)
		{
			capacity--;
			capacity |= capacity >> 1;
			capacity |= capacity >> 2;
			capacity |= capacity >> 4;
			capacity |= capacity >> 8;
			capacity |= capacity >> 16;
			capacity++;
			return capacity;
		}

		template <typename T>
		TK_FORCE_INLINE static void constructItem(T *dst)
			requires(!TTriviallyConstructibleType<T>)
		{
			new (dst) T();
		}

		template <typename T>
		TK_FORCE_INLINE static void constructItem(T *)
			requires(TTriviallyConstructibleType<T>)
		{
		}

		template <typename T>
		TK_FORCE_INLINE static void constructItems(T *dst, i32 count)
			requires(!TTriviallyConstructibleType<T>)
		{
			while (count--)
			{
				new (dst) T();
				++(T *&)dst;
			}
		}

		template <typename T>
		TK_FORCE_INLINE static void constructItems(T *, i32)
			requires(TTriviallyConstructibleType<T>)
		{
		}

		template <typename T, typename U>
		TK_FORCE_INLINE static void constructItems(T *dst, const U *src, i32 count)
			requires(!TBitwiseConstructibleFrom<T, U>)
		{
			while (count--)
			{
				new (dst) T(*src);
				++(T *&)dst;
				++src;
			}
		}

		template <typename T, typename U>
		TK_FORCE_INLINE static void constructItems(T *dst, const U *src, i32 count)
			requires(TBitwiseConstructibleFrom<T, U>)
		{
			memoryCopy((void *)dst, (void *)src, count * sizeof(U));
		}

		template <typename T>
		TK_FORCE_INLINE static void destructItem(T *dst)
			requires(!TTriviallyDestructibleType<T>)
		{
			dst->~T();
		}

		template <typename T>
		TK_FORCE_INLINE static void destructItem(T *)
			requires(TTriviallyDestructibleType<T>)
		{
		}

		template <typename T>
		TK_FORCE_INLINE static void destructItems(T *dst, i32 count)
			requires(!TTriviallyDestructibleType<T>)
		{
			if (dst == nullptr || count <= 0)
				return;
			while (count--)
			{
				dst->~T();
				++dst;
			}
		}

		template <typename T>
		TK_FORCE_INLINE static void destructItems(T *, i32)
			requires(TTriviallyDestructibleType<T>)
		{
		}

		template <typename T>
		TK_FORCE_INLINE static void copyItems(T *dst, const T *src, i32 count)
			requires(!TTriviallyCopyAssignableType<T>)
		{
			while (count--)
			{
				*dst = *src;
				++dst;
				++src;
			}
		}

		template <typename T>
		TK_FORCE_INLINE static void copyItems(T *dst, const T *src, i32 count)
			requires(TTriviallyCopyAssignableType<T>)
		{
			memoryCopy(dst, src, count * sizeof(T));
		}

		template <typename T, typename U>
		TK_FORCE_INLINE static void moveItems(T *dst, const U *src, i32 count)
			requires(!TBitwiseConstructibleFrom<T, U>)
		{
			while (count--)
			{
				new (dst) T((T &&)*src);
				++(T *&)dst;
				++src;
			}
		}

		template <typename T, typename U>
		TK_FORCE_INLINE static void moveItems(T *dst, const U *src, i32 count)
			requires(TBitwiseConstructibleFrom<T, U>)
		{
			memoryCopy((void *)dst, (void *)src, count * sizeof(U));
		}

	public:
		template <typename T>
		TK_FORCE_INLINE static void memoryCopy(T *dest, const T *src, psize size)
		{
			memcpy(static_cast<void *>(dest), static_cast<const void *>(src), size);
		}
		TK_FORCE_INLINE static void memoryCopy(void *dest, const void *src, psize size)
		{
			memcpy(dest, src, size);
		}

		TK_FORCE_INLINE static void memorySet(void *dest, u64 size, i32 value)
		{
			memset(dest, value, static_cast<size_t>(size));
		}

		TK_FORCE_INLINE static void memoryClear(void *dest, u64 size)
		{
			memset(dest, 0, static_cast<size_t>(size));
		}

		TK_FORCE_INLINE static i32 memoryCompare(const void *ptr1, const void *ptr2, u64 size)
		{
			return memcmp(ptr1, ptr2, static_cast<size_t>(size));
		}

	public:
		static bool isValid(IAllocator &);
		static IAllocator &getDefault();
		static bool validateAllocation(void *ptr);
		static bool validateAllocations();

	public:
		TK_FORCE_INLINE static void *allocate(IAllocator &alloc, psize size, psize align = 16)
		{
#if TK_DEBUG
			return allocateDebug(alloc, size, align);
#else
			return alloc.alloc(alloc.ctx, size, align);
#endif
		}

		TK_FORCE_INLINE static void *reallocate(IAllocator &alloc, void *ptr, psize size, psize oldSize, psize align = 16)
		{
#if TK_DEBUG
			return reallocateDebug(alloc, ptr, size, oldSize, align);
#else
			return alloc.realloc(alloc.ctx, ptr, size, oldSize, align);
#endif
		}

		TK_FORCE_INLINE static void free(IAllocator &alloc, void *ptr)
		{
			if (!ptr)
				return;
#if TK_DEBUG
			return freeDebug(alloc, ptr);
#else
			return alloc.free(alloc.ctx, ptr);
#endif
		}

#if TK_DEBUG
	private:
		static void *allocateDebug(IAllocator &alloc, psize size, psize align);
		static void *reallocateDebug(IAllocator &alloc, void *ptr, psize size, psize oldSize, psize align);
		static void freeDebug(IAllocator &alloc, void *ptr);
#endif

	public:
		template <class T>
		inline static T *newObject()
		{
			T *ptr = (T *)allocate(getDefault(), sizeof(T), alignof(T));
			constructItem(ptr);
			return ptr;
		}

		template <class T>
		inline static T *newObject(IAllocator &alloc)
		{
			T *ptr = (T *)allocate(alloc, sizeof(T), alignof(T));
			constructItem(ptr);
			return ptr;
		}

		template <class T, class... Args>
		inline static T *newObject(Args &&...args)
		{
			T *ptr = (T *)allocate(getDefault(), sizeof(T), alignof(T));
			new (ptr) T(toolkit::forward<Args>(args)...);
			return ptr;
		}

		template <class T, class... Args>
		inline static T *newObject(IAllocator &alloc, Args &&...args)
		{
			T *ptr = (T *)allocate(alloc, sizeof(T), alignof(T));
			new (ptr) T(toolkit::forward<Args>(args)...);
			return ptr;
		}

		template <class T>
		inline static T *newArray(u32 count)
		{
			T *ptr = (T *)allocate(getDefault(), sizeof(T) * count, alignof(T));
			constructItems(ptr, count);
			return ptr;
		}

		template <class T>
		inline static T *newArray(IAllocator &alloc, u32 count)
		{
			T *ptr = (T *)allocate(alloc, sizeof(T) * count, alignof(T));
			constructItems(ptr, count);
			return ptr;
		}

		template <class T>
		inline static void deletePtr(T *ptr)
		{
			if (!ptr)
				return;
			destructItem(ptr);
			free(getDefault(), ptr);
		}

		template <class T>
		inline static void deletePtr(IAllocator &alloc, T *ptr)
		{
			if (!ptr)
				return;
			destructItem(ptr);
			free(alloc, ptr);
		}

		template <class T>
		inline static void deleteArray(T *ptr, u32 count)
		{
			if (!ptr)
				return;
			destructItems(ptr, count);
			free(getDefault(), ptr);
		}

		template <class T>
		inline static void deleteArray(IAllocator &alloc, T *ptr, u32 count)
		{
			if (!ptr)
				return;
			destructItems(ptr, count);
			free(alloc, ptr);
		}
	};

	template <class T>
	struct TkAllocatorDeleter
	{
		IAllocator *allocator = nullptr;

		constexpr TkAllocatorDeleter() noexcept = default;

		constexpr explicit TkAllocatorDeleter(IAllocator &alloc) noexcept
			: allocator(&alloc)
		{
		}

		void operator()(T *ptr) const noexcept
		{
			if (!ptr)
				return;

			TkMemory::deletePtr(getAllocator(), ptr);
		}

	private:
		IAllocator &getAllocator() const noexcept
		{
			return allocator ? *allocator : TkMemory::getDefault();
		}
	};

	template <class T>
	class TkStlAllocator
	{
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		using propagate_on_container_move_assignment = std::true_type;
		using is_always_equal = std::false_type;

		template <class U>
		friend class TkStlAllocator;

	public:
		constexpr TkStlAllocator() noexcept = default;

		constexpr explicit TkStlAllocator(IAllocator &alloc) noexcept
			: m_allocator(&alloc)
		{
		}

		template <class U>
		constexpr TkStlAllocator(const TkStlAllocator<U> &other) noexcept
			: m_allocator(other.m_allocator)
		{
		}

		[[nodiscard]] T *allocate(std::size_t count)
		{
			if (count > max_size())
				throw std::bad_array_new_length();

			void *memory = TkMemory::allocate(getAllocator(), static_cast<psize>(sizeof(T) * count), alignof(T));
			if (!memory)
				throw std::bad_alloc();

			return static_cast<T *>(memory);
		}

		void deallocate(T *ptr, std::size_t) noexcept
		{
			if (!ptr)
				return;

			TkMemory::free(getAllocator(), ptr);
		}

		constexpr std::size_t max_size() const noexcept
		{
			return std::numeric_limits<std::size_t>::max() / sizeof(T);
		}

		constexpr IAllocator *allocator() const noexcept
		{
			return m_allocator;
		}

	private:
		IAllocator &getAllocator() const noexcept
		{
			return m_allocator ? *m_allocator : TkMemory::getDefault();
		}

	private:
		IAllocator *m_allocator = nullptr;
	};

	template <class T, class U>
	constexpr bool operator==(const TkStlAllocator<T> &lhs, const TkStlAllocator<U> &rhs) noexcept
	{
		return lhs.allocator() == rhs.allocator();
	}

	template <class T, class U>
	constexpr bool operator!=(const TkStlAllocator<T> &lhs, const TkStlAllocator<U> &rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template <class T>
	using TkUniquePtr = std::unique_ptr<T, TkAllocatorDeleter<T>>;

	template <class T>
	using TkSharedPtr = std::shared_ptr<T>;

	template <class T>
	using TkWeakPtr = std::weak_ptr<T>;

	template <class T>
	using UniquePtr = TkUniquePtr<T>;

	template <class T>
	using SharedPtr = TkSharedPtr<T>;

	template <class T>
	using WeakPtr = TkWeakPtr<T>;

	template <class T, class... Args>
	TkUniquePtr<T> makeUnique(IAllocator &alloc, Args &&...args)
	{
		static_assert(!std::is_array_v<T>, "toolkit::makeUnique<T>() does not support array types.");

		void *memory = TkMemory::allocate(alloc, sizeof(T), alignof(T));
		if (!memory)
			throw std::bad_alloc();

		T *object = nullptr;
		try
		{
			object = new (memory) T(std::forward<Args>(args)...);
		}
		catch (...)
		{
			TkMemory::free(alloc, memory);
			throw;
		}
		return TkUniquePtr<T>(object, TkAllocatorDeleter<T>(alloc));
	}

	template <class T, class... Args>
	TkUniquePtr<T> makeUnique(Args &&...args)
	{
		return makeUnique<T>(TkMemory::getDefault(), std::forward<Args>(args)...);
	}

	template <class T, class... Args>
	TkSharedPtr<T> makeShared(IAllocator &alloc, Args &&...args)
	{
		static_assert(!std::is_array_v<T>, "toolkit::makeShared<T>() does not support array types.");

		return std::allocate_shared<T>(TkStlAllocator<T>(alloc), std::forward<Args>(args)...);
	}

	template <class T, class... Args>
	TkSharedPtr<T> makeShared(Args &&...args)
	{
		return makeShared<T>(TkMemory::getDefault(), std::forward<Args>(args)...);
	}

	template <class T>
	TkWeakPtr<T> makeWeak(const TkSharedPtr<T> &ptr) noexcept
	{
		return TkWeakPtr<T>(ptr);
	}

	template <class T>
	TkSharedPtr<T> lockWeak(const TkWeakPtr<T> &ptr) noexcept
	{
		return ptr.lock();
	}

	template <class T>
	bool expired(const TkWeakPtr<T> &ptr) noexcept
	{
		return ptr.expired();
	}
}
