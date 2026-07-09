/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11Context.hpp"

#if GFX_API_DIRECTX11

#include "Dx11Shader.hpp"
#include "Dx11ShaderProgram.hpp"
#include "Dx11PipelineState.hpp"
#include "Dx11Texture.hpp"
#include "Dx11Buffer.hpp"
#include "Dx11Sampler.hpp"
#include "Dx11VertexLayout.hpp"
#include "Dx11IShaderRes.hpp"
#include "toolkit/graphics/GfxConstantBuffer.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/math/geometry/Viewport.hpp"
#include "toolkit/math/geometry/Rect.hpp"

namespace toolkit
{

#define DX11_CLEAR_SR_ON_STAGE_DISABLE 0

#if DX11_CLEAR_SR_ON_STAGE_DISABLE
    ID3D11ShaderResourceView *EmptySRHandles[GPU_MAX_SR_BINDED] = {};
#endif

    // Ensure to match the indirect commands arguments layout
    static_assert(sizeof(GfxDispatchIndirectArgs) == sizeof(u32) * 3, "Wrong size of GfxDrawIndirectArgs.");
    static_assert(TK_OFFSET_OF(GfxDispatchIndirectArgs, threadGroupCountX) == sizeof(u32) * 0, "Wrong offset for GfxDrawIndirectArgs::ThreadGroupCountX");
    static_assert(TK_OFFSET_OF(GfxDispatchIndirectArgs, threadGroupCountY) == sizeof(u32) * 1, "Wrong offset for GfxDrawIndirectArgs::ThreadGroupCountY");
    static_assert(TK_OFFSET_OF(GfxDispatchIndirectArgs, threadGroupCountZ) == sizeof(u32) * 2, "Wrong offset for GfxDrawIndirectArgs::ThreadGroupCountZ");
    //
    static_assert(sizeof(GfxDrawIndirectArgs) == sizeof(D3D11_DRAW_INSTANCED_INDIRECT_ARGS), "Wrong size of GfxDrawIndirectArgs.");
    static_assert(TK_OFFSET_OF(GfxDrawIndirectArgs, verticesCount) == TK_OFFSET_OF(D3D11_DRAW_INSTANCED_INDIRECT_ARGS, VertexCountPerInstance), "Wrong offset for GfxDrawIndirectArgs::VerticesCount");
    static_assert(TK_OFFSET_OF(GfxDrawIndirectArgs, instanceCount) == TK_OFFSET_OF(D3D11_DRAW_INSTANCED_INDIRECT_ARGS, InstanceCount), "Wrong offset for GfxDrawIndirectArgs::InstanceCount");
    static_assert(TK_OFFSET_OF(GfxDrawIndirectArgs, startVertex) == TK_OFFSET_OF(D3D11_DRAW_INSTANCED_INDIRECT_ARGS, StartVertexLocation), "Wrong offset for GfxDrawIndirectArgs::StartVertex");
    static_assert(TK_OFFSET_OF(GfxDrawIndirectArgs, startInstance) == TK_OFFSET_OF(D3D11_DRAW_INSTANCED_INDIRECT_ARGS, StartInstanceLocation), "Wrong offset for GfxDrawIndirectArgs::StartInstance");
    //
    static_assert(sizeof(GfxDrawIndexedIndirectArgs) == sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS), "Wrong size of GfxDrawIndexedIndirectArgs.");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, indicesCount) == TK_OFFSET_OF(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS, IndexCountPerInstance), "Wrong offset for GfxDrawIndexedIndirectArgs::IndicesCount");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, instanceCount) == TK_OFFSET_OF(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS, InstanceCount), "Wrong offset for GfxDrawIndexedIndirectArgs::InstanceCount");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, startIndex) == TK_OFFSET_OF(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS, StartIndexLocation), "Wrong offset for GfxDrawIndexedIndirectArgs::StartIndex");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, startVertex) == TK_OFFSET_OF(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS, BaseVertexLocation), "Wrong offset for GfxDrawIndexedIndirectArgs::StartVertex");
    static_assert(TK_OFFSET_OF(GfxDrawIndexedIndirectArgs, startInstance) == TK_OFFSET_OF(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS, StartInstanceLocation), "Wrong offset for GfxDrawIndexedIndirectArgs::StartInstance");

    DX11Context::DX11Context(DX11Device *device, ID3D11DeviceContext *context)
        : GfxContext(device), m_device(device), m_context(context),
#if GFX_ALLOW_PROFILE_EVENTS
          m_userDefinedAnnotations(nullptr),
#endif
          m_omDirtyFlag(false), m_rtCount(0), m_rtDepth(nullptr), m_srMaskDirtyGraphics(0), m_srMaskDirtyCompute(0), m_uaDirtyFlag(false), m_cbDirtyFlag(false), m_ibHandle(nullptr), m_vertexLayout(nullptr), m_iaInputLayoutDirtyFlag(false), m_currentState(nullptr), currentBlendState(nullptr), currentRasterizerState(nullptr), currentDepthStencilState(nullptr), currentVS(nullptr),
