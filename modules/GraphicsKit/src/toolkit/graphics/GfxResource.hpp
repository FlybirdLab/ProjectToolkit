/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/StringView.hpp"
#include <toolkit/graphics/common.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit
{

// Releases GPU resource memory and deletes object.
#define GFX_DEL_RESOURCE(x)                                   \
    do                                                        \
    {                                                         \
        if (x)                                                \
        {                                                     \
            TkMemory::deletePtr(TkMemory::getDefault(), (x)); \
            (x) = nullptr;                                    \
        }                                                     \
    } while (false)
#define GFX_DEL_RESOURCES(x)                                      \
    do                                                            \
    {                                                             \
        for (auto &e : (x))                                       \
            if (e)                                                \
            {                                                     \
                TkMemory::deletePtr(TkMemory::getDefault(), (e)); \
                e = nullptr;                                      \
            }                                                     \
    } while (false)

    // The base class for all GPU resources.
    class TK_API GfxResource
    {
    public:
        NON_COPYABLE(GfxResource);

        GfxResource();
        virtual ~GfxResource()
        {
            releaseData();
        }

    public:
        // Points to the cache used by the resource for the resource visibility/usage detection. Written during rendering when resource is used.
        f64 lastRenderTime = -1;
        // Action fired when resource GPU state gets released. All objects and async tasks using this resource should release references to this object nor use its data.
        TkAction<> onReleasing;

    public:
        // Get the GPU resource type.
        virtual GfxResType getType() const = 0;
        TkStringView toString() const;
        // Get amount of GPU memory used by this resource (in bytes). It's a rough estimation. GPU memory may be fragmented, compressed or sub-allocated so the actual memory pressure from this resource may vary (also depends on the current graphics backend).
        u64 getMemoryUse() const;

#if TK_DEBUG
        // Get the resource name.
        TkStringView getName() const;
        // Set the resource name.
        void setName(const TkStringView &name);
#endif
        // Release GPU resource data.
        void releaseData();
        // Action called when GPU device is disposing.
        virtual void onDeviceDispose();

    protected:
        // Releases GPU resource data (implementation).
        virtual void onReleaseData();

    protected:
        u64 m_memoryUsage = 0;
#if TK_DEBUG
        char *m_namePtr = nullptr;
        i32 m_nameSize = 0, m_nameCapacity = 0;
#endif
    };

    // Describes base implementation of Graphics Device resource for rendering back-ends.
    template <class DeviceType, class BaseType>
    class GfxResourceBase : public BaseType
    {
    public:
        GfxResourceBase(DeviceType *device, const TkStringView &name) noexcept
            : BaseType(), m_device(device)
        {
#if TK_DEBUG
            if (name.hasChars())
                BaseType::setName(name);
#endif
            device->addResource(this);
        }

        virtual ~GfxResourceBase()
        {
            if (m_device)
                m_device->removeResource(this);
        }

    public:
        // Get the graphics device.
        TK_FORCE_INLINE DeviceType *getDevice() const
        {
            return m_device;
        }

        void onDeviceDispose() override
        {
            BaseType::onDeviceDispose();
            m_device = nullptr;
        }

    protected:
        DeviceType *m_device;
    };

    // Interface for GPU resources views. Shared base class for texture and buffer views.
    class TK_API GfxResourceView
    {
    protected:
        explicit GfxResourceView()
            : lastRenderTime(&s_dummyLastRenderTime)
        {
        }

    public:
        // Points to the cache used by the resource for the resource visibility/usage detection. Written during rendering when resource view is used.
        f64 *lastRenderTime;

        // Get parent GPU resource owning that view.
        TK_FORCE_INLINE GfxResource *getParent() const
        {
            return m_parent;
        }

        // Get the native pointer to the underlying view. It's a platform-specific handle.
        virtual void *getNativePtr() const = 0;

    protected:
        static f64 s_dummyLastRenderTime;
        GfxResource *m_parent = nullptr;
    };

    // GPU Resource container utility object.
    class TK_API GfxResourcePropertyBase
    {
    public:
        NON_COPYABLE(GfxResourcePropertyBase);

        GfxResourcePropertyBase() = default;
        ~GfxResourcePropertyBase();

    public:
        // Action fired when resource gets released (reference gets cleared bu async tasks should stop execution).
        TkAction<> onRelease;

    protected:
        void onSet(GfxResource *resource);
        void onReleased();

    protected:
        GfxResource *m_resource = nullptr;
    };

    // GPU Resource container utility object.
    template <typename T = GfxResource>
    class GfxResourceProperty : public GfxResourcePropertyBase
    {
    public:
        GfxResourceProperty() = default;
        GfxResourceProperty(T *resource)
        {
            onSet(resource);
        }
        GfxResourceProperty(const GfxResourceProperty &other)
        {
            onSet(other.get());
        }
        GfxResourceProperty(GfxResourceProperty &&other)
        {
            onSet(other.get());
            other.onSet(nullptr);
        }

        GfxResourceProperty &operator=(GfxResourceProperty &&other)
        {
            if (&other != this)
            {
                onSet(other.m_resource);
                other.onSet(nullptr);
            }
            return *this;
        }
        ~GfxResourceProperty() = default;

    public:
        TK_FORCE_INLINE bool operator==(T *other) const
        {
            return get() == other;
        }

        TK_FORCE_INLINE bool operator==(const GfxResourceProperty &other) const
        {
            return get() == other.get();
        }

        TK_FORCE_INLINE GfxResourceProperty &operator=(T &other)
        {
            onSet(&other);
            return *this;
        }

        TK_FORCE_INLINE GfxResourceProperty &operator=(T *other)
        {
            onSet(other);
            return *this;
        }

        // Implicit conversion to GPU Resource
        TK_FORCE_INLINE operator T *() const
        {
            return (T *)m_resource;
        }

        // Implicit conversion to resource
        TK_FORCE_INLINE operator bool() const
        {
            return m_resource != nullptr;
        }

        // Implicit conversion to resource
        TK_FORCE_INLINE T *operator->() const
        {
            return (T *)m_resource;
        }

        // Get linked resource
        TK_FORCE_INLINE T *get() const
        {
            return (T *)m_resource;
        }

    public:
        // Set resource
        void set(T *value)
        {
            onSet(value);
        }

        // Unlink resource
        void unlink()
        {
            onSet(nullptr);
        }
    };

    // Tracking of per-resource or per-subresource state for GPU resources that require to issue resource access barriers during rendering.
    template <typename StateType, StateType InvalidState>
    class GfxResourceState
    {
    public:
        GfxResourceState()
            : m_resourceState(InvalidState), m_allSubresourcesSame(true)
        {
        }

    public:
        void init(u32 subresourceCount, StateType initialState, bool usePerSubresourceTracking)
        {
            TK_ASSERT(m_subresourceState.isEmpty() && subresourceCount > 0);
            m_allSubresourcesSame = true;
            m_resourceState = initialState;
            if (usePerSubresourceTracking && subresourceCount > 1)
                m_subresourceState.resize(subresourceCount, false);
#if TK_DEBUG
            m_subresourceState.setAll(InvalidState);
#endif
        }

        bool isInit() const
        {
            return m_resourceState != InvalidState || m_subresourceState.hasItems();
        }

        void release()
        {
            m_resourceState = InvalidState;
            m_subresourceState.resize(0);
        }

        bool areAllSubresourcesSame() const
        {
            return m_allSubresourcesSame;
        }

        i32 getSubresourcesCount() const
        {
            return m_subresourceState.count();
        }

        bool checkResourceState(StateType state) const
        {
            if (m_allSubresourcesSame)
                return state == m_resourceState;
            for (i32 i = 0; i < m_subresourceState.count(); i++)
            {
                if (m_subresourceState[i] != state)
                    return false;
            }
            return true;
        }

        StateType getSubresourceState(u32 subresourceIndex) const
        {
            if (m_allSubresourcesSame)
                return m_resourceState;
            TK_ASSERT(subresourceIndex >= 0 && subresourceIndex < static_cast<u32>(m_subresourceState.count()));
            return m_subresourceState[subresourceIndex];
        }

        void setResourceState(StateType state)
        {
            m_allSubresourcesSame = 1;
            m_resourceState = state;
#if TK_DEBUG
            for (i32 i = 0; i < m_subresourceState.count(); i++)
                m_subresourceState[i] = InvalidState;
#endif
        }

        void setSubresourceState(i32 subresourceIndex, StateType state)
        {
            // Check if use single state for the whole resource
            if (subresourceIndex == -1 || m_subresourceState.count() <= 1)
            {
                setResourceState(state);
            }
            else
            {
                TK_ASSERT(subresourceIndex < static_cast<i32>(m_subresourceState.count()));

                // Transition for all sub-resources
                if (m_allSubresourcesSame)
                {
                    for (i32 i = 0; i < m_subresourceState.count(); i++)
                        m_subresourceState[i] = m_resourceState;
                    m_allSubresourcesSame = 0;
#if TK_DEBUG
                    m_resourceState = InvalidState;
#endif
                }

                m_subresourceState[subresourceIndex] = state;
            }
        }

    private:
        // The whole resource state (used only if m_allSubresourcesSame is 1).
        StateType m_resourceState : 31;
        // Set to 1 if m_resourceState is valid. In this case, all subresources have the same state.
        // Set to 0 if m_subresourceState is valid. In this case, each subresources may have a different state (or may be unknown).
        u32 m_allSubresourcesSame : 1;
        // The per subresource state (used only if m_allSubresourcesSame is 0).
        TkArray<StateType> m_subresourceState;
    };

    typedef GfxResourceProperty<GfxResource> GfxResourceReference;
    typedef GfxResourceProperty<GfxTexture> GPUTextureReference;
    typedef GfxResourceProperty<GfxBuffer> BufferReference;

}