/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx12/common.hpp>

#if GFX_API_DIRECTX12

#include <toolkit/platforms/System.hpp>
#include "Dx12Device.hpp"

namespace toolkit
{
    class DX12CmdSignature;

    class DX12IndirectParameter
    {
        friend DX12CmdSignature;

    public:
        DX12IndirectParameter()
        {
            m_parameter.Type = (D3D12_INDIRECT_ARGUMENT_TYPE)0xFFFFFFFF;
        }

    public:
        void draw()
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
        }

        void drawIndexed()
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
        }

        void dispatch()
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
        }

        void vertexBufferView(UINT slot)
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
            m_parameter.VertexBuffer.Slot = slot;
        }

        void indexBufferView()
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
        }

        void constant(UINT rootParameterIndex, UINT destOffsetIn32BitValues, UINT num32BitValuesToSet)
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
            m_parameter.Constant.RootParameterIndex = rootParameterIndex;
            m_parameter.Constant.DestOffsetIn32BitValues = destOffsetIn32BitValues;
            m_parameter.Constant.Num32BitValuesToSet = num32BitValuesToSet;
        }

        void constantBufferView(UINT rootParameterIndex)
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
            m_parameter.ConstantBufferView.RootParameterIndex = rootParameterIndex;
        }

        void shaderResourceView(UINT rootParameterIndex)
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
            m_parameter.ShaderResourceView.RootParameterIndex = rootParameterIndex;
        }

        void unorderedAccessView(UINT rootParameterIndex)
        {
            m_parameter.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
            m_parameter.UnorderedAccessView.RootParameterIndex = rootParameterIndex;
        }

        const D3D12_INDIRECT_ARGUMENT_DESC &getDesc() const
        {
            return m_parameter;
        }

    protected:
        D3D12_INDIRECT_ARGUMENT_DESC m_parameter;
    };

    class DX12CmdSignature : public DX12Resource<GfxResource>
    {
    public:
        DX12CmdSignature(DX12Device *device, i32 numParams)
            : DX12Resource<GfxResource>(device, "DX12CmdSignature"), m_signature(nullptr)
        {
            m_parameters.resize(numParams);
        }

    public:
        TK_FORCE_INLINE DX12IndirectParameter &at(i32 entryIndex)
        {
            return m_parameters[entryIndex];
        }

        TK_FORCE_INLINE DX12IndirectParameter &operator[](i32 entryIndex)
        {
            return m_parameters[entryIndex];
        }

        TK_FORCE_INLINE const DX12IndirectParameter &operator[](i32 entryIndex) const
        {
            return m_parameters[entryIndex];
        }

        TK_FORCE_INLINE ID3D12CommandSignature *getSignature() const
        {
            return m_signature;
        }

        void finalize(ID3D12RootSignature *rootSignature = nullptr)
        {
            if (m_signature != nullptr)
                return;

            UINT byteStride = 0;
            bool requiresRootSignature = false;
            const i32 numParameters = m_parameters.count();

            for (i32 i = 0; i < numParameters; i++)
            {
                switch (m_parameters[i].getDesc().Type)
                {
                case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW:
                    byteStride += sizeof(D3D12_DRAW_ARGUMENTS);
                    break;
                case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
                    byteStride += sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
                    break;
                case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH:
                    byteStride += sizeof(D3D12_DISPATCH_ARGUMENTS);
                    break;
                case D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT:
                    byteStride += m_parameters[i].getDesc().Constant.Num32BitValuesToSet * 4;
                    requiresRootSignature = true;
                    break;
                case D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW:
                    byteStride += sizeof(D3D12_VERTEX_BUFFER_VIEW);
                    break;
                case D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW:
                    byteStride += sizeof(D3D12_INDEX_BUFFER_VIEW);
                    break;
                case D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW:
                case D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW:
                case D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW:
                    byteStride += 8;
                    requiresRootSignature = true;
                    break;
                default:
                    // TODO
                    break;
                }
            }

            D3D12_COMMAND_SIGNATURE_DESC desc;
            desc.ByteStride = byteStride;
            desc.NumArgumentDescs = numParameters;
            desc.pArgumentDescs = (const D3D12_INDIRECT_ARGUMENT_DESC *)m_parameters.get();
            desc.NodeMask = 1;

            ID3D12RootSignature *rootSig = rootSignature;
            if (requiresRootSignature)
            {
                TK_ASSERT(rootSig != nullptr);
            }
            else
            {
                rootSig = nullptr;
            }

            const auto result = m_device->getDevice()->CreateCommandSignature(&desc, rootSig, IID_PPV_ARGS(&m_signature));
            LOG_DIRECTX_RESULT(result);

            m_signature->SetName(L"CommandSignature");

            m_memoryUsage = 100;
        }

    public:
        GfxResType getType() const override
        {
            return GfxResType::kDescriptor;
        }

    protected:
        void onReleaseData() override
        {
            DX_DEL(m_signature, 0);
            m_parameters.resize(0);
        }

    protected:
        ID3D12CommandSignature *m_signature;
        TkStaticArray<DX12IndirectParameter, 4> m_parameters;
    };

}

#endif
