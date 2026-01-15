/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <cstdint>

namespace toolkit {

	template<typename T>
	class TkArray;
	template <typename T>
	class TkSpan;
	template<typename T>
	class TkHashSet;
	template <typename KeyType, typename ValueType>
    class TkHashMap;
	
    template<typename T, typename U>
    class TkPair;

	template<typename... Params>
	class TkAction;
	template<typename ReturnType, typename... Params>
	class TkFunc;

	class TkString;
	class TkStringView;

	class TkThread;
	class TkVersion;
	class TkUuid;

	// Forward
	template <typename... Params>
	uint32_t getHash(const TkFunc<void(Params...)> &);
    template<class T, class U>
    inline uint32_t getHash(const TkPair<T, U>& key);
}