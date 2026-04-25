/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxVertexLayout.hpp"

#include "toolkit/collection/HashMap.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/Span.hpp"
#include "GfxDevice.hpp"

namespace toolkit
{

    // VertexElement has been designed to be POD and memory-comparable for faster hashing and comparision.
    struct VertexElementRaw
    {
        u32 words[2];
    };
    static_assert(sizeof(VertexElement) == sizeof(VertexElementRaw), "Incorrect size of the VertexElement!");

    struct VertexBufferLayouts
    {
        GfxVertexLayout *layouts[GFX_MAX_VB_BINDED];

        bool operator==(const VertexBufferLayouts &other) const
        {
            return TkMemory::memoryCompare(&layouts, &other.layouts, sizeof(layouts)) == 0;
        }
    };

    u32 getHash(const VertexBufferLayouts &key)
    {
        u32 hash = HashFunc<void *>::get(key.layouts[0]);
        for (i32 i = 1; i < GFX_MAX_VB_BINDED; i++)
            combineHash(hash, HashFunc<void *>::get(key.layouts[i]));
        return hash;
    }

    template <>
    struct HashFunc<VertexBufferLayouts>
    {
        static u32 get(const VertexBufferLayouts &key)
        {
            return getHash(key);
        }
    };

    TkMutex s_cacheMtx;
    TkHashMap<u32, GfxVertexLayout *> s_layoutCache;
    TkHashMap<VertexBufferLayouts, GfxVertexLayout *> s_vertexBufferCache;

    static GfxVertexLayout *addCache(const VertexBufferLayouts &key, i32 count)
    {
        GfxVertexLayout::Elements elements;
        bool anyValid = false;
        for (i32 slot = 0; slot < count; slot++)
        {
            if (key.layouts[slot])
            {
                anyValid = true;
                i32 start = elements.count();
                elements.add(key.layouts[slot]->getElements());
                for (i32 j = start; j < elements.count(); j++)
                    elements.get()[j].slot = (byte)slot;
            }
        }
        GfxVertexLayout *result = anyValid ? GfxVertexLayout::get(elements) : nullptr;
        s_vertexBufferCache.add(key, result);
        return result;
    }

    TkString VertexElement::toString() const
    {
#if GFX_ENABLE_RESOURCE_NAMING
        return TkString::format("%i, %i, offset %d, %s, slot %i", type, format, offset, perInstance ? "per-instance" : "per-vertex", slot);
#else
        return "VertexElement";
#endif
    }

    bool VertexElement::operator==(const VertexElement &other) const
    {
        auto thisRaw = (const VertexElementRaw *)this;
        auto otherRaw = (const VertexElementRaw *)&other;
        return thisRaw->words[0] == otherRaw->words[0] && thisRaw->words[1] == otherRaw->words[1];
    }

    u32 getHash(const VertexElement &key)
    {
        auto keyRaw = (const VertexElementRaw *)&key;
        u32 hash = keyRaw->words[0];
        combineHash(hash, keyRaw->words[1]);
        return hash;
    }

    GfxVertexLayout::GfxVertexLayout()
        : GfxResource {}
    {
    }

    void GfxVertexLayout::setElements(const Elements &elements, bool explicitOffsets)
    {
        u32 offsets[GFX_MAX_VB_BINDED + 1] = {};
        m_elements = elements;
        for (i32 i = 0; i < m_elements.count(); i++)
        {
            VertexElement &e = m_elements[i];
            TK_ASSERT(e.slot <= GFX_MAX_VB_BINDED); // One special slot after all VBs for any missing vertex elements binding (on Vulkan)
            u32 &offset = offsets[e.slot];
            if (e.offset != 0 || explicitOffsets)
                offset = e.offset;
            else
                e.offset = (byte)offset;
            offset += GfxPixelFormat::sizeInBytes(e.format);
        }
        m_stride = 0;
        for (u32 offset : offsets)
            m_stride += offset;
    }

    TkString GfxVertexLayout::getElementsString() const
    {
        TkString result;
        for (i32 i = 0; i < m_elements.count(); i++)
        {
            if (i != 0)
                result += '\n';
            result += m_elements[i].toString();
        }
        return result;
    }

    VertexElement GfxVertexLayout::findElement(VertexElementTypes type) const
    {
        for (const VertexElement &e : m_elements)
        {
            if (e.type == type)
                return e;
        }
        return VertexElement();
    }

    GfxVertexLayout *GfxVertexLayout::get(const Elements &elements, bool explicitOffsets)
    {
        // Hash input layout
        u32 hash = 0;
        for (const VertexElement &element : elements)
        {
            combineHash(hash, HashFunc<VertexElement>::get(element));
        }

        // Lookup existing cache
        s_cacheMtx.lock();
        GfxVertexLayout *result;
        bool isContains = s_layoutCache.containsKey(hash);
        if (isContains) result = s_layoutCache[hash];
        if (!isContains)
        {
            result = GfxDevice::sInstance->createVertexLayout(elements, explicitOffsets);
            if (!result)
            {
#if TK_DEBUG
                for (auto &e : elements)
                    LogError(" %s", e.toString());
#endif
                LogError("Failed to create vertex layout");
                s_cacheMtx.unlock();
                return nullptr;
            }
            s_layoutCache.add(hash, result);
        }
        s_cacheMtx.unlock();

        return result;
    }

