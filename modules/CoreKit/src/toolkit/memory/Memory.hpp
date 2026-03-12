// Created by cubevlmu on 2025/11/30.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <string>

namespace toolkit
{

	struct AllocatorBase
	{
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
	};

	struct AllocatorMeta
	{
#if TK_DEBUG
		const std::string_view tag = nullptr;
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
			requires(!TIsBitwiseConstructible<T, U>::Value)
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
			requires(TIsBitwiseConstructible<T, U>::Value)
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
			requires(!TIsBitwiseConstructible<T, U>::Value)
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
			requires(TIsBitwiseConstructible<T, U>::Value)
		{
			memoryCopy((void *)dst, (void *)src, count * sizeof(U));
		}

	public:
		template <typename T>
		TK_FORCE_INLINE static void memoryCopy(T *dest, const T *src, psize size)
		{
			memcpy(dest, src, size);
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
		bool isValid(IAllocator &);
		static IAllocator &getDefault();

	public:
		TK_FORCE_INLINE static void *allocate(IAllocator &alloc, psize size, psize align = 16)
		{
			return alloc.alloc(alloc.ctx, size, align);
		}

		TK_FORCE_INLINE static void *reallocate(IAllocator &alloc, void *ptr, psize size, psize oldSize, psize align = 16)
		{
			return alloc.realloc(alloc.ctx, ptr, size, oldSize, align);
		}

		TK_FORCE_INLINE static void free(IAllocator &alloc, void *ptr)
		{
			return alloc.free(alloc.ctx, ptr);
		}

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
		inline T *newObject(Args &&...args)
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
			destructItem(ptr);
			free(getDefault(), ptr);
		}

		template <class T>
		inline static void deletePtr(IAllocator &alloc, T *ptr)
		{
			destructItem(ptr);
			free(alloc, ptr);
		}

		template <class T>
		inline static void deleteArray(T *ptr, u32 count)
		{
			destructItems(ptr, count);
			free(getDefault(), ptr);
		}

		template <class T>
		inline static void deleteArray(IAllocator &alloc, T *ptr, u32 count)
		{
			destructItems(ptr, count);
			free(alloc, ptr);
		}
	};

}
