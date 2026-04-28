/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxTargetPool.hpp"
#include "GfxTexture.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/graphics/GfxContext.hpp"
#include "toolkit/graphics/GfxDevice.hpp"
#include "toolkit/threading/Thread.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{
    struct Entry
    {
        GfxTexture *rt;
        u64 lastFrameReleased;
        u32 descriptionHash;
        bool isOccupied;
    };

    TkArray<Entry> s_temporaryRTs;

    void GfxTargetPool::flush(bool force, i32 framesOffset)
    {
        PROFILE();
        if (framesOffset < 0)
            framesOffset = 3 * 60; // For how many frames RTs should be cached (by default)
        auto *device = GfxDevice::getInstance();
        if (!device)
            return;
        const u64 frameCount = device ? device->getFrameIndex() : 0;
        const u64 maxReleaseFrame = frameCount - TkMath::min<u64>(frameCount, framesOffset);
        force |= device ? device->isDisposing() : true;

        for (i32 i = 0; i < s_temporaryRTs.count(); i++)
        {
            const auto &e = s_temporaryRTs[i];
            if (!e.isOccupied && (force || e.lastFrameReleased < maxReleaseFrame))
            {
                TkMemory::deletePtr(e.rt);
                s_temporaryRTs.removeAt(i--);
                if (s_temporaryRTs.isEmpty())
                    break;
            }
        }
    }

    GfxTexture *GfxTargetPool::get(const GfxTextureDescription &desc)
    {
        PROFILE();
        auto *device = GfxDevice::getInstance();
        if (!device)
            return nullptr;

        // Initialize render targets with pink color in debug builds to prevent incorrect data usage (GPU doesn't clear texture upon creation)
#if TK_DEBUG
#define GFX_TARGET_POOL_CLEAR()                                                                                                                                          \
    if (desc.dimensions == TextureDimensions::kTexture && enumHasAllFlags(desc.flags, GfxTextureFlags::kRenderTarget) && device->isRendering() && TkThread::isInMainThread()) \
        device->getMainContext()->clear(e.rt->view(), Color::Pink);
#else
#define GFX_TARGET_POOL_CLEAR()
#endif

        // Find free render target with the same properties
        const u32 descHash = HashFunc<GfxTextureDescription>::get(desc);
        for (i32 i = 0; i < s_temporaryRTs.count(); i++)
        {
            auto &e = s_temporaryRTs[i];
            if (!e.isOccupied && e.descriptionHash == descHash)
            {
                // Mark as used
                e.isOccupied = true;
                GFX_TARGET_POOL_CLEAR();
                return e.rt;
            }
        }
#if TK_DEBUG
        if (s_temporaryRTs.count() > 2000)
        {
            LogError("Too many textures allocated in GfxTargetPool. Know your limits, sir!");
            return nullptr;
        }
#endif

        // Create new rt
        const TkString name = "TemporaryRT_" + TkStringUtils::toString(s_temporaryRTs.count());
        GfxTexture *rt = device->createTexture(name);
        if (!rt->init(desc))
        {
            TkMemory::deletePtr(rt);
            LogError("Cannot create temporary render target. Description: %s", desc.toString());
            return nullptr;
        }

        // Create temporary rt entry
        Entry e;
        e.isOccupied = true;
        e.lastFrameReleased = 0;
        e.rt = rt;
        e.descriptionHash = descHash;
        s_temporaryRTs.add(e);
        GFX_TARGET_POOL_CLEAR();

#undef GFX_TARGET_POOL_CLEAR
        return rt;
    }

    void GfxTargetPool::release(GfxTexture *rt)
    {
        if (!rt)
            return;
        for (i32 i = 0; i < s_temporaryRTs.count(); i++)
        {
            auto &e = s_temporaryRTs[i];
            if (e.rt == rt)
            {
                // Mark as free
                TK_ASSERT(e.isOccupied);
                e.isOccupied = false;
                auto *device = GfxDevice::getInstance();
                e.lastFrameReleased = device ? device->getFrameIndex() : 0;
                return;
            }
        }
        LogError("Trying to release temporary render target which has not been registered in service!");
    }

}