    GfxVertexLayout *GfxVertexLayout::get(const TkSpan<GfxBuffer *> &vertexBuffers)
    {
        if (vertexBuffers.length() == 0)
            return nullptr;
        if (vertexBuffers.length() == 1)
            return vertexBuffers.get()[0] ? vertexBuffers.get()[0]->getVertexLayout() : nullptr;

        // Build hash key for set of buffers (in case there is layout sharing by different sets of buffers)
        VertexBufferLayouts key;
        for (i32 i = 0; i < vertexBuffers.length() && i < GFX_MAX_VB_BINDED; i++)
            key.layouts[i] = vertexBuffers.get()[i] ? vertexBuffers.get()[i]->getVertexLayout() : nullptr;
        for (i32 i = vertexBuffers.length(); i < GFX_MAX_VB_BINDED; i++)
            key.layouts[i] = nullptr;

        // Lookup existing cache
        s_cacheMtx.lock();
        GfxVertexLayout *result;
        bool hasIt = s_vertexBufferCache.containsKey(key);
        if (hasIt) result = s_vertexBufferCache[key];
        if (!hasIt)
            result = addCache(key, vertexBuffers.length());
        s_cacheMtx.unlock();

        return result;
    }

    GfxVertexLayout *GfxVertexLayout::get(const TkSpan<GfxVertexLayout *> &layouts)
    {
        if (layouts.length() == 0)
            return nullptr;
        if (layouts.length() == 1)
            return layouts.get()[0] ? layouts.get()[0] : nullptr;

        // Build hash key for set of buffers (in case there is layout sharing by different sets of buffers)
        VertexBufferLayouts key;
        for (i32 i = 0; i < layouts.length() && i < GFX_MAX_VB_BINDED; i++)
            key.layouts[i] = layouts.get()[i];
        for (i32 i = layouts.length(); i < GFX_MAX_VB_BINDED; i++)
            key.layouts[i] = nullptr;

        // Lookup existing cache
        s_cacheMtx.lock();
        GfxVertexLayout *result;
        bool hasIt = s_vertexBufferCache.containsKey(key);
        if (hasIt) result = s_vertexBufferCache[key];
        if (!hasIt)
            result = addCache(key, layouts.length());
        s_cacheMtx.unlock();

        return result;
    }

    GfxVertexLayout *GfxVertexLayout::merge(GfxVertexLayout *base, GfxVertexLayout *reference, bool removeUnused, bool addMissing, i32 missingSlotOverride)
    {
        GfxVertexLayout *result = base ? base : reference;
        if (base && reference && base != reference)
        {
            bool elementsModified = false;
            Elements newElements = base->getElements();
            if (removeUnused)
            {
                for (i32 i = newElements.count() - 1; i >= 0; i--)
                {
                    bool missing = true;
                    const VertexElement &e = newElements.get()[i];
                    for (const VertexElement &ee : reference->getElements())
                    {
                        if (ee.type == e.type)
                        {
                            missing = false;
                            break;
                        }
                    }
                    if (missing)
                    {
                        // Remove unused element
                        newElements.removeAtKeepOrder(i);
                        elementsModified = true;
                    }
                }
            }
            if (addMissing)
            {
                for (const VertexElement &e : reference->getElements())
                {
                    bool missing = true;
                    for (const VertexElement &ee : base->getElements())
                    {
                        if (ee.type == e.type)
                        {
                            missing = false;
                            break;
                        }
                    }
                    if (missing)
                    {
                        // Insert any missing elements
                        VertexElement ne = {e.type, missingSlotOverride != -1 ? (byte)missingSlotOverride : e.slot, 0, e.perInstance, e.format};
                        if (e.type == VertexElementTypes::kTexCoord1 ||
                            e.type == VertexElementTypes::kTexCoord2 ||
                            e.type == VertexElementTypes::kTexCoord3)
                        {
                            // Alias missing texcoords with existing texcoords
                            for (const VertexElement &ee : newElements)
                            {
                                if (ee.type == VertexElementTypes::kTexCoord0)
                                {
                                    ne = ee;
                                    ne.type = e.type;
                                    break;
                                }
                            }
                        }
                        newElements.add(ne);
                        elementsModified = true;
                    }
                }
            }
            if (elementsModified)
                result = get(newElements, true);
        }
        return result;
    }

    void ClearVertexs_layoutCache()
    {
        for (const auto &e : s_layoutCache)
            TkMemory::deletePtr(e.Value);
        s_layoutCache.clear();
        s_vertexBufferCache.clear();
    }

}
