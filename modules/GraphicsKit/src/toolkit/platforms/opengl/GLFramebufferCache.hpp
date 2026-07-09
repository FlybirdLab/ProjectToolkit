/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Cache of GL framebuffer objects keyed on (color view, depth view) pairs.
    //
    // OpenGL FBO objects are *not* shared across GL contexts even when the
    // contexts share resources, so this cache is implicitly bound to the GL
    // context that was current when getOrCreate() was first called. In Phase 5
    // that's the swapchain's window context (callers must have a current
    // render context already).
    //
    // Phase 5 limits:
    //   - At most one color attachment (no MRT).
    //   - At most one depth/stencil attachment.
    //   - No layered attachments (cube faces, array slices, 3D slices).
    //   - Lookup is linear; we expect O(few) live framebuffers.
    class GLFramebufferCache
    {
    public:
        struct Key
        {
            class GLTextureView *color = nullptr;
            class GLTextureView *depth = nullptr;

            bool operator==(const Key &rhs) const
            {
                return color == rhs.color && depth == rhs.depth;
            }
        };

        struct Entry
        {
            Key key;
            u32 fbo = 0;
            i32 width = 0;
            i32 height = 0;
        };

        GLFramebufferCache() = default;
        ~GLFramebufferCache() = default;

        bool init();
        void shutdown();

        // Returns the GL framebuffer object name for the given attachment
        // combination, allocating it on first use. Returns 0 on failure.
        u32 getOrCreate(class GLTextureView *color, class GLTextureView *depth);

        // Drop every FBO. Caller must guarantee the right GL context is
        // current (or accept that some glDeleteFramebuffers calls may target
        // names that the driver already cleaned up at context destruction).
        void clear();

        // Forget any cached entry whose color or depth slot points at the
        // given view. Called when a texture is being released so we don't
        // hand out a stale FBO afterwards.
        void invalidateForView(class GLTextureView *view);

    private:
        u32 createFramebuffer(class GLTextureView *color, class GLTextureView *depth, i32 &outWidth, i32 &outHeight);

        TkArray<Entry> m_entries;
    };

}

#endif
