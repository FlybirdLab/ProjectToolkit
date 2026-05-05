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
    namespace
    {
        GfxBlendingMode makeBlendMode(
            GfxBlendingMode::Blend src,
            GfxBlendingMode::Blend dst,
            GfxBlendingMode::Blend srcAlpha,
            GfxBlendingMode::Blend dstAlpha)
        {
            GfxBlendingMode result;
            result.blendEnable = true;
            result.srcBlend = src;
            result.destBlend = dst;
            result.srcBlendAlpha = srcAlpha;
            result.destBlendAlpha = dstAlpha;
            return result;
        }
    }

    GfxBlendingMode GfxBlendingMode::sOpaque = {};
    GfxBlendingMode GfxBlendingMode::sAdditive = makeBlendMode(
        GfxBlendingMode::Blend::kOne,
        GfxBlendingMode::Blend::kOne,
        GfxBlendingMode::Blend::kOne,
        GfxBlendingMode::Blend::kOne);
    GfxBlendingMode GfxBlendingMode::sAlphaBlend = makeBlendMode(
        GfxBlendingMode::Blend::kSrcAlpha,
        GfxBlendingMode::Blend::kInvSrcAlpha,
        GfxBlendingMode::Blend::kOne,
        GfxBlendingMode::Blend::kInvSrcAlpha);
    GfxBlendingMode GfxBlendingMode::sAdd = GfxBlendingMode::sAdditive;
    GfxBlendingMode GfxBlendingMode::sMultiply = makeBlendMode(
        GfxBlendingMode::Blend::kDestColor,
        GfxBlendingMode::Blend::kZero,
        GfxBlendingMode::Blend::kDestAlpha,
        GfxBlendingMode::Blend::kZero);

    bool GfxBlendingMode::operator==(const GfxBlendingMode &other) const
    {
        return alphaToCoverageEnable == other.alphaToCoverageEnable &&
               blendEnable == other.blendEnable &&
               srcBlend == other.srcBlend &&
               destBlend == other.destBlend &&
               blendOp == other.blendOp &&
               srcBlendAlpha == other.srcBlendAlpha &&
               destBlendAlpha == other.destBlendAlpha &&
               blendOpAlpha == other.blendOpAlpha &&
               renderTargetWriteMask == other.renderTargetWriteMask;
    }

    u32 getHash(const GfxBlendingMode &key)
    {
        u32 hash = HashFunc<u8>::get(key.alphaToCoverageEnable ? 1 : 0);
        combineHash(hash, HashFunc<u8>::get(key.blendEnable ? 1 : 0));
        combineHash(hash, HashFunc<GfxBlendingMode::Blend>::get(key.srcBlend));
        combineHash(hash, HashFunc<GfxBlendingMode::Blend>::get(key.destBlend));
        combineHash(hash, HashFunc<GfxBlendingMode::Operation>::get(key.blendOp));
        combineHash(hash, HashFunc<GfxBlendingMode::Blend>::get(key.srcBlendAlpha));
        combineHash(hash, HashFunc<GfxBlendingMode::Blend>::get(key.destBlendAlpha));
        combineHash(hash, HashFunc<GfxBlendingMode::Operation>::get(key.blendOpAlpha));
        combineHash(hash, HashFunc<GfxBlendingMode::ColorWrite>::get(key.renderTargetWriteMask));
        return hash;
    }

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
