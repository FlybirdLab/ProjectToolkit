/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {
    
    // The objects layers selection mask (from layers and tags settings). Uses 1 bit per layer (up to 32 layers).
    class TkLayersMask
    {
    public:
        TK_FORCE_INLINE TkLayersMask() = default;
        TK_FORCE_INLINE TkLayersMask(u32 mask)
            : m_mask {mask}
        {
        }

        TK_FORCE_INLINE bool hasLayer(i32 layerIndex) const
        {
            return (m_mask & (1 << layerIndex)) != 0;
        }
        bool hasLayer(const TkStringView& layerName) const; //TODO

        operator u32() const
        {
            return m_mask;
        }
        bool operator==(const TkLayersMask& other) const
        {
            return m_mask == other.m_mask;
        }
        bool operator!=(const TkLayersMask& other) const
        {
            return m_mask != other.m_mask;
        }
        TkLayersMask operator+(const TkLayersMask& other) const
        {
            return m_mask | other.m_mask;
        }
        TkLayersMask operator-(const TkLayersMask& other) const
        {
            return m_mask & ~other.m_mask;
        }
        TkLayersMask operator&(const TkLayersMask& other) const
        {
            return m_mask & other.m_mask;
        }
        TkLayersMask operator|(const TkLayersMask& other) const
        {
            return m_mask | other.m_mask;
        }
        TkLayersMask operator^(const TkLayersMask& other) const
        {
            return m_mask ^ other.m_mask;
        }
        TkLayersMask operator-() const
        {
            return ~m_mask;
        }

    public:
        u32 m_mask = kMaxU32;
    };
}