// Created by cubevlmu on 2025/11/30.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Memory.hpp"
#include "StdAllocator.hpp"

#if TK_DEBUG
#include <atomic>
#endif

extern "C" {

void* toolkit_system_malloc(size_t size) {
	return toolkit::TkMemory::allocate(toolkit::TkMemory::getDefault(), size);
}

void* toolkit_system_realloc(void* ptr, size_t size) {
	return toolkit::TkMemory::reallocate(toolkit::TkMemory::getDefault(), ptr, size, -1); //TODO fix bug
}

void toolkit_system_free(void* ptr) {
	return toolkit::TkMemory::free(toolkit::TkMemory::getDefault(), ptr);
}

}

namespace toolkit {

#if TK_DEBUG
	namespace
	{
		struct AllocationDebugHeader
		{
			u64 magic = 0;
			u64 serial = 0;
			AllocatorBase *ctx = nullptr;
			u64 allocatorGeneration = 0;
			FreeFn freeFn = nullptr;
			void *raw = nullptr;
			psize requestedSize = 0;
			psize allocatedSize = 0;
			psize alignment = 0;
		};

		struct AllocatorDebugRecord
		{
			AllocatorBase *ctx = nullptr;
			u64 generation = 0;
			bool alive = false;
		};

		struct AllocationDebugRecord
		{
			void *user = nullptr;
			AllocationDebugHeader *header = nullptr;
			u64 serial = 0;
			bool everUsed = false;
			bool alive = false;
		};

		constexpr i32 kMaxTrackedAllocators = 4096;
		constexpr i32 kMaxTrackedAllocations = 65536;
		constexpr u64 kAllocationDebugMagic = 0x544B4D454D484452ull; // TKMEMHDR
		constexpr u8 kAllocationDebugGuard = 0xFD;
		constexpr u8 kAllocationDebugFresh = 0xCD;
		constexpr u8 kAllocationDebugFreed = 0xDD;
		constexpr psize kAllocationDebugTailSize = 32;

		AllocatorDebugRecord *allocatorDebugRecords()
		{
			static AllocatorDebugRecord records[kMaxTrackedAllocators];
			return records;
		}

		AllocationDebugRecord *allocationDebugRecords()
		{
			static AllocationDebugRecord records[kMaxTrackedAllocations];
			return records;
		}

		std::atomic_flag &allocatorDebugLock()
		{
			static std::atomic_flag lock = ATOMIC_FLAG_INIT;
			return lock;
		}

		std::atomic_flag &allocationDebugLock()
		{
			static std::atomic_flag lock = ATOMIC_FLAG_INIT;
			return lock;
		}

		u64 &allocatorDebugNextGeneration()
		{
			static u64 generation = 1;
			return generation;
		}

		u64 &allocationDebugNextSerial()
		{
			static u64 serial = 1;
			return serial;
		}

		struct AllocatorDebugLockGuard
		{
			AllocatorDebugLockGuard()
			{
				while (allocatorDebugLock().test_and_set(std::memory_order_acquire))
				{
				}
			}

			~AllocatorDebugLockGuard()
			{
				allocatorDebugLock().clear(std::memory_order_release);
			}
		};

		struct AllocationDebugLockGuard
		{
			AllocationDebugLockGuard()
			{
				while (allocationDebugLock().test_and_set(std::memory_order_acquire))
				{
				}
			}

			~AllocationDebugLockGuard()
			{
				allocationDebugLock().clear(std::memory_order_release);
			}
		};

		bool isPowerOfTwo(psize value)
		{
			return value > 0 && (value & (value - 1)) == 0;
		}

		psize alignForward(psize value, psize alignment)
		{
			TK_ASSERT(isPowerOfTwo(alignment));
			return (value + alignment - 1) & ~(alignment - 1);
		}

		void fillBytes(void *ptr, psize size, u8 value)
		{
			if (ptr && size > 0)
				TkMemory::memorySet(ptr, size, value);
		}

		bool addWouldOverflow(psize a, psize b)
		{
			return a > kMaxPSize - b;
		}

