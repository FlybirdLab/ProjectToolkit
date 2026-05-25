/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxTexture.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Texture view for the OpenGL backend.
    //
    // Phase 5 adds explicit RT / DS kinds so GLContext / GLFramebufferCache
    // can dispatch on intent. Backbuffer views remain a distinct kind.
    //
    // SR and RT views may point at the same texture handle (a RenderTarget
    // texture also supports sampling). Cross-binding is allowed but the
    // feedback-loop guard in GLContext blocks the case where the same view is
    // bound as both the active RT and an SR at the same time.
    class GLTextureView : public GfxTextureView
    {
    public:
        enum class Kind
        {
            kUnknown,
            kBackBuffer,
            kShaderResource,
            kRenderTarget,
            kDepthStencil,
        };

        GLTextureView() = default;

        // Wire this view up as the swapchain's default framebuffer (FBO 0).
        // Caller still owns the swapchain.
        void initBackBuffer(class GLSwapChain *swapChain, PixelFormat format)
        {
            m_kind = Kind::kBackBuffer;
            m_swapChain = swapChain;
            m_owner = nullptr;
            m_texture = 0;
            m_target = 0;
            m_arrayOrDepthIndex = 0;
            m_mipMapIndex = 0;
            init(nullptr, format, GfxMSAALevel::kNone);
        }

        void initShaderResource(class GLTexture *owner, u32 texture, u32 target, PixelFormat format, i32 arrayOrDepthIndex, i32 mipMapIndex);
        void initRenderTarget(class GLTexture *owner, u32 texture, u32 target, PixelFormat format, i32 arrayOrDepthIndex, i32 mipMapIndex);
        void initDepthStencil(class GLTexture *owner, u32 texture, u32 target, PixelFormat format, i32 arrayOrDepthIndex, i32 mipMapIndex);

        // Kept for source-compat with Phase 4 callers that used the generic
        // "kTexture" name. Routes to initShaderResource.
        void initTexture(class GLTexture *owner, u32 texture, u32 target, PixelFormat format, i32 arrayOrDepthIndex, i32 mipMapIndex)
        {
            initShaderResource(owner, texture, target, format, arrayOrDepthIndex, mipMapIndex);
        }

        bool isBackBuffer() const { return m_kind == Kind::kBackBuffer; }
        bool isShaderResource() const { return m_kind == Kind::kShaderResource || m_kind == Kind::kRenderTarget; }
        bool isRenderTarget() const { return m_kind == Kind::kRenderTarget; }
        bool isDepthStencil() const { return m_kind == Kind::kDepthStencil; }

        // Compat with Phase-3/4 callers that asked "is this a texture view".
        // Returns true for any non-backbuffer kind that wraps a GL texture.
        bool isTexture() const { return m_kind == Kind::kShaderResource || m_kind == Kind::kRenderTarget || m_kind == Kind::kDepthStencil; }

        Kind kind() const { return m_kind; }
        class GLTexture *owner() const { return m_owner; }
        class GLSwapChain *getSwapChain() const { return m_swapChain; }

        u32 texture() const { return m_texture; }
        u32 target() const { return m_target; }
        i32 mipMapIndex() const { return m_mipMapIndex; }
        i32 arrayOrDepthIndex() const { return m_arrayOrDepthIndex; }

        // Backbuffer framebuffer is always 0; Phase-5 texture views also
        // report 0 because their framebuffer is now owned by GLFramebufferCache,
        // not by the view itself. Kept for API back-compat.
        u32 framebuffer() const { return 0; }

        void *getNativePtr() const override
        {
            return const_cast<void *>(static_cast<const void *>(this));
        }

    private:
        Kind m_kind = Kind::kUnknown;
        class GLSwapChain *m_swapChain = nullptr;
        class GLTexture *m_owner = nullptr;
        u32 m_texture = 0;
        u32 m_target = 0;
        i32 m_arrayOrDepthIndex = 0;
        i32 m_mipMapIndex = 0;
    };

    class GLTexture : public GfxTexture
    {
    public:
        ~GLTexture();

        // GL texture name (== GLuint). 0 when not allocated.
        u32 handle() const { return m_handle; }
        // GL texture target (== GLenum). 0 when not allocated.
        u32 target() const { return m_target; }
        // Re-export the format() method from GfxTexture for clarity.
        PixelFormat textureFormat() const { return m_desc.format; }

        // True if this texture was created with kRenderTarget flag.
        bool isColorRenderTarget() const { return m_desc.isRenderTarget(); }
        // True if this texture was created with kDepthStencil flag.
        bool isDepthRenderTarget() const { return m_desc.isDepthStencil(); }

        // Direct accessors for the typed views. view()/view(arr,mip) below
        // returns whichever is the natural "sampling" view; GLContext uses
        // the typed accessors when it needs the RT / DS view explicitly.
        GLTextureView *srView() const { return m_srView; }
        GLTextureView *rtView() const { return m_rtView; }
        GLTextureView *dsView() const { return m_dsView; }

        GfxTextureView *view(i32 arrayOrDepthIndex) const override;
        GfxTextureView *view(i32 arrayOrDepthIndex, i32 mipMapIndex) const override;
        GfxTextureView *viewArray() const override { return nullptr; }
        GfxTextureView *viewVolume() const override { return nullptr; }
        GfxTextureView *viewReadOnlyDepth() const override { return nullptr; }
        void *getNativePtr() const override { return nullptr; }
        bool getData(i32 arrayIndex, i32 mipMapIndex, TextureMipData &data, u32 mipRowPitch) override { return false; }

    protected:
        bool onInit() override;
        void onReleaseData() override;
        void onResidentMipsChange() override {}

    private:
        u32 m_handle = 0;
        u32 m_target = 0;
        // Owned by the texture. At most one of (sr, rt) and one of (ds) are
        // non-null per texture; their fields all reference m_handle.
        class GLTextureView *m_srView = nullptr;
        class GLTextureView *m_rtView = nullptr;
        class GLTextureView *m_dsView = nullptr;
    };

}

#endif
