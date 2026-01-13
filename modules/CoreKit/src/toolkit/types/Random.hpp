/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Math.hpp>

#include <random>
#include <stdlib.h>

namespace toolkit
{

    class TkRandom final
    {
    public:
        // Random float number by c function rand()
        static inline float randNext()
        {
            return (float)::rand() / (float)RAND_MAX;
        }

        // Random float number by c function rand()
        static inline float randRangeNext(float min, float max)
        {
            return min + (max - min) * randNext();
        }

    public:
        TkRandom() = default;
        TkRandom(i32 seed)
            : m_initialSeed {seed}
            , m_seed {seed}
        {
        }

    public:
        // Gets initial seed value
        i32 getInitialSeed() const
        {
            return m_initialSeed;
        }

        // Gets the current seed.
        i32 getCurrentSeed() const
        {
            return m_seed;
        }

        // Initializes this random stream with the specified seed value.
        void init(i32 seed)
        {
            m_initialSeed = seed;
            m_seed = seed;
        }

        // Resets this random stream to the initial seed value.
        void reset() const
        {
            m_seed = m_initialSeed;
        }

        // Generates a new random seed.
        void generateNewSeed()
        {
            init(rand());
        }

        // Generates a new random seed with real random support.
        void generateNewSeedReal()
        {
            std::random_device dev{};
            std::mt19937 mt{dev};
            float rs = mt();
            init(rs);
        }

    public:
        // Returns a random boolean.
        bool getBool() const
        {
            mutateSeed();
            return *(u32 *)&m_seed < (kMaxU32 / 2);
        }

        // Returns a random number between 0 and MAX_u32.
        u32 getUnsignedInt() const
        {
            mutateSeed();
            return *(u32 *)&m_seed;
        }

        // Returns a random number between 0 and 1.
        float getFraction() const
        {
            mutateSeed();
            const float temp = 1.0f;
            float result;
            *(i32 *)&result = (*(i32 *)&temp & 0xff800000) | (m_seed & 0x007fffff);
            return TkMath::fractional(result);
        }

        // Returns a random number between 0 and 1.
        TK_FORCE_INLINE float rand() const
        {
            return getFraction();
        }

        // Helper function for rand implementations.
        TK_FORCE_INLINE i32 randHelper(i32 a) const
        {
            return a > 0 ? TkMath::truncToInt(getFraction() * ((float)a - TkMath::ZeroTolerance)) : 0;
        }

        // Helper function for rand implementations
        TK_FORCE_INLINE i32 randRange(i32 min, i32 max) const
        {
            const i32 range = max - min + 1;
            return min + randHelper(range);
        }

        // Helper function for rand implementations
        TK_FORCE_INLINE float randRange(float min, float max) const
        {
            return min + (max - min) * rand();
        }

    protected:
        // Mutates the current seed into the next seed.
        void mutateSeed() const
        {
            // This can be modified to provide better randomization
            m_seed = m_seed * 196314165 + 907633515;
        }
    private:

        // Holds the initial seed.
        i32 m_initialSeed = 0;
        // Holds the current seed.
        mutable i32 m_seed = 0;
    };

}