		u64 registerAllocatorDebug(AllocatorBase *ctx)
		{
			TK_ASSERT(ctx != nullptr);

			AllocatorDebugLockGuard guard;
			AllocatorDebugRecord *records = allocatorDebugRecords();
			i32 freeIndex = -1;
			for (i32 i = 0; i < kMaxTrackedAllocators; ++i)
			{
				if (records[i].alive && records[i].ctx == ctx)
				{
					TK_ASSERT(false && "Allocator registered twice.");
					return records[i].generation;
				}
				if (freeIndex < 0 && !records[i].alive)
					freeIndex = i;
			}

			TK_ASSERT(freeIndex >= 0 && "Allocator debug registry capacity exhausted.");
			if (freeIndex < 0)
				return 0;

			const u64 generation = allocatorDebugNextGeneration()++;
			records[freeIndex].ctx = ctx;
			records[freeIndex].generation = generation;
			records[freeIndex].alive = true;
			return generation;
		}

		void unregisterAllocatorDebug(AllocatorBase *ctx)
		{
			TK_ASSERT(ctx != nullptr);

			AllocatorDebugLockGuard guard;
			AllocatorDebugRecord *records = allocatorDebugRecords();
			for (i32 i = 0; i < kMaxTrackedAllocators; ++i)
			{
				if (records[i].alive && records[i].ctx == ctx)
				{
					records[i].alive = false;
					records[i].ctx = nullptr;
					return;
				}
			}

			TK_ASSERT(false && "Allocator unregistered without a live registry record.");
		}

		u64 getAllocatorDebugGeneration(AllocatorBase *ctx)
		{
			if (!ctx)
				return 0;

			AllocatorDebugLockGuard guard;
			AllocatorDebugRecord *records = allocatorDebugRecords();
			for (i32 i = 0; i < kMaxTrackedAllocators; ++i)
			{
				if (records[i].alive && records[i].ctx == ctx)
					return records[i].generation;
			}
			return 0;
		}

		AllocationDebugRecord *findAllocationRecordLocked(void *user)
		{
			AllocationDebugRecord *records = allocationDebugRecords();
			for (i32 i = 0; i < kMaxTrackedAllocations; ++i)
			{
				if (records[i].everUsed && records[i].user == user)
					return &records[i];
			}
			return nullptr;
		}

		void registerAllocationDebug(void *user, AllocationDebugHeader *header)
		{
			TK_ASSERT(user != nullptr);
			TK_ASSERT(header != nullptr);

			AllocationDebugLockGuard guard;
			AllocationDebugRecord *records = allocationDebugRecords();
			AllocationDebugRecord *freeRecord = nullptr;

			for (i32 i = 0; i < kMaxTrackedAllocations; ++i)
			{
				if (records[i].everUsed && records[i].user == user)
				{
					if (records[i].alive)
					{
						TK_ASSERT(false && "Allocation debug registry saw the same live user pointer twice.");
						return;
					}
					freeRecord = &records[i];
					break;
				}

				if (!freeRecord && !records[i].alive)
					freeRecord = &records[i];
			}

			TK_ASSERT(freeRecord != nullptr && "Allocation debug registry capacity exhausted.");
			if (!freeRecord)
				return;

			const u64 serial = allocationDebugNextSerial()++;
			header->serial = serial;
			freeRecord->user = user;
			freeRecord->header = header;
			freeRecord->serial = serial;
			freeRecord->everUsed = true;
			freeRecord->alive = true;
		}

		bool tailGuardValid(const AllocationDebugHeader *header)
		{
			if (!header || !header->raw)
				return false;

			const psize userSize = header->requestedSize > 0 ? header->requestedSize : 1;
			const u8 *tail = reinterpret_cast<const u8 *>(header) + sizeof(AllocationDebugHeader) + userSize;
			for (psize i = 0; i < kAllocationDebugTailSize; ++i)
			{
				if (tail[i] != kAllocationDebugGuard)
					return false;
			}
			return true;
		}

