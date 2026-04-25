/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

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
	class ISerializeModifier;
	class ISerializable;

	class TkThread;
	class TkVersion;
	class TkUuid;
	class TkDateTime;
	class TkTimeSpan;
	class TkStringBuilder;

	template<typename T>
    class TkDataCollection;

	// Forward
	// template <typename... Params>
	// uint32_t getHash_(const TkFunc<void(Params...)> &);
    // template<class T, class U>
    // inline uint32_t getHash_(const TkPair<T, U>& key);
}