#if GFX_ALLOW_TESSELLATION_SHADERS
          currentHS(nullptr), currentDS(nullptr),
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
          currentGS(nullptr),
#endif
          currentPS(nullptr), currentCS(nullptr), currentPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED), currentStencilRef(0), currentBlendFactor(FVec4::kOne)
    {
        TK_ASSERT(m_context);
#if GFX_ALLOW_PROFILE_EVENTS
        m_context->QueryInterface(IID_PPV_ARGS(&m_userDefinedAnnotations));
#endif

        m_maxUASlots = GFX_MAX_UA_BINDED;
        if (m_device->getBackendType() != GfxBackendType::kDirectX11)
            m_maxUASlots = 1;

        TkMemory::memoryClear(m_rtHandles, sizeof(m_rtHandles));
        TkMemory::memoryClear(m_srHandles, sizeof(m_srHandles));
        TkMemory::memoryClear(m_uaHandles, sizeof(m_uaHandles));
        TkMemory::memoryClear(m_cbHandles, sizeof(m_cbHandles));
        TkMemory::memoryClear(m_vbHandles, sizeof(m_vbHandles));
        TkMemory::memoryClear(m_vbStrides, sizeof(m_vbStrides));
        TkMemory::memoryClear(m_vbOffsets, sizeof(m_vbOffsets));
    }

    DX11Context::~DX11Context()
    {
#if GFX_ALLOW_PROFILE_EVENTS
        DX_DEL(m_userDefinedAnnotations, 1);
#endif
    }

    void DX11Context::frameBegin()
    {
        GfxContext::frameBegin();

        m_omDirtyFlag = false;
        m_uaDirtyFlag = false;
        m_cbDirtyFlag = false;
        m_iaInputLayoutDirtyFlag = false;
        m_srMaskDirtyGraphics = 0;
        m_srMaskDirtyCompute = 0;
        m_rtCount = 0;
        m_vertexLayout = nullptr;
        m_currentState = nullptr;
        m_rtDepth = nullptr;
        TkMemory::memoryClear(m_rtHandles, sizeof(m_rtHandles));
        TkMemory::memoryClear(m_srHandles, sizeof(m_srHandles));
        TkMemory::memoryClear(m_uaHandles, sizeof(m_uaHandles));
        TkMemory::memoryClear(m_cbHandles, sizeof(m_cbHandles));
        TkMemory::memoryClear(m_vbHandles, sizeof(m_vbHandles));
        TkMemory::memoryClear(m_vbStrides, sizeof(m_vbStrides));
        TkMemory::memoryClear(m_vbOffsets, sizeof(m_vbOffsets));
        m_ibHandle = nullptr;
        currentBlendState = nullptr;
        currentRasterizerState = nullptr;
        currentDepthStencilState = nullptr;
        currentVS = nullptr;
#if GFX_ALLOW_TESSELLATION_SHADERS
        currentHS = nullptr;
        currentDS = nullptr;
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        currentGS = nullptr;
#endif
        currentPS = nullptr;
        currentCS = nullptr;
        currentPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        currentStencilRef = 0;
        currentBlendFactor = FVec4::kOne;

        // Bind dummy vertex buffer (used by missing bindings)
        auto dummyVB = (DX11Buffer *)m_device->getDummyVB();
        ID3D11Buffer *dummyVBBuffer = dummyVB->getBuffer();
        UINT stride = dummyVB->getStride(), offset = 0;
        m_context->IASetVertexBuffers(GFX_MAX_VB_BINDED, 1, &dummyVBBuffer, &stride, &offset);

        // Bind static samplers
        ID3D11SamplerState *samplers[] =
            {
                m_device->m_samplerLinearClamp,
                m_device->m_samplerPointClamp,
                m_device->m_samplerLinearWrap,
                m_device->m_samplerPointWrap,
                m_device->m_samplerShadow,
                m_device->m_samplerShadowLinear};
        const UINT samplersCount = sizeof(samplers) / sizeof(samplers[0]);
        m_context->VSSetSamplers(0, samplersCount, samplers);
#if GFX_ALLOW_TESSELLATION_SHADERS
        m_context->DSSetSamplers(0, samplersCount, samplers);
#endif
        m_context->PSSetSamplers(0, samplersCount, samplers);
        m_context->CSSetSamplers(0, samplersCount, samplers);
    }