		bool allocationHeaderValid(const AllocationDebugHeader *header)
		{
			return header && header->magic == kAllocationDebugMagic && header->raw && header->freeFn &&
				header->allocatedSize >= header->requestedSize + kAllocationDebugTailSize &&
				isPowerOfTwo(header->alignment);
		}

		bool allocationRecordValid(const AllocationDebugRecord &record)
		{
			if (!record.alive || !record.user || !record.header)
				return false;

			const AllocationDebugHeader *header = record.header;
			if (!allocationHeaderValid(header))
				return false;

			if (header->serial != record.serial)
				return false;

			const psize userAddress = reinterpret_cast<psize>(record.user);
			if ((userAddress & (header->alignment - 1)) != 0)
				return false;

			const psize rawAddress = reinterpret_cast<psize>(header->raw);
			if (userAddress < rawAddress + sizeof(AllocationDebugHeader))
				return false;

			const psize userSize = header->requestedSize > 0 ? header->requestedSize : 1;
			if (userAddress + userSize + kAllocationDebugTailSize > rawAddress + header->allocatedSize)
				return false;

			return tailGuardValid(header);
		}

		AllocationDebugRecord *findLiveAllocationDebug(void *user)
		{
			AllocationDebugRecord *record = findAllocationRecordLocked(user);
			if (!record)
			{
				TK_ASSERT(false && "Freeing a pointer that was not allocated by TkMemory.");
				return nullptr;
			}

			if (!record->alive)
			{
				TK_ASSERT(false && "Double free or use-after-free detected by TkMemory.");
				return nullptr;
			}

			if (!allocationRecordValid(*record))
			{
				TK_ASSERT(false && "Memory corruption detected in a TkMemory allocation.");
				return nullptr;
			}

			return record;
		}
	}

	AllocatorBase::AllocatorBase()
	{
		registerAllocatorDebug(this);
	}

	AllocatorBase::~AllocatorBase()
	{
		unregisterAllocatorDebug(this);
	}
#endif

	IAllocator::IAllocator(AllocFn af, ReallocFn rf, FreeFn ff, AllocatorBase* ctx, AllocatorType typeId)
	    : alloc {af}
		, realloc {rf}
		, free {ff}
		, ctx{ctx}
		, typeId {typeId}
	{
#if TK_DEBUG
		debugGeneration = getAllocatorDebugGeneration(ctx);
#endif
	}

	IAllocator::~IAllocator()
	{
		ctx = nullptr;
	}

	bool TkMemory::isValid(IAllocator& a)
	{
		if (a.ctx == nullptr || a.alloc == nullptr || a.realloc == nullptr || a.free == nullptr)
			return false;
#if TK_DEBUG
		const u64 currentGeneration = getAllocatorDebugGeneration(a.ctx);
		return currentGeneration != 0 && currentGeneration == a.debugGeneration;
#else
		return true;
#endif
	}

#if TK_DEBUG
	void *TkMemory::allocateDebug(IAllocator &alloc, psize size, psize align)
	{
		TK_ASSERT(isValid(alloc) && "Invalid or expired allocator used for allocation.");
		TK_ASSERT(isPowerOfTwo(align) && "Allocation alignment must be a power of two.");
		if (!isValid(alloc) || !isPowerOfTwo(align))
			return nullptr;

		const psize requestedSize = size;
		const psize userSize = size > 0 ? size : 1;
		const psize debugAlignment = align > alignof(AllocationDebugHeader) ? align : alignof(AllocationDebugHeader);
		if (addWouldOverflow(sizeof(AllocationDebugHeader), align - 1) ||
			addWouldOverflow(sizeof(AllocationDebugHeader) + align - 1, userSize) ||
			addWouldOverflow(sizeof(AllocationDebugHeader) + align - 1 + userSize, kAllocationDebugTailSize))
		{
			TK_ASSERT(false && "Allocation size overflow.");
			return nullptr;
		}

		const psize totalSize = sizeof(AllocationDebugHeader) + align - 1 + userSize + kAllocationDebugTailSize;
		void *raw = alloc.alloc(alloc.ctx, totalSize, debugAlignment);
		if (!raw)
			return nullptr;

		const psize rawAddress = reinterpret_cast<psize>(raw);
		const psize userAddress = alignForward(rawAddress + sizeof(AllocationDebugHeader), align);
		auto *user = reinterpret_cast<u8 *>(userAddress);
		auto *header = reinterpret_cast<AllocationDebugHeader *>(userAddress - sizeof(AllocationDebugHeader));

		fillBytes(raw, totalSize, kAllocationDebugGuard);
		new (header) AllocationDebugHeader();
		header->magic = kAllocationDebugMagic;
		header->ctx = alloc.ctx;
		header->allocatorGeneration = alloc.debugGeneration;
		header->freeFn = alloc.free;
		header->raw = raw;
		header->requestedSize = requestedSize;
		header->allocatedSize = totalSize;
		header->alignment = align;

		fillBytes(user, userSize, kAllocationDebugFresh);
		fillBytes(user + userSize, kAllocationDebugTailSize, kAllocationDebugGuard);
		registerAllocationDebug(user, header);
		return user;
	}

