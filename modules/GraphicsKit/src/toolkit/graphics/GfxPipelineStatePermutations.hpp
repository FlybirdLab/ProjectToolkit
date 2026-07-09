/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/graphics/GfxDevice.hpp>
#include <toolkit/graphics/GfxShader.hpp>
#include <toolkit/graphics/GfxPipelineState.hpp>

namespace toolkit
{

    template <int Size>
    class GfxPipelineStatePermutations
    {
    public:
        GfxPipelineState *states[Size];

    public:
        GfxPipelineStatePermutations()
        {
            TkMemory::memoryClear(states, sizeof(states));
        }

        ~GfxPipelineStatePermutations()
        {
            dispose();
        }

    public:
        bool isValid() const
        {
            for (int i = 0; i < Size; i++)
            {
                if (states[i] == nullptr || !states[i]->isValid())
                {
                    return false;
                }
            }

            return true;
        }

        TK_FORCE_INLINE GfxPipelineState *get(int index) const
        {
            TK_ASSERT(index >= 0 && index < Size);
            return states[index];
        }

        TK_FORCE_INLINE GfxPipelineState *&operator[](i32 index)
        {
            TK_ASSERT(index >= 0 && index < Size);
            return states[index];
        }

    public:
        void createPipelineStates()
        {
            for (int i = 0; i < Size; i++)
            {
                if (states[i] == nullptr)
                {
                    states[i] = GfxDevice::sInstance->createPipelineState();
                }
            }
        }

        void release()
        {
            for (int i = 0; i < Size; i++)
            {
                if (states[i])
                {
                    states[i]->ReleaseGPU();
                }
            }
        }

        void dispose()
        {
            for (int i = 0; i < Size; i++)
            {
                GFX_DEL_RESOURCE(states[i]);
            }
        }
    };

    template <int Size>
    class GfxPipelineStatePermutationsPs : public GfxPipelineStatePermutations<Size>
    {
    public:
        typedef GfxPipelineStatePermutations<Size> Base;

    public:
        GfxPipelineStatePermutationsPs() = default;
        ~GfxPipelineStatePermutationsPs() = default;

    public:
        bool create(GfxPipelineState::Description &desc, GfxShader *shader, const TkStringView &psName)
        {
            for (int i = 0; i < Size; i++)
            {
                TK_ASSERT(Base::States[i]);

                desc.ps = shader->getPS(psName, i);
                if (Base::States[i]->Init(desc))
                    return true;
            }

            return false;
        }
    };

    template <int Size>
    class ComputeShaderPermutation
    {
    public:
        GfxShaderProgramCS *shaders[Size];

    public:
        ComputeShaderPermutation()
        {
            TkMemory::memoryClear(shaders, sizeof(shaders));
        }
        ~ComputeShaderPermutation() = default;

    public:
        TK_FORCE_INLINE GfxShaderProgramCS *get(const int index) const
        {
            TK_ASSERT(index >= 0 && index < Size);
            return shaders[index];
        }

    public:
        void clear()
        {
            TkMemory::memoryClear(shaders, sizeof(shaders));
        }

        void get(GfxShader *shader, const TkStringView &name)
        {
            for (int i = 0; i < Size; i++)
            {
                shaders[i] = shader->getCS(name, i);
            }
        }
    };

}