#if GFX_ALLOW_PROFILE_EVENTS
    void DX11Context::eventBegin(const char *name)
    {
        if (m_userDefinedAnnotations)
        {
            wchar_t buffer[256];
            i32 len = 0;
            while (name[len] && len < 255)
            {
                buffer[len] = (wchar_t)name[len];
                len++;
            }
            buffer[len] = 0;
            m_userDefinedAnnotations->BeginEvent(buffer);
        }
    }

    void DX11Context::eventEnd()
    {
        if (m_userDefinedAnnotations)
            m_userDefinedAnnotations->EndEvent();
    }
#endif

    void *DX11Context::getNativePtr() const
    {
        return (void *)m_context;
    }

    void DX11Context::clear(GfxTextureView *rt, const Color &color)
    {
        auto rtDX11 = static_cast<DX11TextureView *>(rt);
        if (rtDX11)
        {
            m_context->ClearRenderTargetView(rtDX11->rtv(), color.raw);
        }
    }

    void DX11Context::clearDepth(GfxTextureView *depthBuffer, float depthValue, u8 stencilValue)
    {
        auto depthBufferDX11 = static_cast<DX11TextureView *>(depthBuffer);
        if (depthBufferDX11)
        {
            TK_ASSERT(depthBufferDX11->dsv());
            m_context->ClearDepthStencilView(depthBufferDX11->dsv(), D3D11_CLEAR_DEPTH, depthValue, stencilValue);
        }
    }

    void DX11Context::clearUA(GfxBuffer *buf, const FVec4 &value)
    {
        TK_ASSERT(buf != nullptr && buf->isUnorderedAccess());
        auto uav = ((DX11BufferView *)buf->view())->uav();
        m_context->ClearUnorderedAccessViewFloat(uav, value.raw);
    }

    void DX11Context::clearUA(GfxBuffer *buf, const u32 value[4])
    {
        TK_ASSERT(buf != nullptr && buf->isUnorderedAccess());
        auto uav = ((DX11BufferView *)buf->view())->uav();
        m_context->ClearUnorderedAccessViewUint(uav, value);
    }

    void DX11Context::clearUA(GfxTexture *texture, const u32 value[4])
    {
        TK_ASSERT(texture != nullptr && texture->isUnorderedAccess());
        auto uav = ((DX11TextureView *)texture->view())->uav();
        m_context->ClearUnorderedAccessViewUint(uav, value);
    }

    void DX11Context::clearUA(GfxTexture *texture, const FVec4 &value)
    {
        TK_ASSERT(texture != nullptr && texture->isUnorderedAccess());
        auto uav = ((DX11TextureView *)(texture->isVolume() ? texture->viewVolume() : texture->view()))->uav();
        m_context->ClearUnorderedAccessViewFloat(uav, value.raw);
    }

    void DX11Context::resetRenderTarget()
    {
        if (m_rtCount != 0 || m_rtDepth)
        {
            m_omDirtyFlag = true;
            m_rtCount = 0;
            m_rtDepth = nullptr;

            TkMemory::memoryClear(m_rtHandles, sizeof(m_rtHandles));

            flushOM();
        }
    }

    void DX11Context::setRenderTarget(GfxTextureView *rt)
    {
        auto rtDX11 = reinterpret_cast<DX11TextureView *>(rt);

        ID3D11RenderTargetView *rtv = rtDX11 ? rtDX11->rtv() : nullptr;
        i32 newRtCount = rtv ? 1 : 0;

        if (m_rtCount != newRtCount || m_rtHandles[0] != rtv || m_rtDepth != nullptr)
        {
            m_omDirtyFlag = true;
            m_rtCount = newRtCount;
            m_rtDepth = nullptr;
            m_rtHandles[0] = rtv;
        }
    }

    void DX11Context::setRenderTarget(GfxTextureView *depthBuffer, GfxTextureView *rt)
    {
        auto rtDX11 = reinterpret_cast<DX11TextureView *>(rt);
        auto depthBufferDX11 = static_cast<DX11TextureView *>(depthBuffer);

        ID3D11RenderTargetView *rtv = rtDX11 ? rtDX11->rtv() : nullptr;
        ID3D11DepthStencilView *dsv = depthBufferDX11 ? depthBufferDX11->dsv() : nullptr;
        i32 newRtCount = rtv ? 1 : 0;

        if (m_rtCount != newRtCount || m_rtHandles[0] != rtv || m_rtDepth != dsv)
        {
            m_omDirtyFlag = true;
            m_rtCount = newRtCount;
            m_rtDepth = dsv;
            m_rtHandles[0] = rtv;
        }
    }

    void DX11Context::setRenderTarget(GfxTextureView *depthBuffer, const TkSpan<GfxTextureView *> &rts)
    {
        TK_ASSERT(rts.length() >= 1 && rts.length() <= GFX_MAX_RT_BINDED);

        auto depthBufferDX11 = static_cast<DX11TextureView *>(depthBuffer);
        ID3D11DepthStencilView *dsv = depthBufferDX11 ? depthBufferDX11->dsv() : nullptr;

        ID3D11RenderTargetView *rtvs[GFX_MAX_RT_BINDED];
        for (i32 i = 0; i < rts.length(); i++)
        {
            auto rtDX11 = reinterpret_cast<DX11TextureView *>(rts[i]);
            rtvs[i] = rtDX11 ? rtDX11->rtv() : nullptr;
        }
        i32 rtvsSize = sizeof(ID3D11RenderTargetView *) * rts.length();

        if (m_rtCount != rts.length() || m_rtDepth != dsv || TkMemory::memoryCompare(m_rtHandles, rtvs, rtvsSize) != 0)
        {
            m_omDirtyFlag = true;
            m_rtCount = rts.length();
            m_rtDepth = dsv;
            TkMemory::memoryCopy(m_rtHandles, rtvs, rtvsSize);
        }
    }

    void DX11Context::setBlendFactor(const FVec4 &value)
    {
        currentBlendFactor = value;
        if (currentBlendState)
            m_context->OMSetBlendState(currentBlendState, currentBlendFactor.raw, D3D11_DEFAULT_SAMPLE_MASK);
    }

    void DX11Context::setStencilRef(u32 value)
    {
        if (currentStencilRef != value)
        {
            currentStencilRef = value;
            m_context->OMSetDepthStencilState(currentDepthStencilState, currentStencilRef);
        }
    }

    void DX11Context::resetSR()
    {
        m_srMaskDirtyGraphics = kMaxU32;
        m_srMaskDirtyCompute = kMaxU32;
        TkMemory::memoryClear(m_srHandles, sizeof(m_srHandles));

        const UINT count = GFX_MAX_SR_BINDED;
        m_context->VSSetShaderResources(0, count, m_srHandles);
#if GFX_ALLOW_TESSELLATION_SHADERS
        m_context->HSSetShaderResources(0, count, m_srHandles);
        m_context->DSSetShaderResources(0, count, m_srHandles);
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        m_context->GSSetShaderResources(0, count, m_srHandles);
#endif
        m_context->PSSetShaderResources(0, count, m_srHandles);
        m_context->CSSetShaderResources(0, count, m_srHandles);
    }

    void DX11Context::resetUA()
    {
        m_uaDirtyFlag = false;
        TkMemory::memoryClear(m_uaHandles, sizeof(m_uaHandles));

        m_context->CSSetUnorderedAccessViews(0, m_maxUASlots, m_uaHandles, nullptr);
        m_context->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, 0, nullptr, nullptr);
    }

    void DX11Context::resetCB()
    {
        m_cbDirtyFlag = false;
        TkMemory::memoryClear(m_cbHandles, sizeof(m_cbHandles));

        const UINT count = GFX_MAX_CB_BINDED;
        m_context->VSSetConstantBuffers(0, count, m_cbHandles);
#if GFX_ALLOW_TESSELLATION_SHADERS
        m_context->HSSetConstantBuffers(0, count, m_cbHandles);
        m_context->DSSetConstantBuffers(0, count, m_cbHandles);
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        m_context->GSSetConstantBuffers(0, count, m_cbHandles);
#endif
        m_context->PSSetConstantBuffers(0, count, m_cbHandles);
        m_context->CSSetConstantBuffers(0, count, m_cbHandles);
    }

    void DX11Context::bindCB(i32 slot, GfxConstantBuffer *cb)
    {
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_CB_BINDED);

        ID3D11Buffer *buffer = nullptr;
        if (cb && cb->getSize() > 0)
        {
            auto cbDX11 = static_cast<DX11ConstantBuffer *>(cb);
            buffer = cbDX11->getBuffer();
        }

        if (m_cbHandles[slot] != buffer)
        {
            m_cbDirtyFlag = true;
            m_cbHandles[slot] = buffer;
        }
    }

    void DX11Context::bindSR(i32 slot, GfxResourceView *view)
    {
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_SR_BINDED);
        auto handle = view ? ((IShaderResourceDX11 *)view->getNativePtr())->srv() : nullptr;
        if (m_srHandles[slot] != handle)
        {
            m_srMaskDirtyGraphics |= 1 << slot;
            m_srMaskDirtyCompute |= 1 << slot;
            m_srHandles[slot] = handle;
            if (view)
                *view->lastRenderTime = m_lastRenderTime;
        }
    }

    void DX11Context::bindUA(i32 slot, GfxResourceView *view)
    {
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_UA_BINDED);
        auto handle = view ? ((IShaderResourceDX11 *)view->getNativePtr())->uav() : nullptr;
        if (m_uaHandles[slot] != handle)
        {
            m_uaDirtyFlag = true;
            m_uaHandles[slot] = handle;
            if (view)
                *view->lastRenderTime = m_lastRenderTime;
        }
    }

    void DX11Context::bindVB(const TkSpan<GfxBuffer *> &vertexBuffers, const u32 *vertexBuffersOffsets, GfxVertexLayout *vertexLayout)
    {
        TK_ASSERT(vertexBuffers.length() >= 0 && vertexBuffers.length() <= GFX_MAX_VB_BINDED);

        bool vbEdited = false;
        for (i32 i = 0; i < vertexBuffers.length(); i++)
        {
            const auto vbDX11 = static_cast<DX11Buffer *>(vertexBuffers[i]);
            const auto vb = vbDX11 ? vbDX11->getBuffer() : nullptr;
            vbEdited |= vb != m_vbHandles[i];
            m_vbHandles[i] = vb;
            const UINT stride = vbDX11 ? vbDX11->getStride() : 0;
            vbEdited |= stride != m_vbStrides[i];
            m_vbStrides[i] = stride;
            const UINT offset = vertexBuffersOffsets ? vertexBuffersOffsets[i] : 0;
            vbEdited |= offset != m_vbOffsets[i];
            m_vbOffsets[i] = offset;
        }
        if (vbEdited)
        {
            m_context->IASetVertexBuffers(0, vertexBuffers.length(), m_vbHandles, m_vbStrides, m_vbOffsets);
        }
        if (!vertexLayout)
            vertexLayout = GfxVertexLayout::get(vertexBuffers);
        if (m_vertexLayout != vertexLayout)
        {
            m_vertexLayout = (DX11VertexLayout *)vertexLayout;
            m_iaInputLayoutDirtyFlag = true;
        }
    }

    void DX11Context::bindIB(GfxBuffer *indexBuffer)
    {
        const auto ibDX11 = static_cast<DX11Buffer *>(indexBuffer);
        if (ibDX11 != m_ibHandle)
        {
            m_ibHandle = ibDX11;
            m_context->IASetIndexBuffer(ibDX11->getBuffer(), DxUtils::toDxgiFormat(ibDX11->getFormat()), 0);
        }
    }

    void DX11Context::bindSampler(i32 slot, GfxSampler *sampler)
    {
        const auto samplerDX11 = sampler ? static_cast<DX11Sampler *>(sampler)->samplerState : nullptr;
        m_context->VSSetSamplers(slot, 1, &samplerDX11);
#if GFX_ALLOW_TESSELLATION_SHADERS
        m_context->DSSetSamplers(slot, 1, &samplerDX11);
#endif
        m_context->PSSetSamplers(slot, 1, &samplerDX11);
        m_context->CSSetSamplers(slot, 1, &samplerDX11);
    }

    void DX11Context::updateCB(GfxConstantBuffer *cb, const void *data)
    {
        TK_ASSERT(data && cb);
        auto cbDX11 = static_cast<DX11ConstantBuffer *>(cb);
        const u32 size = cbDX11->getSize();
        if (size == 0)
            return;

        m_context->UpdateSubresource(cbDX11->getBuffer(), 0, nullptr, data, size, 1);
    }

    void DX11Context::dispatch(GfxShaderProgramCS *shader, u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ)
    {
        currentCS = (DX11ShaderProgramCS *)shader;

        flushCBs();
        flushSRVs();
        flushUAVs();
        flushOM();

        m_context->CSSetShader((ID3D11ComputeShader *)shader->getBufferHandle(), nullptr, 0);
        m_context->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

        currentCS = nullptr;
    }

    void DX11Context::dispatchIndirect(GfxShaderProgramCS *shader, GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        TK_ASSERT(bufferForArgs && (bufferForArgs->getFlags() & GfxBufferFlags::kArgument) != GfxBufferFlags::kNone);
        currentCS = (DX11ShaderProgramCS *)shader;

        auto bufferForArgsDX11 = (DX11Buffer *)bufferForArgs;

        flushCBs();
        flushSRVs();
        flushUAVs();
        flushOM();

        m_context->CSSetShader((ID3D11ComputeShader *)shader->getBufferHandle(), nullptr, 0);
        m_context->DispatchIndirect(bufferForArgsDX11->getBuffer(), offsetForArgs);

        currentCS = nullptr;
    }

    void DX11Context::resolveMultisample(GfxTexture *sourceMultisampleTexture, GfxTexture *destTexture, i32 sourceSubResource, i32 destSubResource, PixelFormat format)
    {
        TK_ASSERT(sourceMultisampleTexture && sourceMultisampleTexture->isMultiSample());
        TK_ASSERT(destTexture && !destTexture->isMultiSample());

        auto dstResourceDX11 = static_cast<DX11Texture *>(destTexture);
        auto srcResourceDX11 = static_cast<DX11Texture *>(sourceMultisampleTexture);

        const auto formatDXGI = DxUtils::toDxgiFormat(format == PixelFormat::kUnknown ? destTexture->format() : format);
        m_context->ResolveSubresource(dstResourceDX11->getResource(), destSubResource, srcResourceDX11->getResource(), destSubResource, formatDXGI);
    }

    void DX11Context::drawInstanced(u32 verticesCount, u32 instanceCount, i32 startInstance, i32 startVertex)
    {
        onDrawCall();
        m_context->DrawInstanced(verticesCount, instanceCount, startVertex, startInstance);
    }

    void DX11Context::drawIndexedInstanced(u32 indicesCount, u32 instanceCount, i32 startInstance, i32 startVertex, i32 startIndex)
    {
        onDrawCall();
        m_context->DrawIndexedInstanced(indicesCount, instanceCount, startIndex, startVertex, startInstance);
    }

    void DX11Context::drawInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        TK_ASSERT(bufferForArgs && (bufferForArgs->getFlags() & GfxBufferFlags::kArgument) != GfxBufferFlags::kNone);

        const auto bufferForArgsDX11 = static_cast<DX11Buffer *>(bufferForArgs);

        onDrawCall();
        m_context->DrawInstancedIndirect(bufferForArgsDX11->getBuffer(), offsetForArgs);
    }

    void DX11Context::drawIndexedInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        TK_ASSERT(bufferForArgs && (bufferForArgs->getFlags() & GfxBufferFlags::kArgument) != GfxBufferFlags::kNone);

        const auto bufferForArgsDX11 = static_cast<DX11Buffer *>(bufferForArgs);

        onDrawCall();
        m_context->DrawIndexedInstancedIndirect(bufferForArgsDX11->getBuffer(), offsetForArgs);
    }

    void DX11Context::setViewport(const Viewport &viewport)
    {
        m_context->RSSetViewports(1, (D3D11_VIEWPORT *)&viewport);
    }

    void DX11Context::setScissor(const Rectangle &scissorRect)
    {
        D3D11_RECT rect;
        rect.left = (LONG)scissorRect.getLeft();
        rect.right = (LONG)scissorRect.getRight();
        rect.top = (LONG)scissorRect.getTop();
        rect.bottom = (LONG)scissorRect.getBottom();
        m_context->RSSetScissorRects(1, &rect);
    }

    GfxPipelineState *DX11Context::getState() const
    {
        return m_currentState;
    }

    void DX11Context::setState(GfxPipelineState *state)
    {
        if (m_currentState != state)
        {
            m_currentState = static_cast<DX11PipelineState *>(state);

            ID3D11BlendState *blendState = nullptr;
            ID3D11RasterizerState *rasterizerState = nullptr;
            ID3D11DepthStencilState *depthStencilState = nullptr;
            DX11ShaderProgramVS *vs = nullptr;
#if GFX_ALLOW_TESSELLATION_SHADERS
            DX11ShaderProgramHS *hs = nullptr;
            DX11ShaderProgramDS *ds = nullptr;
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
            DX11ShaderProgramGS *gs = nullptr;
#endif
            DX11ShaderProgramPS *ps = nullptr;
            D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            if (state)
            {
                TK_ASSERT(m_currentState->isValid());
                blendState = m_currentState->blendState;
                rasterizerState = m_device->rasterizerStates[m_currentState->rasterizerStateIndex];
                depthStencilState = m_currentState->depthStencilState;
                TK_ASSERT(m_currentState->vs != nullptr);
                vs = m_currentState->vs;
#if GFX_ALLOW_TESSELLATION_SHADERS
                hs = m_currentState->hs;
                ds = m_currentState->ds;
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
                gs = m_currentState->gs;
#endif
                ps = m_currentState->ps;
                primitiveTopology = m_currentState->primitiveTopology;
            }

            bool shaderEnabled = false;
            if (currentDepthStencilState != depthStencilState)
            {
                currentDepthStencilState = depthStencilState;
                m_context->OMSetDepthStencilState(depthStencilState, currentStencilRef);
            }
            if (currentRasterizerState != rasterizerState)
            {
                currentRasterizerState = rasterizerState;
                m_context->RSSetState(rasterizerState);
            }
            if (currentBlendState != blendState)
            {
                currentBlendState = blendState;
                m_context->OMSetBlendState(blendState, currentBlendFactor.raw, D3D11_DEFAULT_SAMPLE_MASK);
            }
            if (currentVS != vs)
            {
                shaderEnabled |= currentVS == nullptr;
                currentVS = vs;
                m_context->VSSetShader(vs ? vs->getBufferHandleDX11() : nullptr, nullptr, 0);
                m_iaInputLayoutDirtyFlag = true;
            }
#if GFX_ALLOW_TESSELLATION_SHADERS
            if (currentHS != hs)
            {
                shaderEnabled |= currentHS == nullptr;
                currentHS = hs;
                m_context->HSSetShader(hs ? hs->getBufferHandleDX11() : nullptr, nullptr, 0);
            }
            if (currentDS != ds)
            {
                shaderEnabled |= currentDS == nullptr;
                currentDS = ds;
                m_context->DSSetShader(ds ? ds->getBufferHandleDX11() : nullptr, nullptr, 0);
            }
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
            if (currentGS != gs)
            {
                shaderEnabled |= currentGS == nullptr;
                currentGS = gs;
                m_context->GSSetShader(gs ? gs->getBufferHandleDX11() : nullptr, nullptr, 0);
            }
#endif
            if (currentPS != ps)
            {
                shaderEnabled |= currentPS == nullptr;
                currentPS = ps;
                m_context->PSSetShader(ps ? ps->getBufferHandleDX11() : nullptr, nullptr, 0);
            }
            if (currentPrimitiveTopology != primitiveTopology)
            {
                currentPrimitiveTopology = primitiveTopology;
                m_context->IASetPrimitiveTopology(primitiveTopology);
            }
            if (shaderEnabled)
            {
                m_cbDirtyFlag = true;
                m_srMaskDirtyGraphics = kMaxU32;
            }
        }
    }

    void DX11Context::clearState()
    {
        if (!m_context)
            return;

        resetRenderTarget();
        resetSR();
        resetUA();
        resetCB();
        setState(nullptr);

        flushState();
    }

    void DX11Context::flushState()
    {
        flushCBs();
        flushSRVs();
        flushUAVs();
        flushIA();
        flushOM();
    }

    void DX11Context::flush()
    {
        if (m_context)
            m_context->Flush();
    }

    void DX11Context::updateBuffer(GfxBuffer *buffer, const void *data, u32 size, u32 offset)
    {
        TK_ASSERT(data);
        TK_ASSERT(buffer && buffer->getSize() >= size);

        auto bufferDX11 = (DX11Buffer *)buffer;

        if (buffer->isDynamic())
        {
            D3D11_MAPPED_SUBRESOURCE map;
            const HRESULT result = m_context->Map(bufferDX11->getResource(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
            if (FAILED(result))
            {
                LOG_DIRECTX_RESULT(result);
                return;
            }
            TkMemory::memoryCopy((byte *)map.pData + offset, data, size);
            m_context->Unmap(bufferDX11->getResource(), 0);
        }
        else
        {
            D3D11_BOX box;
            box.left = offset;
            box.right = offset + size;
            box.front = 0;
            box.back = 1;
            box.top = 0;
            box.bottom = 1;
            m_context->UpdateSubresource(bufferDX11->getResource(), 0, &box, data, size, 0);
        }
    }

    void DX11Context::copyBuffer(GfxBuffer *dstBuffer, GfxBuffer *srcBuffer, u32 size, u32 dstOffset, u32 srcOffset)
    {
        TK_ASSERT(dstBuffer && srcBuffer);

        auto dstBufferDX11 = static_cast<DX11Buffer *>(dstBuffer);
        auto srcBufferDX11 = static_cast<DX11Buffer *>(srcBuffer);

        D3D11_BOX box;
        box.left = srcOffset;
        box.right = srcOffset + size;
        box.top = 0;
        box.bottom = 1;
        box.front = 0;
        box.back = 1;
        m_context->CopySubresourceRegion(dstBufferDX11->getResource(), 0, dstOffset, 0, 0, srcBufferDX11->getResource(), 0, &box);
    }

    void DX11Context::updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch)
    {
        TK_ASSERT(texture && texture->isAllocated() && data);

        auto textureDX11 = static_cast<DX11Texture *>(texture);

        const i32 subresourceIndex = DxUtils::calcSubresourceIndex(mipIndex, arrayIndex, texture->mipLevels());
        u32 depthPitch = slicePitch;
        if (texture->isVolume())
            depthPitch /= TkMath::max(1, texture->depth() >> mipIndex);
        m_context->UpdateSubresource(textureDX11->getResource(), subresourceIndex, nullptr, data, (UINT)rowPitch, (UINT)depthPitch);
    }

    void DX11Context::copyTexture(GfxTexture *dstResource, u32 dstSubresource, u32 dstX, u32 dstY, u32 dstZ, GfxTexture *srcResource, u32 srcSubresource)
    {
        TK_ASSERT(dstResource && srcResource);

        auto dstResourceDX11 = static_cast<DX11Texture *>(dstResource);
        auto srcResourceDX11 = static_cast<DX11Texture *>(srcResource);

        m_context->CopySubresourceRegion(dstResourceDX11->getResource(), dstSubresource, dstX, dstY, dstZ, srcResourceDX11->getResource(), srcSubresource, nullptr);
    }

    void DX11Context::resetCounter(GfxBuffer *buffer)
    {
    }

    void DX11Context::copyCounter(GfxBuffer *dstBuffer, u32 dstOffset, GfxBuffer *srcBuffer)
    {
        TK_ASSERT(dstBuffer && srcBuffer);

        auto dstBufferDX11 = static_cast<DX11Buffer *>(dstBuffer);
        auto srvUavView = ((DX11BufferView *)srcBuffer->view())->uav();

        m_context->CopyStructureCount(dstBufferDX11->getBuffer(), dstOffset, srvUavView);
    }

    void DX11Context::copyResource(GfxResource *dstResource, GfxResource *srcResource)
    {
        TK_ASSERT(dstResource && srcResource);

        auto dstResourceDX11 = dynamic_cast<IDX11Resource *>(dstResource);
        auto srcResourceDX11 = dynamic_cast<IDX11Resource *>(srcResource);

        m_context->CopyResource(dstResourceDX11->getResource(), srcResourceDX11->getResource());
    }

    void DX11Context::copySubresource(GfxResource *dstResource, u32 dstSubresource, GfxResource *srcResource, u32 srcSubresource)
    {
        TK_ASSERT(dstResource && srcResource);

        auto dstResourceDX11 = dynamic_cast<IDX11Resource *>(dstResource);
        auto srcResourceDX11 = dynamic_cast<IDX11Resource *>(srcResource);

        m_context->CopySubresourceRegion(dstResourceDX11->getResource(), dstSubresource, 0, 0, 0, srcResourceDX11->getResource(), srcSubresource, nullptr);
    }

    void DX11Context::flushSRVs()
    {
#define FLUSH_STAGE(STAGE) \
    if (current##STAGE)    \
    m_context->STAGE##SetShaderResources(0, GFX_MAX_SR_BINDED, m_srHandles)
        if (currentCS)
        {
            if (m_srMaskDirtyCompute)
            {
                m_srMaskDirtyCompute = 0;
                FLUSH_STAGE(CS);
            }
        }
        else
        {
            if (m_srMaskDirtyGraphics)
            {
                m_srMaskDirtyGraphics = 0;
                FLUSH_STAGE(VS);
#if GFX_ALLOW_TESSELLATION_SHADERS
                FLUSH_STAGE(HS);
                FLUSH_STAGE(DS);
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
                FLUSH_STAGE(GS);
#endif
                FLUSH_STAGE(PS);
            }
        }
#undef FLUSH_STAGE
    }

    void DX11Context::flushUAVs()
    {
        if (m_uaDirtyFlag)
        {
            m_uaDirtyFlag = false;

            u32 initialCounts[GFX_MAX_UA_BINDED] = {0};
            if (currentCS)
                m_context->CSSetUnorderedAccessViews(0, m_maxUASlots, m_uaHandles, initialCounts);
            else
                m_context->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, m_rtCount, m_maxUASlots - m_rtCount, m_uaHandles + m_rtCount, initialCounts);
        }
    }

    void DX11Context::flushCBs()
    {
        if (m_cbDirtyFlag)
        {
            m_cbDirtyFlag = false;

#define FLUSH_STAGE(STAGE) \
    if (current##STAGE)    \
    m_context->STAGE##SetConstantBuffers(0, GFX_MAX_CB_BINDED, m_cbHandles)
            FLUSH_STAGE(VS);
#if GFX_ALLOW_TESSELLATION_SHADERS
            FLUSH_STAGE(HS);
            FLUSH_STAGE(DS);
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
            FLUSH_STAGE(GS);
#endif
            FLUSH_STAGE(PS);
            FLUSH_STAGE(CS);
#undef FLUSH_STAGE
        }
    }

    void DX11Context::flushOM()
    {
        if (m_omDirtyFlag)
        {
            m_omDirtyFlag = false;
            i32 uaCount = 0;
            for (i32 i = m_maxUASlots - 1; i >= 0; i--)
            {
                if (m_uaHandles[i])
                {
                    uaCount = i + 1;
                    break;
                }
            }

            if (uaCount > 0)
                m_context->OMSetRenderTargetsAndUnorderedAccessViews(m_rtCount, m_rtHandles, m_rtDepth, 0, D3D11_KEEP_UNORDERED_ACCESS_VIEWS, nullptr, nullptr);
            else
                m_context->OMSetRenderTargets(m_rtCount, m_rtHandles, m_rtDepth);
        }
    }

    void DX11Context::flushIA()
    {
        if (m_iaInputLayoutDirtyFlag)
        {
            m_iaInputLayoutDirtyFlag = false;
            ID3D11InputLayout *inputLayout = currentVS ? currentVS->getInputLayout(m_vertexLayout) : nullptr;
            m_context->IASetInputLayout(inputLayout);
        }
    }

    void DX11Context::onDrawCall()
    {
        flushCBs();
        flushSRVs();
        flushUAVs();
        flushIA();
        flushOM();
    }

}

#endif
