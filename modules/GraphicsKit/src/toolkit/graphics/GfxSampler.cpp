/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxSampler.hpp"
#include "GfxDevice.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    GfxSamplerDescription GfxSamplerDescription::newSampler(GfxSamplerFilter filter, GfxSamplerAddressMode addressMode)
    {
        GfxSamplerDescription desc;
        desc.clear();
        desc.filter = filter;
        desc.addressU = desc.addressV = desc.addressW = addressMode;
        return desc;
    }

    void GfxSamplerDescription::clear()
    {
        TkMemory::memoryClear(this, sizeof(GfxSamplerDescription));
        maxMipLevel = kMaxF32;
    }

    bool GfxSamplerDescription::equals(const GfxSamplerDescription &other) const
    {
        return filter == other.filter && addressU == other.addressU && addressV == other.addressV && addressW == other.addressW && mipBias == other.mipBias && maxAnisotropy == other.maxAnisotropy && minMipLevel == other.minMipLevel && maxMipLevel == other.maxMipLevel && borderColor == other.borderColor && comparisonFunction == other.comparisonFunction;
    }

    TkString GfxSamplerDescription::toString() const
    {
        return TkString::format("Filter: %i, Address: %ix%ix%i, MipBias: %i, MaxAnisotropy: %i, MinMipLevel: %i, maxMipLevel: %i, BorderColor: %i, ComparisonFunction: %i",
                              (i32)filter,
                              (i32)addressU,
                              (i32)addressV,
                              (i32)addressW,
                              mipBias,
                              maxAnisotropy,
                              minMipLevel,
                              maxMipLevel,
                              (i32)borderColor,
                              (i32)comparisonFunction);
    }

    u32 getHash(const GfxSamplerDescription &key)
    {
        u32 hashCode = (u32)key.filter;
        hashCode = (hashCode * 397) ^ (u32)key.addressU;
        hashCode = (hashCode * 397) ^ (u32)key.addressV;
        hashCode = (hashCode * 397) ^ (u32)key.addressW;
        hashCode = (hashCode * 397) ^ (u32)key.mipBias;
        hashCode = (hashCode * 397) ^ (u32)key.maxAnisotropy;
        hashCode = (hashCode * 397) ^ (u32)key.minMipLevel;
        hashCode = (hashCode * 397) ^ (u32)key.maxMipLevel;
        hashCode = (hashCode * 397) ^ (u32)key.borderColor;
        hashCode = (hashCode * 397) ^ (u32)key.comparisonFunction;
        return hashCode;
    }

    GfxSampler *GfxSampler::newSampler()
    {
        return GfxDevice::sInstance->createSampler();
    }

    GfxSampler::GfxSampler()
        : GfxResource {}
    {
    }

    bool GfxSampler::init(const GfxSamplerDescription &desc)
    {
        releaseData();
        m_desc = desc;
        if (!onInit())
        {
            releaseData();
            LogWarn("Cannot initialize sampler. Description: %s", desc.toString());
            return false;
        }
        return true;
    }

    GfxResType GfxSampler::getType() const
    {
        return GfxResType::kSampler;
    }

    void GfxSampler::onReleaseData()
    {
        m_desc.clear();
    }

}