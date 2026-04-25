/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/PixelFormat.hpp>
#include <toolkit/graphics/GfxResource.hpp>
#include <toolkit/math/Vector2.hpp>
#include <toolkit/graphics/common.hpp>
#include <toolkit/graphics/GfxWindow.hpp>

namespace toolkit
{
    // Graphics swap chain object that provides rendering to native window backbuffer.
    class TK_API GfxSwapChain : public GfxResource
    {
    protected:
        GfxSwapChain();

    public:
        // Get the linked window.
        class TkWindow *getWindow() const
        {
            return m_window;
        }

        // The output backbuffer width (in pixels).
        TK_FORCE_INLINE i32 getWidth() const
        {
            return m_width;
        }

        // The output backbuffer height (in pixels).
        TK_FORCE_INLINE i32 getHeight() const
        {
            return m_height;
        }

        // The output backbuffer surface format.
        TK_FORCE_INLINE PixelFormat getFormat() const
        {
            return m_format;
        }

        // The output backbuffer width and height (in pixels).
        TK_FORCE_INLINE FVec2 getSize() const
        {
            return FVec2(static_cast<float>(m_width), static_cast<float>(m_height));
        }

        // The output backbuffer aspect ratio.
        TK_FORCE_INLINE float getAspectRatio() const
        {
            return static_cast<float>(m_width) / m_height;
        }

        // Get amount of backbuffer swaps.
        TK_FORCE_INLINE u64 getPresentCount() const
        {
            return m_presentCount;
        }

        // True if running in fullscreen mode.
        virtual bool isFullscreen() = 0;
        // Set the fullscreen state.
        virtual void setFullscreen(bool isFullscreen) = 0;
        // Get the view for the output back buffer texture (for the current frame rendering).
        virtual class GfxTextureView *getBackBufferView() = 0;
        // Copies the backbuffer contents to the destination texture.
        virtual void copyBackbuffer(class GfxContext *context, GfxTexture *dst) = 0;

        // Checks if task is ready to render.
        virtual bool isReady() const
        {
            // Skip rendering for the hidden windows
            return getWidth() > 0 && (m_window->isVisible() || m_window->m_state.showAfterFirstPaint);
        }

        // Creates GPU async task that will gather render target data from the GPU.
        virtual class TkTask *downloadDataAsync(class TextureData &result);

    public:
        // Begin task rendering.
        virtual void begin(class RenderTask *task)
        {
        }
        // End task rendering.
        virtual void end(RenderTask *task);
        // Present back buffer to the output.
        virtual void present(bool vsync);
        // Resize output back buffer.
        virtual bool resize(i32 width, i32 height) = 0;

        GfxResType getType() const final override;

    protected:
        i32 m_width = 0;
        i32 m_height = 0;
        u64 m_presentCount = 0;
        PixelFormat m_format = PixelFormat::kUnknown;
        GfxWindow *m_window = nullptr;
        class TkTask *m_downloadTask = nullptr;

    };

}