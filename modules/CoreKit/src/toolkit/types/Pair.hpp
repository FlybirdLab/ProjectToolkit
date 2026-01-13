/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/base/Hash.hpp>

namespace toolkit {
    
    template<typename T, typename U>
    class TkPair
    {
    public:
        TkPair() = default;
        TkPair(const T& key, const U& value)
            : first(key)
            , second(value) { }
        TkPair(const TkPair& other)
            : first(other.first)
            , second(other.second) { }
        TkPair(TkPair&& other) noexcept
            : first(moveTemp(other.first))
            , second(moveTemp(other.second)) { }

    public:
        TkPair& operator=(const TkPair& other)
        {
            if (this == &other)
                return *this;
            first = other.first;
            second = other.second;
            return *this;
        }


        TkPair& operator=(TkPair&& other) noexcept
        {
            if (this != &other)
            {
                first = MoveTemp(other.first);
                second = MoveTemp(other.second);
            }
            return *this;
        }
        bool operator==(const TkPair& other) const
        {
            return first == other.first && second == other.second;
        }
        bool operator!=(const TkPair& other) const
        {
            return first != other.first || second != other.second;
        }
        
    public:
        T first;
        U second;
    };


    template<class T, class U>
    inline u32 getHash(const TkPair<T, U>& key)
    {
        u32 hash = getHash(key.first);
        combineHash(hash, getHash(key.second));
        return hash;
    }


    template<class T, class U>
    inline constexpr TkPair<T, U> toPair(const T& key, const U& value)
    {
        return TkPair<T, U>(key, value);
    }
}