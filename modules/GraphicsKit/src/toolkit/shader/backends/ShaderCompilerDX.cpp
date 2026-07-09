/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ShaderCompilerDX.hpp"

#if TK_WINDOWS

#include "toolkit/shader/core/Context.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"
#include "toolkit/platforms/win32/ComPointer.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/utils/Convertor.hpp"
#include <d3d12shader.h>
#include <dxcapi.h>

namespace toolkit
{
    struct DxShaderHeader
    {
        byte SrDimensions[GFX_MAX_SR_BINDED];
        byte UaDimensions[GFX_MAX_UA_BINDED];
    };

    // Helper class to include source for DX shaders compiler.
    class IncludeDX : public IDxcIncludeHandler
    {
    private:
        ShaderCompilationContext *m_context;
        IDxcLibrary *m_library;

    public:
        IncludeDX(ShaderCompilationContext *context, IDxcLibrary *library)
        {
            m_context = context;
            m_library = library;
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return 1;
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            return 1;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
        {
            if (riid == __uuidof(IDxcIncludeHandler) || riid == __uuidof(IUnknown))
            {
                AddRef();
                *ppvObject = this;
                return S_OK;
            }

            *ppvObject = nullptr;
            return E_NOINTERFACE;
        }

        HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob **ppIncludeSource) override
        {
            *ppIncludeSource = nullptr;
            const char *source;
            i32 sourceLength;
            const TkString filename(StringAsUTF8<>(pFilename).get());
            if (!ShaderCompiler::getIncludedFileSource(m_context, "", filename.get(), source, sourceLength))
                return E_FAIL;
            IDxcBlobEncoding *textBlob;
            if (FAILED(m_library->CreateBlobWithEncodingFromPinned((LPBYTE)source, sourceLength, CP_UTF8, &textBlob)))
                return E_FAIL;
            *ppIncludeSource = textBlob;
            return S_OK;
        }
    };

