/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ShaderCompilerD3D.hpp"
#include "sdhc/Compiler.hpp"
#include "sdhc/Context.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"
#include "toolkit/platforms/win32/ComPointer.hpp"
#include <d3dcompiler.h>

namespace toolkit
{
    // Helper class to include source for D3D shaders compiler.
    class IncludeD3D : public ID3DInclude
    {
    private:
        ShaderCompilationContext *m_context;

    public:
        IncludeD3D(ShaderCompilationContext *context)
        {
            m_context = context;
        }

        HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override
        {
            const char *source;
            i32 sourceLength;
            const TkString filename(pFileName);
            if (!ShaderCompiler::getIncludedFileSource(m_context, "", filename.get(), source, sourceLength))
                return E_FAIL;
            *ppData = source;
            *pBytes = sourceLength;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Close(LPCVOID pData) override
        {
            return S_OK;
        }
    };

    ShaderCompilerD3D::ShaderCompilerD3D(GfxShaderProfile profile)
        : ShaderCompiler(profile)
    {
    }

#ifdef GFX_USE_SHADERS_DEBUG_LAYER

    static bool processDebugInfo(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, void *srcData, u32 srcDataSize)
    {
        // TODO: test with D3D_DISASM_ENABLE_INSTRUCTION_CYCLE option

        ComPtr<ID3DBlob> debug;
        HRESULT result = D3DDisassemble(srcData, srcDataSize, D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS | D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING, nullptr, &debug);
        if (FAILED(result))
        {
            LogWarn("DirectX error: %i at %s:%i", result, __FILE__, __LINE__);
            context->onError("D3DDisassemble failed.");
            return false;
        }

        // Extract debug info
        char *debugData = static_cast<char *>(debug->GetBufferPointer());
        const i32 debugDataSize = static_cast<i32>(debug->GetBufferSize());
        context->onCollectDebugInfo(meta, permutationIndex, debugData, debugDataSize);

        return true;
    }

#endif
    static bool processShader(ShaderCompilationContext *context, TkArray<ShaderCompiler::ShaderResourceBuffer> &constantBuffers, ID3D11ShaderReflection *reflector, D3D11_SHADER_DESC &desc, ShaderBindings &bindings)
    {
        // Extract constant buffers usage information
        for (u32 a = 0; a < desc.ConstantBuffers; a++)
        {
            auto cb = reflector->GetConstantBufferByIndex(a);
            D3D11_SHADER_BUFFER_DESC cbDesc;
            cb->GetDesc(&cbDesc);
            if (cbDesc.Type == D3D_CT_CBUFFER)
            {
                // Find CB slot index
                i32 slot = -1;
                for (u32 b = 0; b < desc.BoundResources; b++)
                {
                    D3D11_SHADER_INPUT_BIND_DESC bDesc;
                    reflector->GetResourceBindingDesc(b, &bDesc);
                    if (TkStringUtils::compareStrings(bDesc.Name, cbDesc.Name) == 0)
                    {
                        slot = bDesc.BindPoint;
                        break;
                    }
                }
                if (slot == -1)
                {
                    context->onError("Missing bound resource.");
                    return false;
                }

                // Set flag
                bindings.usedCBsMask |= 1 << slot;

                // Try to add CB to the list
                for (i32 b = 0; b < constantBuffers.count(); b++)
                {
                    auto &cc = constantBuffers[b];
                    if (cc.slot == slot)
                    {
                        cc.isUsed = true;
                        cc.size = cbDesc.Size;
                        break;
                    }
                }
            }
        }

        // Extract resources usage
        for (u32 i = 0; i < desc.BoundResources; i++)
        {
            D3D11_SHADER_INPUT_BIND_DESC resDesc;
            reflector->GetResourceBindingDesc(i, &resDesc);
            switch (resDesc.Type)
            {
            // Sampler
            case D3D_SIT_SAMPLER:
                break;

            // Constant Buffer
            case D3D_SIT_CBUFFER:
            case D3D_SIT_TBUFFER:
                break;

            // Shader Resource
            case D3D_SIT_TEXTURE:
            case D3D_SIT_STRUCTURED:
            case D3D_SIT_BYTEADDRESS:
                for (UINT shift = 0; shift < resDesc.BindCount; shift++)
                    bindings.usedSRsMask |= 1 << (resDesc.BindPoint + shift);
                break;

            // Unordered Access
            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                for (UINT shift = 0; shift < resDesc.BindCount; shift++)
                    bindings.usedUAsMask |= 1 << (resDesc.BindPoint + shift);
                break;
            default: break;
            }
        }

        return true;
    }

