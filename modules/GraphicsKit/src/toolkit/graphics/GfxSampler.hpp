/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/GfxResource.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    // GPU sampler filter modes.
    enum class GfxSamplerFilter
    {
        // Filter using the nearest found pixel. Texture appears pixelated.
        kPoint = 0,
        // Filter using the linear average of the nearby pixels. Texture appears blurry.
        kBilinear = 1,
        // Filter using the linear average of the nearby pixels and nearby mipmaps. Texture appears blurry.
        kTrilinear = 2,
        // Filter using the anisotropic filtering that improves quality when viewing textures at a steep angles. Texture appears sharp at extreme viewing angles.
        kAnisotropic = 3,
        MAX
    };

    // GPU sampler address modes.
    enum class GfxSamplerAddressMode
    {
        // Texture coordinates wrap back to the valid range.
        kWrap = 0,
        // Texture coordinates are clamped within the valid range.
        kClamp = 1,
        // Texture coordinates flip every time the size of the valid range is passed.
        kMirror = 2,
        // Texture coordinates outside of the valid range will return a separately set border color.
        kBorder = 3,
        MAX
    };

    // GPU sampler comparision function types.
    enum class GfxSamplerCompareFunction
    {
        // Never pass the comparison.
        kNever = 0,
        // If the source data is less than the destination data, the comparison passes.
        kLess = 1,
        MAX
    };

    // GPU sampler border color types.
    enum class GfxSamplerBorderColor
    {
        // Indicates black, with the alpha component as fully transparent.
        kTransparentBlack = 0,
        // Indicates black, with the alpha component as fully opaque.
        kOpaqueBlack = 1,
        // Indicates white, with the alpha component as fully opaque.
        kOpaqueWhite = 2,
        MAX
    };

    // A common description for all samplers.
    struct TK_API GfxSamplerDescription
    {
        // The filtering method to use when sampling a texture.
        GfxSamplerFilter filter;
        // The addressing mode for outside [0..1] range for U coordinate.
        GfxSamplerAddressMode addressU;
        // The addressing mode for outside [0..1] range for V coordinate.
        GfxSamplerAddressMode addressV;
        // The addressing mode for outside [0..1] range for W coordinate.
        GfxSamplerAddressMode addressW;

        // The mip bias to be added to mipmap LOD calculation.
        f32 mipBias;
        // The minimum mip map level that will be used, where 0 is the highest resolution mip level.
        f32 minMipLevel;
        // The maximum mip map level that will be used, where 0 is the highest resolution mip level. To have no upper limit on LOD set this to a large value such as MAX_f32.
        f32 maxMipLevel;

        // The maximum number of samples that can be used to improve the quality of sample footprints that are anisotropic.
        i32 maxAnisotropy;

        // The border color to use if Border is specified for AddressU, AddressV, or AddressW.
        GfxSamplerBorderColor borderColor;
        // A function that compares sampled data against existing sampled data.
        GfxSamplerCompareFunction comparisonFunction;

    public:
        // Creates a new Sampler with default settings.
        static GfxSamplerDescription newSampler(GfxSamplerFilter filter = GfxSamplerFilter::kPoint, GfxSamplerAddressMode addressMode = GfxSamplerAddressMode::kWrap);

    public:
        void clear();
        bool equals(const GfxSamplerDescription &other) const;
        TkString toString() const;

    public:
        TK_FORCE_INLINE bool operator==(const GfxSamplerDescription &other) const
        {
            return equals(other);
        }

        TK_FORCE_INLINE bool operator!=(const GfxSamplerDescription &other) const
        {
            return !equals(other);
        }
    };
    u32 getHash(const GfxSamplerDescription &key);
    template <>
    struct HashFunc<GfxSamplerDescription>
    {
        static u32 get(const GfxSamplerDescription &k)
        {
            return getHash(k);
        }
    };

    
    // GPU texture sampler object.
    class TK_API GfxSampler : public GfxResource
    {
        static GfxSampler *newSampler();

    protected:
        GfxSampler();

    public:
        // Gets sampler description structure.
        const GfxSamplerDescription &getDescription() const
        {
            return m_desc;
        }

        // Creates new sampler
        bool init(const GfxSamplerDescription &desc);

        GfxResType getType() const final override;

    protected:
        virtual bool onInit() = 0;
        void onReleaseData() override;

    protected:
        GfxSamplerDescription m_desc;
    };

}