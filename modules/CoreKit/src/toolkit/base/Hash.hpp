/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target: HashModelTest.cpp
 */

#pragma once

#include <toolkit/base/TypeTraits.hpp>

namespace toolkit
{
	
	// use if you want fast hash with low probability of collisions and size (8 bytes) is not an issue
	// can change in future, do not serialize
	struct RuntimeHash
	{
		static RuntimeHash fromU64(u64 hash);
		RuntimeHash() {}
		explicit RuntimeHash(const char *string);
		RuntimeHash(const void *data, u32 len);

		bool operator!=(RuntimeHash rhs) const { return hash != rhs.hash; }
		bool operator==(RuntimeHash rhs) const { return hash == rhs.hash; }

		u64 getHashValue() const { return hash; }

	private:
		u64 hash = 0;
#ifdef TK_DEBUG
		const char *debug_str = nullptr;
#endif
	};

	// same as RuntimeHash, but only 32 bits
	struct RuntimeHash32
	{
		static RuntimeHash32 fromU32(u32 hash);
		RuntimeHash32() {}
		explicit RuntimeHash32(const char *string);
		RuntimeHash32(const void *data, u32 len);

		bool operator!=(RuntimeHash32 rhs) const { return hash != rhs.hash; }
		bool operator==(RuntimeHash32 rhs) const { return hash == rhs.hash; }

		u32 getHashValue() const { return hash; }

	private:
		u32 hash = 0;
	};

	// use if you want to serialize it
	// 64bits
	struct StableHash
	{
		static StableHash fromU64(u64 hash);
		StableHash() {}
		explicit StableHash(const char *str);
		StableHash(const void *data, u32 len);

		bool operator!=(StableHash rhs) const { return hash != rhs.hash; }
		bool operator==(StableHash rhs) const { return hash == rhs.hash; }
		bool operator<(StableHash rhs) const { return hash < rhs.hash; }

		u64 getHashValue() const { return hash; }

	private:
		u64 hash = 0;
	};

	// same as StableHash, but only 32bits
	struct StableHash32
	{
		static StableHash32 fromU32(u32 hash);
		StableHash32() {}
		explicit StableHash32(const char *string);
		StableHash32(const void *data, u32 len);

		bool operator!=(StableHash32 rhs) const { return hash != rhs.hash; }
		bool operator==(StableHash32 rhs) const { return hash == rhs.hash; }
		bool operator<(StableHash32 rhs) const { return hash < rhs.hash; }

		u32 getHashValue() const { return hash; }

	private:
		u32 hash = 0;
	};

	using FilePathHash = StableHash;
	using BoneNameHash = StableHash;

	// begin/end pair cannot cross thread boundaries
	struct RollingStableHasher
	{
		void begin();
		void update(const void *data, u32 len);
		StableHash32 end();
		StableHash end64();
	};

	// begin/end pair cannot cross thread boundaries
	struct RollingHasher
	{
		void begin();
		void update(const void *data, u32 len);
		RuntimeHash32 end();
	};

	template <typename T, typename Enable = void>
	struct HashFunc;

	template <typename T, typename Enable>
	struct HashFunc
	{
		static u32 get(const T &)
		{
			static_assert(sizeof(T) == 0, "HashFunc<T> is not specialized");
			return 0;
		}
	};

	template <typename T>
	struct HashFunc<T, decltype((void)getHash(*(const T *)nullptr))>
	{
		static u32 get(const T &k)
		{
			return getHash(k);
		}
	};

	template <>
	struct HashFunc<RuntimeHash>
	{
		static u32 get(const RuntimeHash &k)
		{
			const u64 hash = k.getHashValue();
			return u32(hash ^ (hash >> 16));
		}
	};

	template <>
	struct HashFunc<StableHash>
	{
		static u32 get(const StableHash &k)
		{
			const u64 hash = k.getHashValue();
			return u32(hash ^ (hash >> 32));
		}
	};

	template <>
	struct HashFunc<StableHash32>
	{
		static u32 get(const StableHash32 &k)
		{
			return k.getHashValue();
		}
	};

	template <>
	struct HashFunc<RuntimeHash32>
	{
		static u32 get(const RuntimeHash32 &k)
		{
			return k.getHashValue();
		}
	};

#define HASH_FUNC_FOR_INTERGER(TYPE)  \
	template <>                       \
	struct HashFunc<TYPE>             \
	{                                 \
		static u32 get(const TYPE &k) \
		{                             \
			return k;                 \
		}                             \
	};
	HASH_FUNC_FOR_INTERGER(u8);
	HASH_FUNC_FOR_INTERGER(i8);
	HASH_FUNC_FOR_INTERGER(u16);
	HASH_FUNC_FOR_INTERGER(i16);
	HASH_FUNC_FOR_INTERGER(i32);
	HASH_FUNC_FOR_INTERGER(u32);
	HASH_FUNC_FOR_INTERGER(char);
#undef HASH_FUNC_FOR_INTERGER

	template <>
	struct HashFunc<u64>
	{
		static u32 get(const u64 &k)
		{
			return (u32)k + ((u32)(k >> 32) * 23);
		}
	};

	template <>
	struct HashFunc<i64>
	{
		static u32 get(const i64 &k)
		{
			return (u32)k + ((u32)(k >> 32) * 23);
		}
	};

	template <>
	struct HashFunc<f32>
	{
		static u32 get(const f32 &k)
		{
			return *(u32 *)&k;
		}
	};

	template <>
	struct HashFunc<f64>
	{
		static u32 get(const f64 &k)
		{
			return HashFunc<u64>::get(*(u64 *)&k);
		}
	};

	template <>
	struct HashFunc<void *>
	{
		static u32 get(const void *k)
		{
			static const i64 shift = 3;
			return (u32)((i64)(k) >> shift);
		}
	};

	[[deprecated("HashFunc<T*> fallback hashes pointer addresses. Prefer a typed getHash(const T&) or HashFunc<T> specialization when object identity is not intended.")]]
	TK_FORCE_INLINE u32 hashPointerFallback(const void *k)
	{
		return HashFunc<void *>::get(k);
	}

	template <typename T>
	struct HashFunc<T *>
	{
		static u32 get(const T *k)
		{
			return hashPointerFallback(k);
		}
	};

	template <typename T>
	struct HashFunc<const T *>
	{
		static u32 get(const T *k)
		{
			return hashPointerFallback(k);
		}
	};

	template <TEnumType T>
	struct HashFunc<T, void>
	{
		static u32 get(T v)
		{
			using U = __underlying_type(T);
			return HashFunc<U>::get((U)v);
		}
	};

	inline void combineHash(u32 &hash, const u32 value)
	{
		// Reference: Boost lib
		hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}

	template <typename T>
	inline void combineHash(u32 &hash, const T *value)
	{
		combineHash(hash, HashFunc<void*>::get(value)); //FIX
	}
}
