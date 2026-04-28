/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxResource.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    static const char *s_GfxResType[] = {
        "GfxRenderTarget",
        "GfxTexture",
        "GfxCubeTexture",
        "GfxVolumeTexture",
        "GfxBuffer",
        "GfxShader",
        "GfxPipelineState",
        "GfxDescriptor",
        "GfxQuery",
        "GfxSampler"};

    TkStringView GfxResource::toString() const
    {
        return s_GfxResType[(i32)getType()];
    }

    GfxResourcePropertyBase::~GfxResourcePropertyBase()
    {
        const auto e = m_resource;
        if (e)
        {
            m_resource = nullptr;
            e->onReleasing.unbind<GfxResourcePropertyBase, &GfxResourcePropertyBase::onReleased>(this);
        }
    }

    void GfxResourcePropertyBase::onSet(GfxResource *resource)
    {
        auto e = m_resource;
        if (e != resource)
        {
            if (e)
                e->onReleasing.unbind<GfxResourcePropertyBase, &GfxResourcePropertyBase::onReleased>(this);
            m_resource = e = resource;
            if (e)
                e->onReleasing.bind<GfxResourcePropertyBase, &GfxResourcePropertyBase::onReleased>(this);
        }
    }

    void GfxResourcePropertyBase::onReleased()
    {
        auto e = m_resource;
        if (e)
        {
            m_resource = nullptr;
            e->onReleasing.unbind<GfxResourcePropertyBase, &GfxResourcePropertyBase::onReleased>(this);
        }
    }

    GfxResource::GfxResource()
    {
    }

    GfxResource::~GfxResource()
    {
        // onDeletingObject
        releaseData();

#if TK_DEBUG && GFX_ENABLE_RESOURCE_NAMING
        if (m_memoryUsage != 0)
            LogError("'%s' has not been disposed before destruction", getName());
#endif
#if GFX_ENABLE_RESOURCE_NAMING
        if (m_namePtr)
            TkMemory::free(TkMemory::getDefault(), m_namePtr);
#endif
    }

    u64 GfxResource::getMemoryUse() const
    {
        return m_memoryUsage;
    }

    static_assert((GFX_ENABLE_RESOURCE_NAMING) == (TK_DEBUG), "Update build condition on around GfxResource Name property getter/setter.");

#if GFX_ENABLE_RESOURCE_NAMING

    TkStringView GfxResource::getName() const
    {
        return TkStringView(m_namePtr, m_nameSize);
    }

    void GfxResource::setName(const TkStringView &name)
    {
        if (m_nameCapacity < name.length() + 1)
        {
            if (m_namePtr)
                TkMemory::free(TkMemory::getDefault(), m_namePtr);
            m_nameCapacity = name.length() + 1;
            m_namePtr = (char *)TkMemory::allocate(TkMemory::getDefault(), m_nameCapacity * sizeof(char), 16);
        }
        m_nameSize = name.length();
        if (name.hasChars())
        {
            TkMemory::memoryCopy(m_namePtr, name.get(), m_nameSize * sizeof(char));
            m_namePtr[m_nameSize] = 0;
        }
    }

#endif

    void GfxResource::releaseData()
    {
        if (m_memoryUsage != 0)
        {
            onReleasing();
            onReleaseData();
            m_memoryUsage = 0;
        }
    }

    void GfxResource::onDeviceDispose()
    {
        // By default we want to release resource data but keep it alive
        onReleaseData();
    }

    void GfxResource::onReleaseData()
    {
    }

    double GfxResourceView::s_dummyLastRenderTime = -1;

}