	void *TkMemory::reallocateDebug(IAllocator &alloc, void *ptr, psize size, psize oldSize, psize align)
	{
		(void)oldSize;

		if (!ptr)
			return allocateDebug(alloc, size, align);

		if (size == 0)
		{
			freeDebug(alloc, ptr);
			return nullptr;
		}

		AllocationDebugHeader *oldHeader = nullptr;
		{
			AllocationDebugLockGuard guard;
			AllocationDebugRecord *record = findLiveAllocationDebug(ptr);
			if (!record)
				return nullptr;
			oldHeader = record->header;
		}

		void *newPtr = allocateDebug(alloc, size, align);
		if (!newPtr)
			return nullptr;

		const psize bytesToCopy = oldHeader->requestedSize < size ? oldHeader->requestedSize : size;
		memoryCopy(newPtr, ptr, bytesToCopy);
		freeDebug(alloc, ptr);
		return newPtr;
	}

	void TkMemory::freeDebug(IAllocator &alloc, void *ptr)
	{
		TK_ASSERT(isValid(alloc) && "Invalid or expired allocator used for free.");
		if (!isValid(alloc))
			return;

		AllocationDebugHeader *header = nullptr;
		{
			AllocationDebugLockGuard guard;
			AllocationDebugRecord *record = findLiveAllocationDebug(ptr);
			if (!record)
				return;

			header = record->header;
			if (header->ctx != alloc.ctx || header->allocatorGeneration != alloc.debugGeneration)
				TK_ASSERT(false && "Allocation was freed with a different allocator than it was allocated with.");

			record->alive = false;
		}

		const psize userSize = header->requestedSize > 0 ? header->requestedSize : 1;
		fillBytes(ptr, userSize, kAllocationDebugFreed);
		header->magic = 0;
		header->freeFn(header->ctx, header->raw);
	}
#endif

	bool TkMemory::validateAllocation(void *ptr)
	{
#if TK_DEBUG
		if (!ptr)
			return true;

		AllocationDebugLockGuard guard;
		AllocationDebugRecord *record = findAllocationRecordLocked(ptr);
		return record && allocationRecordValid(*record);
#else
		(void)ptr;
		return true;
#endif
	}

	bool TkMemory::validateAllocations()
	{
#if TK_DEBUG
		AllocationDebugLockGuard guard;
		AllocationDebugRecord *records = allocationDebugRecords();
		for (i32 i = 0; i < kMaxTrackedAllocations; ++i)
		{
			if (records[i].alive && !allocationRecordValid(records[i]))
				return false;
		}
		return true;
#else
		return true;
#endif
	}

	IAllocator& TkMemory::getDefault()
	{
		static StdAllocator s_alloc {};
		static IAllocator alloc = s_alloc.asInterface();
		return alloc;
	}
}