    bool ShaderCompilerD3D::compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite)
    {
        if (!writeShaderFunctionBegin(m_context, meta))
            return false;

        // Prepare
        auto options = m_context->options;
        auto type = meta.getStage();
        IncludeD3D include(m_context);
        TkString profileName;
        switch (type)
        {
        case ShaderStage::kVertex:
            profileName = "vs";
            break;
        case ShaderStage::kHull:
            profileName = "hs";
            break;
        case ShaderStage::kDomain:
            profileName = "ds";
            break;
        case ShaderStage::kGeometry:
            profileName = "gs";
            break;
        case ShaderStage::kPixel:
            profileName = "ps";
            break;
        case ShaderStage::kCompute:
            profileName = "cs";
            break;
        default:
            return true;
        }
        if (m_profile == GfxShaderProfile::kDirectX_SM5)
        {
            profileName += "_5_0";
        }
        else
        {
            profileName += "_4_0";
            if (type == ShaderStage::kDomain || type == ShaderStage::kHull)
            {
                m_context->onError("Tessellation is not supported on DirectX 10");
                return false;
            }
        }

        // Compile all shader function permutations
        AdditionalDataVS additionalDataVS;
        TkString sourceWithPreamble(getShaderMetaDefinesPreamble());
        sourceWithPreamble += options->source;
        for (i32 permutationIndex = 0; permutationIndex < meta.permutations.count(); permutationIndex++)
        {
            m_macros.clear();

            // Get function permutation macros
            meta.getDefinitionsForPermutation(permutationIndex, m_macros);

            // Add additional define for compiled function name
            getDefineForFunction(meta, m_macros);

            // Add custom and global macros (global last because contain null define to indicate ending)
            m_macros.add(m_context->options->macros);
            m_macros.add(m_globalMacros);

            // Compile
            ComPtr<ID3DBlob> errors;
            ComPtr<ID3DBlob> shader;
            HRESULT result = D3DCompile2(
                sourceWithPreamble.get(),
                sourceWithPreamble.length(),
                m_context->targetNameAnsi,
                reinterpret_cast<const D3D_SHADER_MACRO *>(m_macros.get()),
                &include,
                meta.name.get(),
                profileName.get(),
                m_flags,
                0,
                0,
                nullptr,
                0,
                &shader,
                &errors);
            if (FAILED(result))
            {
                const auto msg = static_cast<const char *>(errors->GetBufferPointer());
                m_context->onError(msg);
                return false;
            }
            void *shaderBuffer = shader->GetBufferPointer();
            u32 shaderBufferSize = static_cast<i32>(shader->GetBufferSize());

            // Perform shader reflection
            ComPtr<ID3D11ShaderReflection> reflector;
            result = D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_ID3D11ShaderReflection, (void **)&reflector);
            if (FAILED(result))
            {
                LogWarn("DirectX error: %i at %s:%i", result, __FILE__, __LINE__);
                m_context->onError("D3DReflect failed.");
                return false;
            }

            // Get shader description
            D3D11_SHADER_DESC desc;
            reflector->GetDesc(&desc);

            // Process shader reflection data
            void *additionalData = nullptr;
            if (type == ShaderStage::kVertex)
            {
                additionalData = &additionalDataVS;
                additionalDataVS.inputs.clear();
                for (UINT inputIdx = 0; inputIdx < desc.InputParameters; inputIdx++)
                {
                    D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
                    reflector->GetInputParameterDesc(inputIdx, &inputDesc);
                    if (inputDesc.SystemValueType != D3D10_NAME_UNDEFINED)
                        continue;
                    auto format = PixelFormat::kUnknown;
                    switch (inputDesc.ComponentType)
                    {
                    case D3D_REGISTER_COMPONENT_UINT32:
                        if (inputDesc.Mask >= 0b1111)
                            format = PixelFormat::kR32G32B32A32_UInt;
                        else if (inputDesc.Mask >= 0b111)
                            format = PixelFormat::kR32G32B32_UInt;
                        else if (inputDesc.Mask >= 0b11)
                            format = PixelFormat::kR32G32_UInt;
                        else
                            format = PixelFormat::kR32_UInt;
                        break;
                    case D3D_REGISTER_COMPONENT_SINT32:
                        if (inputDesc.Mask >= 0b1111)
                            format = PixelFormat::kR32G32B32A32_SInt;
                        else if (inputDesc.Mask >= 0b111)
                            format = PixelFormat::kR32G32B32_SInt;
                        else if (inputDesc.Mask >= 0b11)
                            format = PixelFormat::kR32G32_SInt;
                        else
                            format = PixelFormat::kR32_SInt;
                        break;
                    case D3D_REGISTER_COMPONENT_FLOAT32:
                        if (inputDesc.Mask >= 0b1111)
                            format = PixelFormat::kR32G32B32A32_Float;
                        else if (inputDesc.Mask >= 0b111)
                            format = PixelFormat::kR32G32B32_Float;
                        else if (inputDesc.Mask >= 0b11)
                            format = PixelFormat::kR32G32_Float;
                        else
                            format = PixelFormat::kR32_Float;
                        break;
                    default: break;
                    }
                    additionalDataVS.inputs.add({parseVertexElementType(inputDesc.SemanticName, inputDesc.SemanticIndex), 0, 0, 0, format});
                }
            }
            ShaderBindings bindings = {desc.InstructionCount, 0, 0, 0};
            if (!processShader(m_context, m_constantBuffers, reflector.get(), desc, bindings))
                return false;

#ifdef GFX_USE_SHADERS_DEBUG_LAYER
            // Generate debug information
            if (!processDebugInfo(m_context, meta, permutationIndex, shaderBuffer, shaderBufferSize))
                return false;
#endif

            // Strip shader bytecode for an optimization
            ComPtr<ID3DBlob> shaderStripped;
            if (!options->generateDebugData)
            {
                // Strip shader bytes
                result = D3DStripShader(
                    shaderBuffer,
                    shaderBufferSize,
                    D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS,
                    &shaderStripped);
                if (FAILED(result))
                {
                    LogWarn("Cannot strip shader.");
                    return false;
                }

                // Set new buffer
                shaderBuffer = shaderStripped->GetBufferPointer();
                shaderBufferSize = static_cast<i32>(shaderStripped->GetBufferSize());
            }

            if (!writeShaderFunctionPermutation(m_context, meta, permutationIndex, bindings, shaderBuffer, shaderBufferSize))
                return false;

            if (customDataWrite && !customDataWrite(m_context, meta, permutationIndex, m_macros, additionalData))
                return false;
        }

        return writeShaderFunctionEnd(m_context, meta);
    }

    bool ShaderCompilerD3D::onCompileBegin()
    {
        if (!ShaderCompiler::onCompileBegin())
            return false;

        m_globalMacros.add({"DIRECTX", "1"});

        m_flags = 0;
        if (m_context->options->noOptimize)
            m_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
        else
            m_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
        if (m_context->options->generateDebugData)
            m_flags |= D3DCOMPILE_DEBUG;
        if (m_context->options->treatWarningsAsErrors)
            m_flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if GFX_API_DIRECTX12
        m_flags |= D3DCOMPILE_ALL_RESOURCES_BOUND;
#endif

        return true;
    }

}
