/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

namespace toolkit {
    
    // Easy bit mask manipulation.
    template<typename T>
    class TkBitMask
    {
    public:
        TkBitMask()
            : m_bitmask {0}
        {
        }
        TkBitMask(T mask)
            : m_bitmask {mask}
        {
        }

        TkBitMask(const TkBitMask& b) = default;
        TkBitMask& operator=(const TkBitMask& b) = default;

    public:
        // Set or unset a bit at the given position.
        void set(T mask, bool setBits = true)
        {
            m_bitmask = (setBits) ? (m_bitmask | mask) : (m_bitmask & ~mask);
        }

        // Unset a bit (set to zero) at the given position.
        void unset(T mask)
        {
            m_bitmask &= ~mask;
        }

        // Flip the bits at the given position. It will go from 1 to 0 or from 0 to 1.
        void flip(T mask)
        {
            m_bitmask ^= mask;
        }

        // Return true if the bit is set or false if it's not.
        bool get(T mask) const
        {
            return (m_bitmask & mask) == mask;
        }

        // Given a mask check if any are enabled.
        bool getAny(T mask) const
        {
            return (m_bitmask & mask) != 0;
        }

    protected:
        T m_bitmask;
    };
    

}