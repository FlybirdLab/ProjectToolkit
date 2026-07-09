/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/math/geometry/Rect.hpp"
#include "toolkit/math/geometry/Viewport.hpp"
#include "toolkit/utils/Color.hpp"
#include "utils/PixelFormat.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{

// Get the graphics texture view. Checks if pointer is not null and texture has one or more mip levels loaded.
#define GET_TEXTURE_VIEW_SAFE(t) (t && t->residentMipLevels() > 0 ? t->view() : nullptr)

    // The graphics dispatch indirect command arguments data.
    struct GfxDispatchIndirectArgs
    {
        // The X dimension of dispatch size.
        u32 threadGroupCountX;
        // The Y dimension of dispatch size.
        u32 threadGroupCountY;
        // The Z dimension of dispatch size.
        u32 threadGroupCountZ;
    };

    // The graphics draw indirect command arguments data.
    struct GfxDrawIndirectArgs
    {
        // The number of vertices to draw for each instance.
        u32 verticesCount;
        // The number of instances to draw.
        u32 instanceCount;
        // An offset added to each vertex index.
        u32 startVertex;
        // An offset added to each instance index.
        u32 startInstance;
    };

    // The graphics draw indexed indirect command arguments data.
    struct GfxDrawIndexedIndirectArgs
    {
        // The number of indices to draw for each instance.
        u32 indicesCount;
        // The number of instances to draw.
        u32 instanceCount;
        // An offset into the index buffer where drawing should begin.
        u32 startIndex;
        // An offset added to each vertex index.
        u32 startVertex;
        // An offset added to each instance index.
        u32 startInstance;
    };

    // Interface for graphics device context that can record and send graphics commands to the graphics in a sequence.
    class TK_API GfxContext
    {
    protected:
        GfxContext(GfxDevice *device);

#if TK_DEBUG
        enum class InvalidBindPoint
        {
            SRV,
            UAV,
            DSV,
            RTV
        };

        static void logInvalidResourceUsage(i32 slot, const GfxResourceView *view, InvalidBindPoint bindPoint);
#endif

    public:
        // Get the graphics device.
        TK_FORCE_INLINE GfxDevice *getDevice() const
        {
            return m_device;
        }

    public:
        // Begin new frame and enters commands collecting mode.
        virtual void frameBegin();
        // End the current frame rendering.
        virtual void frameEnd();

#if TK_DEBUG
        // Begin the profile event.
        virtual void eventBegin(const char *name)
        {
        }
        // End the last profile event.
        virtual void eventEnd()
        {
        }
#endif

    public:
        // Get the native pointer to the underlying graphics device context. It's a low-level platform-specific handle.
        virtual void *getNativePtr() const = 0;

    public:
        // Clear texture surface with a color. Supports volumetric textures and texture arrays (including cube textures).
        virtual void clear(GfxTextureView *rt, const Color &color) = 0;
        // Clear depth buffer.
        virtual void clearDepth(GfxTextureView *depthBuffer, float depthValue = 1.0f, u8 stencilValue = 0) = 0;
        // Clear an unordered access buffer with a float value.
        virtual void clearUA(GfxBuffer *buf, const FVec4 &value) = 0;
        // Clear an unordered access buffer with an unsigned value.
        virtual void clearUA(GfxBuffer *buf, const u32 value[4]) = 0;
        // Clear an unordered access texture with an unsigned value.
        virtual void clearUA(GfxTexture *texture, const u32 value[4]) = 0;
        // Clear an unordered access texture with a float value.
        virtual void clearUA(GfxTexture *texture, const FVec4 &value) = 0;

    public:
        // Update the buffer data.
        virtual void updateBuffer(GfxBuffer *buffer, const void *data, u32 size, u32 offset = 0) = 0;
        // Update the texture data.
        virtual void updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch) = 0;

        // Copy the buffer data.
        virtual void copyBuffer(GfxBuffer *dstBuffer, GfxBuffer *srcBuffer, u32 size, u32 dstOffset = 0, u32 srcOffset = 0) = 0;
        // Copy region of the texture.
        virtual void copyTexture(GfxTexture *dstResource, u32 dstSubresource, u32 dstX, u32 dstY, u32 dstZ, GfxTexture *srcResource, u32 srcSubresource) = 0;

        // Reset the counter buffer to zero (hidden by the driver).
        virtual void resetCounter(GfxBuffer *buffer) = 0;

        // Copy the counter buffer value.
        virtual void copyCounter(GfxBuffer *dstBuffer, u32 dstOffset, GfxBuffer *srcBuffer) = 0;
        // Copy the resource data (whole resource).
        virtual void copyResource(GfxResource *dstResource, GfxResource *srcResource) = 0;
        // Copy the subresource data.
        virtual void copySubresource(GfxResource *dstResource, u32 dstSubresource, GfxResource *srcResource, u32 srcSubresource) = 0;

    public:
        // Unbind all the render targets and flushes the change with the driver (used to prevent driver detection of resource hazards, eg. when down-scaling the texture).
        virtual void resetRenderTarget() = 0;

        // Set the render target to the output.
        virtual void setRenderTarget(GfxTextureView *rt) = 0;
        // Set the render target and the depth buffer to the output.
        virtual void setRenderTarget(GfxTextureView *depthBuffer, GfxTextureView *rt) = 0;
        // Set the render targets and the depth buffer to the output.
        virtual void setRenderTarget(GfxTextureView *depthBuffer, const TkSpan<GfxTextureView *> &rts) = 0;
        // Set the blend factor that modulate values for a pixel shader, render target, or both.
        virtual void setBlendFactor(const FVec4 &value) = 0;
        // Set the reference value for depth stencil tests.
        virtual void setStencilRef(u32 value) = 0;

    public:
        // Unbind all shader resource slots and flushes the change with the driver (used to prevent driver detection of resource hazards, eg. when down-scaling the texture).
        virtual void resetSR() = 0;
        // Unbind all unordered access resource slots and flushes the change with the driver (used to prevent driver detection of resource hazards, eg. when down-scaling the texture).
        virtual void resetUA() = 0;
        // Unbind all constant buffer slots and flushes the change with the driver (used to prevent driver detection of resource hazards, eg. when down-scaling the texture).
        virtual void resetCB() = 0;

        // Unbind shader resource slot.
        TK_FORCE_INLINE void unbindSR(i32 slot)
        {
            bindSR(slot, static_cast<GfxResourceView *>(nullptr));
        }
        // Unbind unordered access resource slot.
        TK_FORCE_INLINE void unbindUA(i32 slot)
        {
            bindUA(slot, static_cast<GfxResourceView *>(nullptr));
        }
        // Unbind constant buffer slot.
        TK_FORCE_INLINE void unbindCB(i32 slot)
        {
            bindCB(slot, nullptr);
        }

        // Bind the texture to the shader resource slot.
        void bindSR(i32 slot, GfxTexture *t);
        // Bind the resource view to the shader resource slot (texture view or buffer view).
        virtual void bindSR(i32 slot, GfxResourceView *view) = 0;
        // Bind the resource view to the unordered access slot (texture view or buffer view).
        virtual void bindUA(i32 slot, GfxResourceView *view) = 0;
        // Bind the constant buffer to the slot.
        virtual void bindCB(i32 slot, GfxConstantBuffer *cb) = 0;
        // Bind the vertex buffers to the pipeline.
        virtual void bindVB(const TkSpan<GfxBuffer *> &vertexBuffers, const u32 *vertexBuffersOffsets = nullptr, GfxVertexLayout *vertexLayout = nullptr) = 0;
        // Bind the index buffer to the pipeline.
        virtual void bindIB(GfxBuffer *indexBuffer) = 0;
        // Bind the texture sampler to the pipeline.
        virtual void bindSampler(i32 slot, GfxSampler *sampler) = 0;

    public:
        // Update the constant buffer data.
        virtual void updateCB(GfxConstantBuffer *cb, const void *data) = 0;

    public:
        // Execute a command list from a thread group.
        virtual void dispatch(class GfxShaderProgramCS *shader, u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) = 0;
        // Execute a command list from a thread group. Buffer must contain GfxDispatchIndirectArgs.
        virtual void dispatchIndirect(GfxShaderProgramCS *shader, GfxBuffer *bufferForArgs, u32 offsetForArgs) = 0;

        // Resolve the multisampled texture by performing a copy of the resource into a non-multisampled resource.
        virtual void resolveMultisample(GfxTexture *sourceMultisampleTexture, GfxTexture *destTexture, i32 sourceSubResource = 0, i32 destSubResource = 0, PixelFormat format = PixelFormat::kUnknown) = 0;

        // Draw the specified source texture to destination render target (using fullscreen triangle). Copy contents with resizing and format conversion support. Uses linear texture sampling.
        void draw(GfxTexture *dst, GfxTexture *src);
        // Draw the specified texture to render target (using fullscreen triangle). Copy contents with resizing and format conversion support. Uses linear texture sampling.
        void draw(GfxTexture *rt);
        // Draw the specified texture to render target (using fullscreen triangle). Copy contents with resizing and format conversion support. Uses linear texture sampling.
        void draw(GfxTextureView *rt);
        void drawFullscreenTriangle();

        // Draw non-indexed, non-instanced primitives.
        TK_FORCE_INLINE void draw(i32 startVertex, u32 verticesCount)
        {
            drawInstanced(verticesCount, 1, 0, startVertex);
        }

        // Draw the instanced primitives.
        virtual void drawInstanced(u32 verticesCount, u32 instanceCount, i32 startInstance = 0, i32 startVertex = 0) = 0;
        // Draw the indexed primitives.
        TK_FORCE_INLINE void drawIndexed(u32 indicesCount, i32 startVertex = 0, i32 startIndex = 0)
        {
            drawIndexedInstanced(indicesCount, 1, 0, startVertex, startIndex);
        }

        // Draw the indexed, instanced primitives.
        virtual void drawIndexedInstanced(u32 indicesCount, u32 instanceCount, i32 startInstance = 0, i32 startVertex = 0, i32 startIndex = 0) = 0;
        // Draw the instanced GPU-generated primitives. Buffer must contain GfxDrawIndirectArgs.
        virtual void drawInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs) = 0;
        // Draw the instanced GPU-generated indexed primitives. Buffer must contain GfxDrawIndexedIndirectArgs.
        virtual void drawIndexedInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs) = 0;

    public:
        // Set the rendering viewport and scissor rectangle.
        TK_FORCE_INLINE void setViewportAndScissors(float width, float height)
        {
            const Viewport viewport(0.0f, 0.0f, width, height);
            setViewport(viewport);
            const Rectangle rect(0.0f, 0.0f, width, height);
            setScissor(rect);
        }

        // Set the rendering viewport and scissor rectangle.
        TK_FORCE_INLINE void setViewportAndScissors(const Viewport &viewport)
        {
            setViewport(viewport);
            const Rectangle rect(viewport.loc.x, viewport.loc.y, viewport.w, viewport.h);
            setScissor(rect);
        }

        // Set the rendering viewport.
        TK_FORCE_INLINE void setViewport(float width, float height)
        {
            const Viewport viewport(0.0f, 0.0f, width, height);
            setViewport(viewport);
        }

        // Set the rendering viewport.
        virtual void setViewport(const Viewport &viewport) = 0;
        // Set the scissor rectangle.
        virtual void setScissor(const Rectangle &scissorRect) = 0;

    public:
        // Set the graphics pipeline state.
        virtual void setState(GfxPipelineState *state) = 0;
        // Get the current pipeline state binded to the graphics pipeline.
        virtual GfxPipelineState *getState() const = 0;

        // Clear the context state.
        virtual void clearState() = 0;

        // Flushes the internal cached context state with a command buffer.
        virtual void flushState() = 0;
        // Flushes the command buffer (calls graphics execution).
        virtual void flush() = 0;

        // Set the state of the resource (or subresource).
        virtual void setResourceState(GfxResource *resource, u64 state, i32 subresource = -1);
        // Forces graphics backend to rebind descriptors after command list was used by external graphics library.
        virtual void forceRebindDescriptors();

    protected:
        double m_lastRenderTime = -1;

    private:
        GfxDevice *m_device;
    };

}
