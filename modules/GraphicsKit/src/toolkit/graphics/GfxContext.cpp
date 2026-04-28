/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxContext.hpp"
#include "GfxTexture.hpp"
#include "GfxDevice.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/Span.hpp"

namespace toolkit
{

    GfxContext::GfxContext(GfxDevice *device)
        : m_device(device)
    {
    }

#if TK_DEBUG

    void GfxContext::logInvalidResourceUsage(i32 slot, const GfxResourceView *view, InvalidBindPoint bindPoint)
    {
        GfxResource *resource = view ? view->getParent() : nullptr;
        const char *resourceType = "resource";
        const char *flagType = "flags";
        if (resource)
        {
            switch (resource->getType())
            {
            case GfxResType::kRenderTarget:
            case GfxResType::kTexture:
            case GfxResType::kCubeTexture:
            case GfxResType::kVolumeTexture:
                resourceType = "texture";
                flagType = "GfxTextureFlags";
                break;
            case GfxResType::kBuffer:
                resourceType = "buffer";
                flagType = "GfxBufferFlags";
                break;
            default:
                break;
            }
        }
        const char *usage = "-";
        switch (bindPoint)
        {
        case InvalidBindPoint::SRV:
            usage = "shader resource";
            break;
        case InvalidBindPoint::UAV:
            usage = "unordered access";
            break;
        case InvalidBindPoint::DSV:
            usage = "depth stencil";
            break;
        case InvalidBindPoint::RTV:
            usage = "render target";
            break;
        }
        LogError("Incorrect %s bind at slot %i as %s (ensure to setup correct %s when creating that resource)", resourceType, slot, usage, flagType);
    }

#endif

    void GfxContext::frameBegin()
    {
        m_lastRenderTime = System::getTimeSeconds();
    }

    void GfxContext::frameEnd()
    {
        clearState();
        flushState();
    }

    void GfxContext::bindSR(i32 slot, GfxTexture *t)
    {
        TK_ASSERT(t == nullptr || t->residentMipLevels() == 0 || t->isShaderResource());
        bindSR(slot, GET_TEXTURE_VIEW_SAFE(t));
    }

    void GfxContext::draw(GfxTexture *dst, GfxTexture *src)
    {
        TK_ASSERT(dst && src);
        resetRenderTarget();
        const float width = (float)dst->width();
        const float height = (float)dst->height();
        setViewport(width, height);
        setRenderTarget(dst->view());
        bindSR(0, src->view());
        auto *ps = m_device->getCopyLinearPS();
        if (!ps)
        {
            LogError("Gfx copy pipeline is not initialized.");
            return;
        }
        setState(ps);
        drawFullscreenTriangle();
    }

    void GfxContext::draw(GfxTexture *rt)
    {
        TK_ASSERT(rt);
        bindSR(0, rt);
        auto *ps = m_device->getCopyLinearPS();
        if (!ps)
        {
            LogError("Gfx copy pipeline is not initialized.");
            return;
        }
        setState(ps);
        drawFullscreenTriangle();
    }

    void GfxContext::draw(GfxTextureView *rt)
    {
        TK_ASSERT(rt);
        bindSR(0, rt);
        auto *ps = m_device->getCopyLinearPS();
        if (!ps)
        {
            LogError("Gfx copy pipeline is not initialized.");
            return;
        }
        setState(ps);
        drawFullscreenTriangle();
    }

    void GfxContext::drawFullscreenTriangle()
    {
        auto *vb = m_device->getFullscreenTriangleVB();
        if (!vb)
        {
            LogError("Fullscreen triangle vertex buffer is not initialized.");
            return;
        }

        bindVB(TkSpan<GfxBuffer *>(&vb, 1));
        draw(0, 3);
    }

    void GfxContext::setResourceState(GfxResource *resource, u64 state, i32 subresource)
    {
    }

    void GfxContext::forceRebindDescriptors()
    {
    }

}
