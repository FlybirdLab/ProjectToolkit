/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxDevice.hpp>

#if GFX_API_NULL

namespace toolkit
{

    // Implementation of Graphics Device for Null backend.
    class NullDevice : public GfxDevice
    {
        friend class NullContext;
        friend class NullSwapChain;

    private:
        NullContext *m_mainContext;
        class NullAdapter *m_adapter;

    public:
        static GfxDevice *create();
        NullDevice();
        ~NullDevice();

    public:
        GfxContext *getMainContext() override;
        GfxAdapter *getAdapter() const override;
        void *getNativePtr() const override;
        bool init() override;
        // bool loadContent() override;
        void draw() override;
        void dispose() override;
        void waitForGPU() override;
        GfxTexture *createTexture(const TkStringView &name) override;
        GfxShader *createShader(const TkStringView &name) override;
        GfxPipelineState *createPipelineState() override;
        GfxTimerQuery *createTimerQuery() override;
        GfxBuffer *createBuffer(const TkStringView &name) override;
        GfxSampler *createSampler() override;
        GfxVertexLayout *createVertexLayout(const VertexElements &elements, bool explicitOffsets) override;
        GfxSwapChain *createSwapChain(GfxWindow *window) override;
        GfxConstantBuffer *createConstantBuffer(u32 size, const TkStringView &name) override;
    };

    extern GfxDevice *CreateNullDevice();

}

#endif