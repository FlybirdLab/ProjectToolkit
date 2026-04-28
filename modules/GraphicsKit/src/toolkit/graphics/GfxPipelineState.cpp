/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxPipelineState.hpp"

#include "GfxDevice.hpp"
#include "common.hpp"

namespace toolkit
{

    GfxPipelineState *GfxPipelineState::newPipeline()
    {
        return GfxDevice::sInstance->createPipelineState();
    }

    GfxPipelineState::GfxPipelineState()
        : GfxResource{}
    {
    }

    bool GfxPipelineState::init(const Description &desc)
    {
        // Cache description in debug builds
#if TK_DEBUG
        debugDesc = desc;
#endif

        // Cache shader stages usage flags for pipeline state
        m_meta.instructionsCount = 0;
        m_meta.usedCBsMask = 0;
        m_meta.usedSRsMask = 0;
        m_meta.usedUAsMask = 0;
#define CHECK_STAGE(stage)                                           \
    if (desc.stage)                                                  \
    {                                                                \
        m_meta.usedCBsMask |= desc.stage->getBindings().usedCBsMask; \
        m_meta.usedSRsMask |= desc.stage->getBindings().usedSRsMask; \
        m_meta.usedUAsMask |= desc.stage->getBindings().usedUAsMask; \
    }
        CHECK_STAGE(vs);
        CHECK_STAGE(hs);
        CHECK_STAGE(ds);
        CHECK_STAGE(gs);
        CHECK_STAGE(ps);
#undef CHECK_STAGE

        return true;
    }

    GfxResType GfxPipelineState::getType() const
    {
        return GfxResType::kPipelineState;
    }

    GfxPipelineState::Description GfxPipelineState::Description::Default =
        {
            true,                                // DepthEnable
            true,                                // DepthWriteEnable
            true,                                // DepthClipEnable
            GfxComparisonFunc::kLess,            // DepthFunc
            false,                               // StencilEnable
            0xff,                                // StencilReadMask
            0xff,                                // StencilWriteMask
            GfxComparisonFunc::kAlways,          // StencilFunc
            StencilOperation::kKeep,             // StencilFailOp
            StencilOperation::kKeep,             // StencilDepthFailOp
            StencilOperation::kKeep,             // StencilPassOp
            nullptr,                             // VS
            nullptr,                             // HS
            nullptr,                             // DS
            nullptr,                             // GS
            nullptr,                             // PS
            GfxPrimitiveTopologyType::kTriangle, // PrimitiveTopology
            false,                               // Wireframe
            GfxCullMode::kNormal,                // CullMode
            GfxBlendingMode::sOpaque,            // BlendMode
    };

    GfxPipelineState::Description GfxPipelineState::Description::DefaultNoDepth =
        {
            false,                               // DepthEnable
            false,                               // DepthWriteEnable
            false,                               // DepthClipEnable
            GfxComparisonFunc::kLess,            // DepthFunc
            false,                               // StencilEnable
            0xff,                                // StencilReadMask
            0xff,                                // StencilWriteMask
            GfxComparisonFunc::kAlways,          // StencilFunc
            StencilOperation::kKeep,             // StencilFailOp
            StencilOperation::kKeep,             // StencilDepthFailOp
            StencilOperation::kKeep,             // StencilPassOp
            nullptr,                             // VS
            nullptr,                             // HS
            nullptr,                             // DS
            nullptr,                             // GS
            nullptr,                             // PS
            GfxPrimitiveTopologyType::kTriangle, // PrimitiveTopology
            false,                               // Wireframe
            GfxCullMode::kNormal,                // CullMode
            GfxBlendingMode::sOpaque,            // BlendMode
    };

    GfxPipelineState::Description GfxPipelineState::Description::DefaultFullscreenTriangle =
        {
            false,                               // DepthEnable
            false,                               // DepthWriteEnable
            false,                               // DepthClipEnable
            GfxComparisonFunc::kLess,            // DepthFunc
            false,                               // StencilEnable
            0xff,                                // StencilReadMask
            0xff,                                // StencilWriteMask
            GfxComparisonFunc::kAlways,          // StencilFunc
            StencilOperation::kKeep,             // StencilFailOp
            StencilOperation::kKeep,             // StencilDepthFailOp
            StencilOperation::kKeep,             // StencilPassOp
            nullptr,                             // VS (Set to default quad VS via GfxDevice)
            nullptr,                             // HS
            nullptr,                             // DS
            nullptr,                             // GS
            nullptr,                             // PS
            GfxPrimitiveTopologyType::kTriangle, // PrimitiveTopology
            false,                               // Wireframe
            GfxCullMode::kTwoSided,              // CullMode
            GfxBlendingMode::sOpaque,            // BlendMode
    };
}