    ShaderCompilerDX::ShaderCompilerDX(GfxShaderProfile profile)
        : ShaderCompiler(profile)
    {
        IDxcCompiler3 *compiler = nullptr;
        IDxcLibrary *library = nullptr;
        IDxcContainerReflection *containerReflection = nullptr;
        if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, __uuidof(compiler), reinterpret_cast<void **>(&compiler))) ||
            FAILED(DxcCreateInstance(CLSID_DxcLibrary, __uuidof(library), reinterpret_cast<void **>(&library))) ||
            FAILED(DxcCreateInstance(CLSID_DxcContainerReflection, __uuidof(containerReflection), reinterpret_cast<void **>(&containerReflection))))
        {
            LogError("DxcCreateInstance failed");
        }
        m_compiler = compiler;
        m_library = library;
        m_containerReflection = containerReflection;
        static bool PrintVersion = true;
        if (PrintVersion)
        {
            PrintVersion = false;
            IDxcVersionInfo *version = nullptr;
            if (compiler && SUCCEEDED(compiler->QueryInterface(__uuidof(version), reinterpret_cast<void **>(&version))))
            {
                UINT32 major, minor;
                version->GetVersion(&major, &minor);
                LogInfo("DXC version %i.%i", major, minor);
                version->Release();
            }
        }
    }

    ShaderCompilerDX::~ShaderCompilerDX()
    {
        auto compiler = (IDxcCompiler2 *)m_compiler;
        if (compiler)
            compiler->Release();
        auto library = (IDxcLibrary *)m_library;
        if (library)
            library->Release();
        auto containerReflection = (IDxcContainerReflection *)m_containerReflection;
        if (containerReflection)
            containerReflection->Release();
    }

    bool ShaderCompilerDX::compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite)
    {
        if (!writeShaderFunctionBegin(m_context, meta))
            return false;

        // Prepare
        auto options = m_context->options;
        auto compiler = (IDxcCompiler3 *)m_compiler;
        auto library = (IDxcLibrary *)m_library;
        auto containerReflection = (IDxcContainerReflection *)m_containerReflection;
        auto type = meta.getStage();
        IncludeDX include(m_context, library);
        const wchar *targetProfile;
        switch (type)
        {
        case ShaderStage::kVertex:
            targetProfile = L"vs_6_0";
            break;
        case ShaderStage::kHull:
            targetProfile = L"hs_6_0";
            break;
        case ShaderStage::kDomain:
            targetProfile = L"ds_6_0";
            break;
        case ShaderStage::kGeometry:
            targetProfile = L"gs_6_0";
            break;
        case ShaderStage::kPixel:
            targetProfile = L"ps_6_0";
            break;
        case ShaderStage::kCompute:
            targetProfile = L"cs_6_0";
            break;
        default:
            return false;
        }
        TkString sourceWithPreamble(getShaderMetaDefinesPreamble());
        sourceWithPreamble += options->source;
        ComPtr<IDxcBlobEncoding> textBlob;
        if (FAILED(library->CreateBlobWithEncodingFromPinned((LPBYTE)sourceWithPreamble.get(), sourceWithPreamble.length(), CP_UTF8, &textBlob)))
            return false;
        DxcBuffer textBuffer;
        textBuffer.Ptr = textBlob->GetBufferPointer();
        textBuffer.Size = textBlob->GetBufferSize();
        textBuffer.Encoding = DXC_CP_ACP;
        const StringAsUTF16<> entryPoint(meta.name.get(), meta.name.length());
        TkArray<TkString> definesStrings;
        TkStaticArray<const wchar *, 12> args;
        if (m_context->options->noOptimize)
            args.add(DXC_ARG_SKIP_OPTIMIZATIONS);
        else
            args.add(DXC_ARG_OPTIMIZATION_LEVEL3);
        if (m_context->options->treatWarningsAsErrors)
            args.add(DXC_ARG_WARNINGS_ARE_ERRORS);
        if (m_context->options->generateDebugData)
            args.add(DXC_ARG_DEBUG);
        args.add(L"-T");
        args.add(targetProfile);
        args.add(L"-E");
        args.add(entryPoint.get());
        args.add(StringAsUTF16<>(options->targetName.get()).get());
        TkStaticArray<const wchar *, 250> argsFull;
        TkString debugOutputFolder;
        if (m_context->options->generateDebugData)
        {
            TkString toolkitTemp = System::getTempDir() / "toolkit";
            if (!TkFileSystem::isDirExist(toolkitTemp))
                TkFileSystem::makeDir(toolkitTemp);
            TkString shadersTemp = toolkitTemp / "shaders";
            if (!TkFileSystem::isDirExist(shadersTemp))
                TkFileSystem::makeDir(shadersTemp);
            debugOutputFolder = shadersTemp / "dxc";
            if (!TkFileSystem::isDirExist(debugOutputFolder))
                TkFileSystem::makeDir(debugOutputFolder);
        }

        // Compile all shader function permutations
        AdditionalDataVS additionalDataVS;
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

            // Convert defines from char* to Char*
            const i32 macrosCount = m_macros.count() - 1;
            definesStrings.resize(macrosCount);
            for (i32 i = 0; i < macrosCount; i++)
            {
                auto &macro = m_macros[i];
                auto &define = definesStrings[i];
                define = macro.name;
                if (macro.definition && *macro.definition)
                {
                    define += "=";
                    define += macro.definition;
                }
            }

            // Build full list of arguments
            argsFull.clear();
            for (auto &e : args)
                argsFull.add(e);
            for (auto &d : definesStrings)
            {
                argsFull.add(L"-D");
                argsFull.add(StringAsUTF16<>(*d).get());
            }

            // Compile
            ComPtr<IDxcResult> results;
            HRESULT result = compiler->Compile(
                &textBuffer,
                (LPCWSTR *)argsFull.get(),
                argsFull.count(),
                &include,
                IID_PPV_ARGS(&results));
            if (SUCCEEDED(result) && results)
                results->GetStatus(&result);
            if (FAILED(result))
            {
                if (results)
                {
                    ComPtr<IDxcBlobEncoding> error;
                    results->GetErrorBuffer(&error);
                    if (error && error->GetBufferSize() > 0)
                    {
                        ComPtr<IDxcBlobEncoding> errorUtf8;
                        library->GetBlobAsUtf8(error, &errorUtf8);
                        if (errorUtf8)
                        {
                            m_context->onError((const char *)errorUtf8->GetBufferPointer());
                        }
                    }
                }
                return false;
            }

            // Get the output
            ComPtr<IDxcBlob> shaderBuffer = nullptr;
            if (FAILED(results->GetResult(&shaderBuffer)))
            {
                LogError("IDxcOperationResult::GetResult failed.");
                return false;
            }

#ifdef GFX_USE_SHADERS_DEBUG_LAYER
            // Generate debug information
            {
                // Disassemble compiled shader
                ComPtr<IDxcResult> disassembly;
                DxcBuffer shaderDxcBuffer;
                shaderDxcBuffer.Ptr = shaderBuffer->GetBufferPointer();
                shaderDxcBuffer.Size = shaderBuffer->GetBufferSize();
                shaderDxcBuffer.Encoding = DXC_CP_ACP;
                if (FAILED(compiler->Disassemble(&shaderDxcBuffer, IID_PPV_ARGS(&disassembly))))
                    return false;
                ComPtr<IDxcBlob> disassemblyBlob;
                ComPtr<IDxcBlobUtf16> disassemblyPath;
                if (FAILED(disassembly->GetOutput(DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(disassemblyBlob.getAddressOf()), disassemblyPath.getAddressOf())))
                    return false;
                ComPtr<IDxcBlobEncoding> disassemblyUtf8;
                if (FAILED(library->GetBlobAsUtf8(disassemblyBlob, &disassemblyUtf8)))
                    return false;

                // Extract debug info
                m_context->onCollectDebugInfo(meta, permutationIndex, (const char *)disassemblyUtf8->GetBufferPointer(), (i32)disassemblyUtf8->GetBufferSize());
            }
#endif
            if (m_context->options->generateDebugData)
            {
                ComPtr<IDxcBlob> pdbBlob;
                ComPtr<IDxcBlobUtf16> pdbName;
                if (results->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(pdbBlob.getAddressOf()), pdbName.getAddressOf()) == S_OK)
                {
                    const TkString pdbPath = debugOutputFolder / TkString(StringAsUTF8<>(pdbName->GetStringPointer(), (i32)pdbName->GetStringLength()).get());
                    OpenFileParams params{
                        .path = pdbPath,
                        .mode = FileMode::kCreateAlways,
                        .access = FileAccess::kWrite,
                        .share = FileShare::kRead,
                    };
                    auto handle = TkFileSystem::openFile(params);
                    if (TkFileSystem::isFileOpened(handle))
                    {
                        TkFileSystem::writeBytes(handle, (byte *)pdbBlob->GetBufferPointer(), (i32)pdbBlob->GetBufferSize());
                        TkFileSystem::closeFile(handle);
                    }
                }
            }

            // Perform shader reflection
            if (FAILED(containerReflection->Load(shaderBuffer)))
            {
                LogError("IDxcContainerReflection::Load failed.");
                return false;
            }
            const u32 dxilPartKind = DXC_PART_DXIL;
            u32 dxilPartIndex = ~0u;
            if (FAILED(containerReflection->FindFirstPartKind(dxilPartKind, &dxilPartIndex)))
            {
                LogError("IDxcContainerReflection::FindFirstPartKind failed.");
                return false;
            }
            ComPtr<ID3D12ShaderReflection> shaderReflection;
            if (FAILED(containerReflection->GetPartReflection(dxilPartIndex, IID_PPV_ARGS(&shaderReflection))))
            {
                LogError("IDxcContainerReflection::GetPartReflection failed.");
                return false;
            }

            // Get shader description
            D3D12_SHADER_DESC desc;
            TkMemory::memoryClear(&desc, sizeof(desc));
            shaderReflection->GetDesc(&desc);

            // Process shader reflection data
            void *additionalData = nullptr;
            if (type == ShaderStage::kVertex)
            {
                additionalData = &additionalDataVS;
                additionalDataVS.inputs.clear();
                for (UINT inputIdx = 0; inputIdx < desc.InputParameters; inputIdx++)
                {
                    D3D12_SIGNATURE_PARAMETER_DESC inputDesc;
                    shaderReflection->GetInputParameterDesc(inputIdx, &inputDesc);
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
            DxShaderHeader header;
            TkMemory::memoryClear(&header, sizeof(header));
            ShaderBindings bindings = {desc.InstructionCount, 0, 0, 0};
            for (u32 a = 0; a < desc.ConstantBuffers; a++)
            {
                auto cb = shaderReflection->GetConstantBufferByIndex(a);
                D3D12_SHADER_BUFFER_DESC cbDesc;
                cb->GetDesc(&cbDesc);
                if (cbDesc.Type == D3D_CT_CBUFFER)
                {
                    // Find CB slot index
                    i32 slot = -1;
                    for (u32 b = 0; b < desc.BoundResources; b++)
                    {
                        D3D12_SHADER_INPUT_BIND_DESC bDesc;
                        shaderReflection->GetResourceBindingDesc(b, &bDesc);
                        if (TkStringUtils::compareStrings(bDesc.Name, cbDesc.Name) == 0)
                        {
                            slot = bDesc.BindPoint;
                            break;
                        }
                    }
                    if (slot == -1)
                    {
                        m_context->onError("Missing bound resource.");
                        return false;
                    }

                    // Set flag
                    bindings.usedCBsMask |= 1 << slot;

                    // Try to add CB to the list
                    for (i32 b = 0; b < m_constantBuffers.count(); b++)
                    {
                        auto &cc = m_constantBuffers[b];
                        if (cc.slot == slot)
                        {
                            cc.isUsed = true;
                            cc.size = cbDesc.Size;
                            break;
                        }
                    }
                }
            }
            for (u32 i = 0; i < desc.BoundResources; i++)
            {
                D3D12_SHADER_INPUT_BIND_DESC resDesc;
                shaderReflection->GetResourceBindingDesc(i, &resDesc);
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
                    for (UINT shift = 0; shift < resDesc.BindCount; shift++)
                    {
                        bindings.usedSRsMask |= 1 << (resDesc.BindPoint + shift);
                        header.SrDimensions[resDesc.BindPoint + shift] = static_cast<byte>(resDesc.Dimension);
                    }
                    break;
                case D3D_SIT_STRUCTURED:
                case D3D_SIT_BYTEADDRESS:
                    for (UINT shift = 0; shift < resDesc.BindCount; shift++)
                    {
                        bindings.usedSRsMask |= 1 << (resDesc.BindPoint + shift);
                        header.SrDimensions[resDesc.BindPoint + shift] = static_cast<byte>(D3D_SRV_DIMENSION_BUFFER);
                    }
                    break;

                // Unordered Access
                case D3D_SIT_UAV_RWTYPED:
                case D3D_SIT_UAV_RWSTRUCTURED:
                case D3D_SIT_UAV_RWBYTEADDRESS:
                case D3D_SIT_UAV_APPEND_STRUCTURED:
                case D3D_SIT_UAV_CONSUME_STRUCTURED:
                case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                    for (UINT shift = 0; shift < resDesc.BindCount; shift++)
                    {
                        bindings.usedUAsMask |= 1 << (resDesc.BindPoint + shift);
                        header.UaDimensions[resDesc.BindPoint + shift] = (byte)resDesc.Dimension; // D3D_SRV_DIMENSION matches D3D12_UAV_DIMENSION
                    }
                    break;
                default:
                    break;
                }
            }

            if (!writeShaderFunctionPermutation(m_context, meta, permutationIndex, bindings, &header, sizeof(header), shaderBuffer->GetBufferPointer(), (i32)shaderBuffer->GetBufferSize()))
                return false;

            if (customDataWrite && !customDataWrite(m_context, meta, permutationIndex, m_macros, additionalData))
                return false;
        }

        return writeShaderFunctionEnd(m_context, meta);
    }

    bool ShaderCompilerDX::onCompileBegin()
    {
        if (!ShaderCompiler::onCompileBegin())
            return false;

        m_globalMacros.add({"DIRECTX", "1"});

        return true;
    }

}

#endif