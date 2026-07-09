/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ShaderCompilerVulkan.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/shader/core/Context.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"

// Use glslang for HLSL to SPIR-V translation
// Source: https://github.com/KhronosGroup/glslang
// License: modified BSD
#define NV_EXTENSIONS 1
#define AMD_EXTENSIONS 1
#define ENABLE_HLSL 1
#define ENABLE_OPT 1
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <spirv-tools/libspirv.hpp>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#define PRINT_UNIFORMS 0
#define PRINT_DESCRIPTORS 0

namespace toolkit
{
    static TkMutex s_mtx;
    static i32 s_instance = 0;

    // Hack to use VkDescriptorType if Vulkan runtime is not used
    typedef enum VkDescriptorType
    {
        VK_DESCRIPTOR_TYPE_SAMPLER = 0,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
        VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT = 1000138000,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
        VK_DESCRIPTOR_TYPE_BEGIN_RANGE = VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_END_RANGE = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        VK_DESCRIPTOR_TYPE_RANGE_SIZE = (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT - VK_DESCRIPTOR_TYPE_SAMPLER + 1),
        VK_DESCRIPTOR_TYPE_MAX_ENUM = 0x7FFFFFFF
    } VkDescriptorType;

    enum class SpirvShaderResourceType
    {
        Unknown = 0,
        ConstantBuffer = 1,
        Buffer = 2,
        Sampler = 3,
        Texture1D = 4,
        Texture2D = 5,
        Texture3D = 6,
        TextureCube = 7,
        Texture1DArray = 8,
        Texture2DArray = 9,
    };

    enum class SpirvShaderResourceBindingType : byte
    {
        INVALID = 0,
        CB = 1,
        SAMPLER = 2,
        SRV = 3,
        UAV = 4,
        MAX
    };

    struct SpirvShaderDescriptorInfo
    {
        enum
        {
            MaxDescriptors = 64,
        };

        // A single descriptor data.
        struct Descriptor
        {
            // The binding slot (the descriptors slot).
            byte binding;
            // The layout slot (the descriptors set slot).
            byte set;
            // The input slot (the pipeline slot).
            byte slot;
            // The resource binding type (the graphics pipeline abstraction binding layer type).
            SpirvShaderResourceBindingType bindingType;
            // The Vulkan descriptor type.
            VkDescriptorType descriptorType;
            // The resource type.
            SpirvShaderResourceType resourceType;
            // The resource format.
            PixelFormat resourceFormat;
            // The amount of slots used by the descriptor (eg. array of textures size).
            u32 count;
        };

        u16 imageInfosCount;
        u16 bufferInfosCount;
        u32 texelBufferViewsCount;
        u32 descriptorTypesCount;
        Descriptor descriptorTypes[MaxDescriptors];
    };

    struct SpirvShaderHeader
    {
        enum class Types
        {
            // The raw SPIR-V byte code.
            Raw = 0,
        };
        // The data type.
        Types type;
        // The shader descriptors usage information.
        SpirvShaderDescriptorInfo descriptorInfo;
    };

    class Includer : public glslang::TShader::Includer
    {
    private:
        ShaderCompilationContext *m_context;

        IncludeResult *include(const char *headerName, const char *includerName, int depth) const
        {
            const char *source;
            i32 sourceLength;
            if (!ShaderCompiler::getIncludedFileSource(m_context, includerName, headerName, source, sourceLength))
                return nullptr;
            return TkMemory::newObject<IncludeResult>(TkMemory::getDefault(), headerName, source, sourceLength, nullptr);
        }

    public:
        Includer(ShaderCompilationContext *context)
        {
            m_context = context;
        }

    public:
        IncludeResult *includeLocal(const char *headerName, const char *includerName, size_t inclusionDepth) override
        {
            return include(headerName, includerName, (int)inclusionDepth);
        }

        IncludeResult *includeSystem(const char *headerName, const char *includerName, size_t inclusionDepth) override
        {
            return include(headerName, includerName, (int)inclusionDepth);
        }

        void releaseInclude(IncludeResult *result) override
        {
            if (result)
                TkMemory::deletePtr(result);
        }
    };

    ShaderCompilerVulkan::ShaderCompilerVulkan(GfxShaderProfile profile)
        : ShaderCompiler(profile)
    {
        TkMutexGuard lock(s_mtx);
        if (s_instance == 0)
        {
            glslang::InitializeProcess();
            const auto ver = glslang::GetVersion();
            LogInfo("Using glslang %i.%i.%i compiler (SPIR-V version: %s)", ver.major, ver.minor, ver.patch, spvSoftwareVersionString());
        }
        s_instance++;
    }

    ShaderCompilerVulkan::~ShaderCompilerVulkan()
    {
        TkMutexGuard lock(s_mtx);
        s_instance--;
        if (s_instance == 0)
        {
            glslang::FinalizeProcess();
        }
    }

#if 0
    // Legacy internal glslang reflection path. This requires MachineIndependent
    // headers that are not shipped in the current vendored glslang package.
    struct Descriptor
    {
        i32 slot;
        i32 binding;
        i32 size;
        i32 count;
        SpirvShaderResourceBindingType bindingType;
        VkDescriptorType descriptorType;
        SpirvShaderResourceType resourceType;
        PixelFormat resourceFormat;
        std::string name;
    };

    static SpirvShaderResourceType getTextureType(const glslang::TSampler &sampler)
    {
        switch (sampler.dim)
        {
        case glslang::Esd1D:
            return sampler.isArrayed() ? SpirvShaderResourceType::Texture1DArray : SpirvShaderResourceType::Texture1D;
        case glslang::Esd2D:
            return sampler.isArrayed() ? SpirvShaderResourceType::Texture2DArray : SpirvShaderResourceType::Texture2D;
        case glslang::Esd3D:
            return SpirvShaderResourceType::Texture3D;
        case glslang::EsdCube:
            return SpirvShaderResourceType::TextureCube;
        default:
            TK_ASSERT(false && "getTextureType");
            return SpirvShaderResourceType::Unknown;
        }
    }

    static PixelFormat getResourceFormat(glslang::TBasicType basicType, u32 vectorSize)
    {
        switch (basicType)
        {
        case glslang::EbtVoid:
            return PixelFormat::kUnknown;
        case glslang::EbtFloat:
            switch (vectorSize)
            {
            case 1:
                return PixelFormat::kR32_Float;
            case 2:
                return PixelFormat::kR32G32_Float;
            case 3:
                return PixelFormat::kR32G32B32_Float;
            case 4:
                return PixelFormat::kR32G32B32A32_Float;
            }
            break;
        case glslang::EbtFloat16:
            switch (vectorSize)
            {
            case 1:
                return PixelFormat::kR16_Float;
            case 2:
                return PixelFormat::kR16G16_Float;
            case 4:
                return PixelFormat::kR16G16B16A16_Float;
            }
            break;
        case glslang::EbtUint:
            switch (vectorSize)
            {
            case 1:
                return PixelFormat::kR32_UInt;
            case 2:
                return PixelFormat::kR32G32_UInt;
            case 3:
                return PixelFormat::kR32G32B32_UInt;
            case 4:
                return PixelFormat::kR32G32B32A32_UInt;
            }
            break;
        case glslang::EbtInt:
            switch (vectorSize)
            {
            case 1:
                return PixelFormat::kR32_SInt;
            case 2:
                return PixelFormat::kR32G32_SInt;
            case 3:
                return PixelFormat::kR32G32B32_SInt;
            case 4:
                return PixelFormat::kR32G32B32A32_SInt;
            }
            break;
        case glslang::EbtUint8:
            switch (vectorSize)
            {
            case 1:
                return PixelFormat::kR8_UInt;
            case 2:
                return PixelFormat::kR8G8_UInt;
            case 4:
                return PixelFormat::kR8G8B8A8_UInt;
            }
            break;
        case glslang::EbtInt8:
            switch (vectorSize)
            {
            case 1:
                return PixelFormat::kR8_SInt;
            case 2:
                return PixelFormat::kR8G8_SInt;
            case 4:
                return PixelFormat::kR8G8B8A8_SInt;
            }
            break;
        case glslang::EbtUint16:
            switch (vectorSize)
            {
            case 1:
                return PixelFormat::kR16_UInt;
            case 2:
                return PixelFormat::kR16G16_UInt;
            case 4:
                return PixelFormat::kR16G16B16A16_UInt;
            }
            break;
        case glslang::EbtInt16:
            switch (vectorSize)
            {
            case 1:
                return PixelFormat::kR16_SInt;
            case 2:
                return PixelFormat::kR16G16_SInt;
            case 4:
                return PixelFormat::kR16G16B16A16_SInt;
            }
            break;
        default:
            break;
        }
        return PixelFormat::kUnknown;
    }

    static PixelFormat getResourceFormat(const glslang::TSampler &sampler)
    {
        return getResourceFormat(sampler.type, sampler.vectorSize);
    }

    static PixelFormat getResourceFormat(const glslang::TType &type)
    {
        return getResourceFormat(type.getBasicType(), type.getVectorSize());
    }

    static bool isUavType(const glslang::TType &type)
    {
        if (type.getQualifier().isReadOnly())
            return false;
        return (type.getBasicType() == glslang::EbtSampler && type.getSampler().isImage()) || (type.getQualifier().storage == glslang::EvqBuffer);
    }

    class DescriptorsCollector
    {
    public:
        i32 images = 0;
        i32 buffers = 0;
        i32 texelBuffers = 0;
        i32 descriptorsCount = 0;
        Descriptor descriptors[SpirvShaderDescriptorInfo::MaxDescriptors];

    public:
        Descriptor *add(glslang::TVarEntryInfo &ent)
        {
            const glslang::TType &type = ent.symbol->getType();
            const char *name = ent.symbol->getName().c_str();
            auto &qualifier = type.getQualifier();
            if (descriptorsCount == SpirvShaderDescriptorInfo::MaxDescriptors)
            {
                // Prevent too many descriptors
                LogWarn("Too many descriptors in use.");
                return nullptr;
            }

            // Guess the descriptor type based on reflection information
            VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
            SpirvShaderResourceType resourceType = SpirvShaderResourceType::Unknown;
            SpirvShaderResourceBindingType resourceBindingType = SpirvShaderResourceBindingType::INVALID;
            if (type.getBasicType() == glslang::EbtSampler)
            {
                if (!qualifier.hasBinding())
                {
                    // Each resource must have binding specified (from HLSL shaders that do it explicitly)
                    LogWarn("Found an uniform \'%s\' without a binding qualifier. Each uniform must have an explicitly defined binding number.", name);
                    return nullptr;
                }

                if (type.getSampler().isCombined())
                {
                    // Texture + Sampler combined is not supported
                    LogWarn("Combined sampler \'%s\' from glsl language is not supported.", name);
                    return nullptr;
                }

                if (type.getSampler().isPureSampler())
                {
                    // Sampler
                    descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                    resourceType = SpirvShaderResourceType::Sampler;
                    resourceBindingType = SpirvShaderResourceBindingType::SAMPLER;
                }
                else if (type.getSampler().dim == glslang::EsdBuffer)
                {
                    if (isUavType(type))
                    {
                        // Buffer UAV
                        descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                        resourceType = SpirvShaderResourceType::Buffer;
                        resourceBindingType = SpirvShaderResourceBindingType::UAV;
                    }
                    else
                    {
                        // Buffer SRV
                        descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                        resourceType = SpirvShaderResourceType::Buffer;
                        resourceBindingType = SpirvShaderResourceBindingType::SRV;
                    }
                }
                else if (type.isTexture())
                {
                    // Texture SRV
                    descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    resourceType = getTextureType(type.getSampler());
                    resourceBindingType = SpirvShaderResourceBindingType::SRV;
                }
                else if (type.isImage())
                {
                    if (type.getSampler().dim == glslang::EsdBuffer)
                    {
                        // Buffer UAV
                        descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        resourceType = SpirvShaderResourceType::Buffer;
                    }
                    else
                    {
                        // Texture UAV
                        descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        resourceType = getTextureType(type.getSampler());
                    }
                    resourceBindingType = SpirvShaderResourceBindingType::UAV;
                }
            }
            else if (qualifier.storage == glslang::EvqUniform)
            {
                if (type.getBasicType() != glslang::EbtBlock)
                {
                    // Skip uniforms that are not contained inside structures
                    LogWarn("Invalid uniform \'%s %s\'. Shader uniforms that are not constant buffer blocks are not supported.", name, type.getBasicTypeString().c_str());
                    return nullptr;
                }

                // CB
                descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                resourceType = SpirvShaderResourceType::ConstantBuffer;
                name = type.getTypeName().c_str();
                resourceBindingType = SpirvShaderResourceBindingType::CB;
            }
            else if (qualifier.storage == glslang::EvqBuffer)
            {
                if (qualifier.isReadOnly())
                {
                    // Buffer SRV
                    descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    resourceType = SpirvShaderResourceType::Buffer;
                    resourceBindingType = SpirvShaderResourceBindingType::SRV;
                }
                else
                {
                    // Buffer UAV
                    descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    resourceType = SpirvShaderResourceType::Buffer;
                    resourceBindingType = SpirvShaderResourceBindingType::UAV;
                }
            }

            const auto index = descriptorsCount++;
            auto &descriptor = descriptors[index];
            descriptor.binding = index;
            descriptor.slot = qualifier.layoutBinding;
            descriptor.size = -1;
            descriptor.bindingType = resourceBindingType;
            descriptor.descriptorType = descriptorType;
            descriptor.resourceType = resourceType;
            descriptor.resourceFormat = getResourceFormat(type.getSampler());
            descriptor.name = name;
            descriptor.count = type.isSizedArray() ? type.getCumulativeArraySize() : 1;

            // Get the output info about shader uniforms usage
            switch (descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                images += descriptor.count;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                buffers += descriptor.count;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                texelBuffers += descriptor.count;
                break;
            default:
                LogWarn("Invalid descriptor type %i for symbol %s.", (i32)descriptorType, name);
                return nullptr;
            }

            return &descriptor;
        }
    };

    class MyIoMapResolver : public glslang::TDefaultIoResolverBase
    {
    private:
        i32 m_set;
        DescriptorsCollector *m_collector;

    public:
        MyIoMapResolver(i32 set, DescriptorsCollector *collector, const glslang::TIntermediate &intermediate)
            : TDefaultIoResolverBase(intermediate), m_set(set), m_collector(collector)
        {
        }

    public:
        // [glslang::TDefaultIoResolverBase]
        bool validateBinding(EShLanguage stage, glslang::TVarEntryInfo &ent) override
        {
            return true;
        }

        glslang::TResourceType getResourceType(const glslang::TType &type) override
        {
            if (isUavType(type))
                return glslang::EResUav;
            if (isSrvType(type))
                return glslang::EResTexture;
            if (isSamplerType(type))
                return glslang::EResSampler;
            if (isUboType(type))
                return glslang::EResUbo;
            return glslang::EResCount;
        }

        int resolveBinding(EShLanguage stage, glslang::TVarEntryInfo &ent) override
        {
            // Skip unused things
            if (!ent.live)
                return -1;

            // Add resource
            const auto descriptor = m_collector->add(ent);
            if (descriptor)
            {
                return ent.newBinding = reserveSlot(m_set, descriptor->binding, descriptor->count);
            }
            return ent.newBinding;
        }

        int resolveSet(EShLanguage stage, glslang::TVarEntryInfo &ent) override
        {
            // Skip unused things
            if (!ent.live)
                return -1;

            // Use different slot per-stage
            return ent.newSet = m_set;
        }
    };
#endif

    enum SpirvOp : u16
    {
        SpvOpName = 5,
        SpvOpDecorate = 71,
        SpvOpMemberDecorate = 72,
        SpvOpTypeInt = 21,
        SpvOpTypeFloat = 22,
        SpvOpTypeVector = 23,
        SpvOpTypeMatrix = 24,
        SpvOpTypeImage = 25,
        SpvOpTypeSampler = 26,
        SpvOpTypeSampledImage = 27,
        SpvOpTypeArray = 28,
        SpvOpTypeRuntimeArray = 29,
        SpvOpTypeStruct = 30,
        SpvOpTypePointer = 32,
        SpvOpConstant = 43,
        SpvOpVariable = 59,
    };

    enum SpirvStorageClass : u32
    {
        SpvStorageUniformConstant = 0,
        SpvStorageInput = 1,
        SpvStorageUniform = 2,
        SpvStoragePushConstant = 9,
        SpvStorageImage = 11,
        SpvStorageStorageBuffer = 12,
    };

    enum SpirvDecoration : u32
    {
        SpvDecorationBlock = 2,
        SpvDecorationBufferBlock = 3,
        SpvDecorationBuiltIn = 11,
        SpvDecorationNonWritable = 24,
        SpvDecorationNonReadable = 25,
        SpvDecorationLocation = 30,
        SpvDecorationBinding = 33,
        SpvDecorationDescriptorSet = 34,
        SpvDecorationOffset = 35,
        SpvDecorationArrayStride = 6,
    };

    enum SpirvDim : u32
    {
        SpvDim1D = 0,
        SpvDim2D = 1,
        SpvDim3D = 2,
        SpvDimCube = 3,
        SpvDimBuffer = 5,
        SpvDimSubpassData = 6,
    };

    enum class SpirvTypeKind
    {
        Unknown,
        Int,
        Float,
        Vector,
        Matrix,
        Image,
        Sampler,
        SampledImage,
        Array,
        RuntimeArray,
        Struct,
        Pointer,
    };

    struct SpirvTypeInfo
    {
        SpirvTypeKind kind = SpirvTypeKind::Unknown;
        u32 width = 0;
        bool isSigned = false;
        u32 componentType = 0;
        u32 vectorSize = 1;
        u32 columnCount = 1;
        u32 sampledType = 0;
        u32 dim = 0;
        bool arrayed = false;
        u32 sampled = 0;
        u32 elementType = 0;
        u32 length = 1;
        u32 storageClass = 0;
        u32 pointeeType = 0;
        u32 arrayStride = 0;
        std::vector<u32> members;
        std::vector<i32> memberOffsets;
    };

    struct SpirvVarInfo
    {
        u32 typeId = 0;
        u32 storageClass = 0;
    };

    struct SpirvDecorInfo
    {
        i32 location = -1;
        i32 binding = -1;
        i32 set = -1;
        bool block = false;
        bool bufferBlock = false;
        bool builtIn = false;
        bool nonWritable = false;
        bool nonReadable = false;
    };

    struct SpirvVertexInput
    {
        TkString semantic;
        u32 semanticIndex = 0;
        PixelFormat format = PixelFormat::kUnknown;
    };

    struct SpirvDescriptorRemap
    {
        u32 id = 0;
        u32 binding = 0;
        u32 set = 0;
    };

    struct SpirvReflectionResult
    {
        u32 instructionsCount = 0;
        TkArray<SpirvVertexInput> vertexInputs;
        std::vector<SpirvDescriptorRemap> descriptorRemaps;
    };

    static u32 readSpirvString(const u32 *words, u16 wordCount, u16 firstWord, std::string &value)
    {
        value.clear();
        for (u16 i = firstWord; i < wordCount; i++)
        {
            const u32 word = words[i];
            for (u32 b = 0; b < 4; b++)
            {
                const char c = (char)((word >> (b * 8)) & 0xff);
                if (c == '\0')
                    return i + 1;
                value.push_back(c);
            }
        }
        return wordCount;
    }

    static u32 resolveSpirvType(const std::vector<SpirvTypeInfo> &types, u32 typeId)
    {
        while (typeId < types.size())
        {
            const auto &type = types[typeId];
            if (type.kind == SpirvTypeKind::Pointer)
                typeId = type.pointeeType;
            else
                break;
        }
        return typeId;
    }

    static u32 getSpirvArrayCount(const std::vector<SpirvTypeInfo> &types, u32 typeId)
    {
        typeId = resolveSpirvType(types, typeId);
        if (typeId >= types.size())
            return 1;
        const auto &type = types[typeId];
        return type.kind == SpirvTypeKind::Array ? std::max(1u, type.length) : 1u;
    }

    static u32 unwrapSpirvArray(const std::vector<SpirvTypeInfo> &types, u32 typeId)
    {
        typeId = resolveSpirvType(types, typeId);
        while (typeId < types.size())
        {
            const auto &type = types[typeId];
            if (type.kind == SpirvTypeKind::Array || type.kind == SpirvTypeKind::RuntimeArray)
                typeId = resolveSpirvType(types, type.elementType);
            else
                break;
        }
        return typeId;
    }

    static PixelFormat getSpirvResourceFormat(const std::vector<SpirvTypeInfo> &types, u32 typeId)
    {
        typeId = resolveSpirvType(types, typeId);
        u32 vectorSize = 1;
        if (typeId < types.size() && types[typeId].kind == SpirvTypeKind::Vector)
        {
            vectorSize = types[typeId].vectorSize;
            typeId = resolveSpirvType(types, types[typeId].componentType);
        }
        if (typeId >= types.size())
            return PixelFormat::kUnknown;

        const auto &type = types[typeId];
        if (type.kind == SpirvTypeKind::Float)
        {
            if (type.width == 32)
            {
                switch (vectorSize)
                {
                case 1: return PixelFormat::kR32_Float;
                case 2: return PixelFormat::kR32G32_Float;
                case 3: return PixelFormat::kR32G32B32_Float;
                case 4: return PixelFormat::kR32G32B32A32_Float;
                default: break;
                }
            }
            if (type.width == 16)
            {
                switch (vectorSize)
                {
                case 1: return PixelFormat::kR16_Float;
                case 2: return PixelFormat::kR16G16_Float;
                case 4: return PixelFormat::kR16G16B16A16_Float;
                default: break;
                }
            }
        }
        else if (type.kind == SpirvTypeKind::Int)
        {
            if (type.width == 32)
            {
                if (type.isSigned)
                {
                    switch (vectorSize)
                    {
                    case 1: return PixelFormat::kR32_SInt;
                    case 2: return PixelFormat::kR32G32_SInt;
                    case 3: return PixelFormat::kR32G32B32_SInt;
                    case 4: return PixelFormat::kR32G32B32A32_SInt;
                    default: break;
                    }
                }
                switch (vectorSize)
                {
                case 1: return PixelFormat::kR32_UInt;
                case 2: return PixelFormat::kR32G32_UInt;
                case 3: return PixelFormat::kR32G32B32_UInt;
                case 4: return PixelFormat::kR32G32B32A32_UInt;
                default: break;
                }
            }
            if (type.width == 16)
            {
                if (type.isSigned)
                {
                    switch (vectorSize)
                    {
                    case 1: return PixelFormat::kR16_SInt;
                    case 2: return PixelFormat::kR16G16_SInt;
                    case 4: return PixelFormat::kR16G16B16A16_SInt;
                    default: break;
                    }
                }
                switch (vectorSize)
                {
                case 1: return PixelFormat::kR16_UInt;
                case 2: return PixelFormat::kR16G16_UInt;
                case 4: return PixelFormat::kR16G16B16A16_UInt;
                default: break;
                }
            }
            if (type.width == 8)
            {
                if (type.isSigned)
                {
                    switch (vectorSize)
                    {
                    case 1: return PixelFormat::kR8_SInt;
                    case 2: return PixelFormat::kR8G8_SInt;
                    case 4: return PixelFormat::kR8G8B8A8_SInt;
                    default: break;
                    }
                }
                switch (vectorSize)
                {
                case 1: return PixelFormat::kR8_UInt;
                case 2: return PixelFormat::kR8G8_UInt;
                case 4: return PixelFormat::kR8G8B8A8_UInt;
                default: break;
                }
            }
        }
        return PixelFormat::kUnknown;
    }

    static u32 getSpirvTypeSize(const std::vector<SpirvTypeInfo> &types, u32 typeId)
    {
        typeId = resolveSpirvType(types, typeId);
        if (typeId >= types.size())
            return 0;
        const auto &type = types[typeId];
        switch (type.kind)
        {
        case SpirvTypeKind::Int:
        case SpirvTypeKind::Float:
            return type.width / 8;
        case SpirvTypeKind::Vector:
            return getSpirvTypeSize(types, type.componentType) * type.vectorSize;
        case SpirvTypeKind::Matrix:
            return getSpirvTypeSize(types, type.componentType) * type.columnCount;
        case SpirvTypeKind::Array:
            return type.arrayStride ? type.arrayStride * type.length : getSpirvTypeSize(types, type.elementType) * type.length;
        case SpirvTypeKind::Struct:
        {
            u32 size = 0;
            for (u32 i = 0; i < type.members.size(); i++)
            {
                const u32 memberSize = getSpirvTypeSize(types, type.members[i]);
                const u32 memberOffset = i < type.memberOffsets.size() && type.memberOffsets[i] >= 0 ? (u32)type.memberOffsets[i] : size;
                size = std::max(size, memberOffset + memberSize);
            }
            return size;
        }
        default:
            return 0;
        }
    }

    static SpirvShaderResourceType getSpirvTextureType(const SpirvTypeInfo &imageType)
    {
        switch (imageType.dim)
        {
        case SpvDim1D:
            return imageType.arrayed ? SpirvShaderResourceType::Texture1DArray : SpirvShaderResourceType::Texture1D;
        case SpvDim2D:
            return imageType.arrayed ? SpirvShaderResourceType::Texture2DArray : SpirvShaderResourceType::Texture2D;
        case SpvDim3D:
            return SpirvShaderResourceType::Texture3D;
        case SpvDimCube:
            return SpirvShaderResourceType::TextureCube;
        case SpvDimBuffer:
            return SpirvShaderResourceType::Buffer;
        default:
            return SpirvShaderResourceType::Unknown;
        }
    }

    static u32 addSpirvDescriptor(SpirvShaderHeader &header, ShaderBindings &bindings, SpirvShaderResourceBindingType bindingType, VkDescriptorType descriptorType, SpirvShaderResourceType resourceType, PixelFormat format, u32 slot, u32 set, u32 count)
    {
        auto &descriptorInfo = header.descriptorInfo;
        if (descriptorInfo.descriptorTypesCount >= SpirvShaderDescriptorInfo::MaxDescriptors)
        {
            LogWarn("Too many Vulkan shader descriptors in use.");
            return (u32)-1;
        }

        auto &descriptor = descriptorInfo.descriptorTypes[descriptorInfo.descriptorTypesCount];
        descriptor.binding = (byte)descriptorInfo.descriptorTypesCount;
        descriptor.set = (byte)set;
        descriptor.slot = (byte)slot;
        descriptor.bindingType = bindingType;
        descriptor.descriptorType = descriptorType;
        descriptor.resourceType = resourceType;
        descriptor.resourceFormat = format;
        descriptor.count = std::max(1u, count);
        descriptorInfo.descriptorTypesCount++;

        switch (descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            descriptorInfo.imageInfosCount += (u16)descriptor.count;
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            descriptorInfo.bufferInfosCount += descriptor.count;
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            descriptorInfo.texelBufferViewsCount += descriptor.count;
            break;
        default:
            break;
        }

        for (u32 i = 0; i < descriptor.count; i++)
        {
            const u32 boundSlot = slot + i;
            if (boundSlot >= 32)
                continue;
            if (bindingType == SpirvShaderResourceBindingType::CB)
                bindings.usedCBsMask |= 1u << boundSlot;
            else if (bindingType == SpirvShaderResourceBindingType::SRV)
                bindings.usedSRsMask |= 1u << boundSlot;
            else if (bindingType == SpirvShaderResourceBindingType::UAV)
                bindings.usedUAsMask |= 1u << boundSlot;
        }

        return descriptor.binding;
    }

    static void patchSpirvDescriptorBindings(std::vector<unsigned> &spirv, SpirvReflectionResult &result)
    {
        if (result.descriptorRemaps.empty())
            return;

        u32 insertOffset = 5;
        for (u32 offset = 5; offset < spirv.size();)
        {
            const u32 instruction = spirv[offset];
            const u16 wordCount = (u16)(instruction >> 16);
            const u16 op = (u16)(instruction & 0xffff);
            if (wordCount == 0 || offset + wordCount > spirv.size())
                break;

            if (op == SpvOpDecorate && wordCount >= 4)
            {
                insertOffset = offset + wordCount;
                for (auto &remap : result.descriptorRemaps)
                {
                    if (spirv[offset + 1] != remap.id)
                        continue;

                    if (spirv[offset + 2] == SpvDecorationBinding)
                        spirv[offset + 3] = remap.binding;
                    else if (spirv[offset + 2] == SpvDecorationDescriptorSet)
                        spirv[offset + 3] = remap.set;
                }
            }
            else if (op == SpvOpMemberDecorate || op == SpvOpName)
            {
                insertOffset = offset + wordCount;
            }

            offset += wordCount;
        }

        std::vector<unsigned> extraDecorations;
        for (const auto &remap : result.descriptorRemaps)
        {
            bool hasSet = false;
            for (u32 offset = 5; offset < spirv.size();)
            {
                const u32 instruction = spirv[offset];
                const u16 wordCount = (u16)(instruction >> 16);
                const u16 op = (u16)(instruction & 0xffff);
                if (wordCount == 0 || offset + wordCount > spirv.size())
                    break;

                if (op == SpvOpDecorate && wordCount >= 4 && spirv[offset + 1] == remap.id && spirv[offset + 2] == SpvDecorationDescriptorSet)
                {
                    hasSet = true;
                    break;
                }
                offset += wordCount;
            }

            if (!hasSet)
            {
                extraDecorations.push_back((4u << 16) | SpvOpDecorate);
                extraDecorations.push_back(remap.id);
                extraDecorations.push_back(SpvDecorationDescriptorSet);
                extraDecorations.push_back(remap.set);
            }
        }

        if (!extraDecorations.empty())
            spirv.insert(spirv.begin() + insertOffset, extraDecorations.begin(), extraDecorations.end());
    }

    static TkString normalizeSpirvVertexSemantic(const std::string &name, u32 location)
    {
        std::string semantic = name;
        const auto lastDot = semantic.find_last_of('.');
        if (lastDot != std::string::npos)
            semantic = semantic.substr(lastDot + 1);
        const auto lastColon = semantic.find_last_of(':');
        if (lastColon != std::string::npos)
            semantic = semantic.substr(lastColon + 1);
        if (semantic.rfind("in_", 0) == 0)
            semantic = semantic.substr(3);
        else if (semantic.rfind("input_", 0) == 0)
            semantic = semantic.substr(6);

        for (char &c : semantic)
            c = (char)std::toupper((unsigned char)c);

        if (semantic == "UV")
            semantic = "TEXCOORD0";
        else if (semantic == "TEXCOORD")
            semantic += std::to_string(location > 0 ? location - 1 : 0);
        else if (semantic == "POS")
            semantic = "POSITION";

        if (semantic.empty())
        {
            switch (location)
            {
            case 0: semantic = "POSITION"; break;
            case 1: semantic = "TEXCOORD0"; break;
            case 2: semantic = "NORMAL"; break;
            default: semantic = "ATTRIBUTE" + std::to_string(location); break;
            }
        }
        return TkString(semantic.c_str());
    }

    static bool reflectSpirv(const std::vector<unsigned> &spirv, ShaderStage stage, u32 stageSet, SpirvShaderHeader &header, ShaderBindings &bindings, TkArray<ShaderCompiler::ShaderResourceBuffer> &constantBuffers, SpirvReflectionResult &result)
    {
        if (spirv.size() < 5 || spirv[0] != 0x07230203)
        {
            LogWarn("Invalid SPIR-V module.");
            return false;
        }

        const u32 bound = spirv[3];
        std::vector<SpirvTypeInfo> types(bound);
        std::vector<SpirvVarInfo> variables(bound);
        std::vector<SpirvDecorInfo> decorations(bound);
        std::vector<u64> constants(bound, 0);
        std::vector<std::string> names(bound);

        for (u32 offset = 5; offset < spirv.size();)
        {
            const u32 instruction = spirv[offset];
            const u16 wordCount = (u16)(instruction >> 16);
            const u16 op = (u16)(instruction & 0xffff);
            if (wordCount == 0 || offset + wordCount > spirv.size())
                break;
            const u32 *words = &spirv[offset];
            result.instructionsCount++;

            switch (op)
            {
            case SpvOpName:
                if (wordCount >= 3 && words[1] < bound)
                    readSpirvString(words, wordCount, 2, names[words[1]]);
                break;
            case SpvOpDecorate:
                if (wordCount >= 3 && words[1] < bound)
                {
                    auto &decor = decorations[words[1]];
                    switch (words[2])
                    {
                    case SpvDecorationBlock: decor.block = true; break;
                    case SpvDecorationBufferBlock: decor.bufferBlock = true; break;
                    case SpvDecorationBuiltIn: decor.builtIn = true; break;
                    case SpvDecorationNonWritable: decor.nonWritable = true; break;
                    case SpvDecorationNonReadable: decor.nonReadable = true; break;
                    case SpvDecorationLocation: if (wordCount >= 4) decor.location = (i32)words[3]; break;
                    case SpvDecorationBinding: if (wordCount >= 4) decor.binding = (i32)words[3]; break;
                    case SpvDecorationDescriptorSet: if (wordCount >= 4) decor.set = (i32)words[3]; break;
                    case SpvDecorationArrayStride:
                        if (wordCount >= 4 && words[1] < types.size())
                            types[words[1]].arrayStride = words[3];
                        break;
                    default:
                        break;
                    }
                }
                break;
            case SpvOpMemberDecorate:
                if (wordCount >= 5 && words[1] < bound && words[3] == SpvDecorationOffset)
                {
                    auto &type = types[words[1]];
                    if (type.memberOffsets.size() <= words[2])
                        type.memberOffsets.resize(words[2] + 1, -1);
                    type.memberOffsets[words[2]] = (i32)words[4];
                }
                break;
            case SpvOpTypeInt:
                if (wordCount >= 4 && words[1] < bound)
                {
                    types[words[1]].kind = SpirvTypeKind::Int;
                    types[words[1]].width = words[2];
                    types[words[1]].isSigned = words[3] != 0;
                }
                break;
            case SpvOpTypeFloat:
                if (wordCount >= 3 && words[1] < bound)
                {
                    types[words[1]].kind = SpirvTypeKind::Float;
                    types[words[1]].width = words[2];
                }
                break;
            case SpvOpTypeVector:
                if (wordCount >= 4 && words[1] < bound)
                {
                    types[words[1]].kind = SpirvTypeKind::Vector;
                    types[words[1]].componentType = words[2];
                    types[words[1]].vectorSize = words[3];
                }
                break;
            case SpvOpTypeMatrix:
                if (wordCount >= 4 && words[1] < bound)
                {
                    types[words[1]].kind = SpirvTypeKind::Matrix;
                    types[words[1]].componentType = words[2];
                    types[words[1]].columnCount = words[3];
                }
                break;
            case SpvOpTypeImage:
                if (wordCount >= 9 && words[1] < bound)
                {
                    auto &type = types[words[1]];
                    type.kind = SpirvTypeKind::Image;
                    type.sampledType = words[2];
                    type.dim = words[3];
                    type.arrayed = words[5] != 0;
                    type.sampled = words[7];
                }
                break;
            case SpvOpTypeSampler:
                if (wordCount >= 2 && words[1] < bound)
                    types[words[1]].kind = SpirvTypeKind::Sampler;
                break;
            case SpvOpTypeSampledImage:
                if (wordCount >= 3 && words[1] < bound)
                {
                    types[words[1]].kind = SpirvTypeKind::SampledImage;
                    types[words[1]].elementType = words[2];
                }
                break;
            case SpvOpTypeArray:
                if (wordCount >= 4 && words[1] < bound)
                {
                    types[words[1]].kind = SpirvTypeKind::Array;
                    types[words[1]].elementType = words[2];
                    types[words[1]].length = words[3] < constants.size() ? (u32)constants[words[3]] : 1;
                }
                break;
            case SpvOpTypeRuntimeArray:
                if (wordCount >= 3 && words[1] < bound)
                {
                    types[words[1]].kind = SpirvTypeKind::RuntimeArray;
                    types[words[1]].elementType = words[2];
                }
                break;
            case SpvOpTypeStruct:
                if (wordCount >= 2 && words[1] < bound)
                {
                    auto &type = types[words[1]];
                    type.kind = SpirvTypeKind::Struct;
                    type.members.assign(words + 2, words + wordCount);
                    if (type.memberOffsets.size() < type.members.size())
                        type.memberOffsets.resize(type.members.size(), -1);
                }
                break;
            case SpvOpTypePointer:
                if (wordCount >= 4 && words[1] < bound)
                {
                    types[words[1]].kind = SpirvTypeKind::Pointer;
                    types[words[1]].storageClass = words[2];
                    types[words[1]].pointeeType = words[3];
                }
                break;
            case SpvOpConstant:
                if (wordCount >= 4 && words[2] < bound)
                {
                    constants[words[2]] = words[3];
                    if (wordCount >= 5)
                        constants[words[2]] |= (u64)words[4] << 32;
                }
                break;
            case SpvOpVariable:
                if (wordCount >= 4 && words[2] < bound)
                {
                    variables[words[2]].typeId = words[1];
                    variables[words[2]].storageClass = words[3];
                }
                break;
            default:
                break;
            }

            offset += wordCount;
        }

        for (u32 id = 0; id < variables.size(); id++)
        {
            const auto &var = variables[id];
            if (var.typeId == 0)
                continue;

            const auto &decor = decorations[id];
            u32 typeId = resolveSpirvType(types, var.typeId);
            const u32 descriptorTypeId = unwrapSpirvArray(types, typeId);

            if (stage == ShaderStage::kVertex && var.storageClass == SpvStorageInput && !decor.builtIn)
            {
                SpirvVertexInput input;
                input.semantic = normalizeSpirvVertexSemantic(names[id], decor.location >= 0 ? (u32)decor.location : result.vertexInputs.count());
                input.format = getSpirvResourceFormat(types, typeId);
                result.vertexInputs.add(input);
                continue;
            }

            if (decor.binding < 0)
                continue;

            const u32 slot = (u32)decor.binding;
            const u32 set = stageSet;
            const u32 count = getSpirvArrayCount(types, typeId);
            if (descriptorTypeId >= types.size())
                continue;

            const auto &type = types[descriptorTypeId];
            if (var.storageClass == SpvStorageStorageBuffer || (var.storageClass == SpvStorageUniform && decorations[descriptorTypeId].bufferBlock))
            {
                const bool readOnly = decor.nonWritable || decorations[descriptorTypeId].nonWritable;
                const u32 binding = addSpirvDescriptor(header, bindings, readOnly ? SpirvShaderResourceBindingType::SRV : SpirvShaderResourceBindingType::UAV, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SpirvShaderResourceType::Buffer, PixelFormat::kUnknown, slot, set, count);
                if (binding != (u32)-1)
                    result.descriptorRemaps.push_back({id, binding, set});
                continue;
            }

            if (var.storageClass == SpvStorageUniform && decorations[descriptorTypeId].block)
            {
                const u32 binding = addSpirvDescriptor(header, bindings, SpirvShaderResourceBindingType::CB, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SpirvShaderResourceType::ConstantBuffer, PixelFormat::kUnknown, slot, set, count);
                if (binding != (u32)-1)
                    result.descriptorRemaps.push_back({id, binding, set});
                const u32 size = getSpirvTypeSize(types, descriptorTypeId);
                for (i32 i = 0; i < constantBuffers.count(); i++)
                {
                    auto &cb = constantBuffers[i];
                    if (cb.slot == slot)
                    {
                        cb.isUsed = true;
                        cb.size = size;
                        break;
                    }
                }
                continue;
            }

            if (var.storageClass == SpvStorageUniformConstant || var.storageClass == SpvStorageImage)
            {
                u32 imageTypeId = descriptorTypeId;
                if (type.kind == SpirvTypeKind::SampledImage)
                    imageTypeId = resolveSpirvType(types, type.elementType);
                if (imageTypeId >= types.size())
                    continue;

                const auto &imageType = types[imageTypeId];
                if (type.kind == SpirvTypeKind::Sampler)
                {
                    const u32 binding = addSpirvDescriptor(header, bindings, SpirvShaderResourceBindingType::SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER, SpirvShaderResourceType::Sampler, PixelFormat::kUnknown, slot, set, count);
                    if (binding != (u32)-1)
                        result.descriptorRemaps.push_back({id, binding, set});
                }
                else if (imageType.kind == SpirvTypeKind::Image)
                {
                    const bool isUav = imageType.sampled == 2 || var.storageClass == SpvStorageImage || decor.nonReadable;
                    const bool isBuffer = imageType.dim == SpvDimBuffer;
                    const VkDescriptorType vkType = isBuffer ? (isUav ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) : (isUav ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
                    const u32 binding = addSpirvDescriptor(header, bindings, isUav ? SpirvShaderResourceBindingType::UAV : SpirvShaderResourceBindingType::SRV, vkType, getSpirvTextureType(imageType), getSpirvResourceFormat(types, imageType.sampledType), slot, set, count);
                    if (binding != (u32)-1)
                        result.descriptorRemaps.push_back({id, binding, set});
                }
            }
        }

        return true;
    }

    bool ShaderCompilerVulkan::compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite)
    {
        // TODO: test without locking
        TkMutexGuard lock(s_mtx);
        Includer includer(m_context);

        // Prepare
        if (!writeShaderFunctionBegin(m_context, meta))
            return false;
        auto options = m_context->options;
        auto type = meta.getStage();

        // Prepare
        EShLanguage lang = EShLanguage::EShLangCount;
        switch (type)
        {
        case ShaderStage::kVertex:
            lang = EShLanguage::EShLangVertex;
            break;
        case ShaderStage::kHull:
            lang = EShLanguage::EShLangTessControl;
            break;
        case ShaderStage::kDomain:
            lang = EShLanguage::EShLangTessEvaluation;
            break;
        case ShaderStage::kGeometry:
            lang = EShLanguage::EShLangGeometry;
            break;
        case ShaderStage::kPixel:
            lang = EShLanguage::EShLangFragment;
            break;
        case ShaderStage::kCompute:
            lang = EShLanguage::EShLangCompute;
            break;
        default:
            LogError("Unknown shader type.");
            return false;
        }
        EShMessages messages = (EShMessages)(EShMsgReadHlsl | EShMsgSpvRules | EShMsgVulkanRules);

        // Compile all shader function permutations
        AdditionalDataVS additionalDataVS;
        for (i32 permutationIndex = 0; permutationIndex < meta.permutations.count(); permutationIndex++)
        {
#if PRINT_DESCRIPTORS
            LogWarn("VULKAN SHADER %s: %s[%i]", m_context->options->targetName, meta.name, permutationIndex);
#endif
            m_macros.clear();

            // Get function permutation macros
            meta.getDefinitionsForPermutation(permutationIndex, m_macros);

            // Add additional define for compiled function name
            getDefineForFunction(meta, m_macros);

            // Add custom and global macros (global last because contain null define to indicate ending)
            m_macros.add(m_context->options->macros);
            m_macros.add(m_globalMacros);

            // Offset inputs for some pipeline stages to match the descriptors sets layout
            i32 stageSet;
            switch (type)
            {
            case ShaderStage::kVertex:
            case ShaderStage::kCompute:
                stageSet = 0;
                break;
            case ShaderStage::kPixel:
                stageSet = 1;
                break;
            case ShaderStage::kGeometry:
                stageSet = 2;
                break;
            case ShaderStage::kHull:
                stageSet = 3;
                break;
            case ShaderStage::kDomain:
                stageSet = 4;
                break;
            default:
                LogError("Unknown shader type.");
                return false;
            }

            // Parse HLSL shader using glslang
            glslang::TShader shader(lang);
            glslang::TProgram program;
            shader.setEntryPoint(meta.name.get());
            shader.setSourceEntryPoint(meta.name.get());
            int lengths = options->srcLen - 1;
            const char *names = m_context->targetNameAnsi;
            shader.setStringsWithLengthsAndNames(&options->source, &lengths, &names, 1);
            const int defaultVersion = 450;
            std::string preamble;
            preamble.append(getShaderMetaDefinesPreamble());
            for (i32 i = 0; i < m_macros.count() - 1; i++)
            {
                auto &macro = m_macros[i];
                preamble.append("#define ");
                preamble.append(macro.name);
                if (macro.definition)
                {
                    preamble.append(" ");
                    preamble.append(macro.definition);
                }
                preamble.append("\n");
            }
            shader.setPreamble(preamble.c_str());
            shader.setInvertY(true);
            // shader.setAutoMapLocations(true);
            // shader.setAutoMapBindings(true);
            // shader.setShiftBinding(glslang::TResourceType::EResUav, 500);
            shader.setHlslIoMapping(true);
            shader.setEnvInput(glslang::EShSourceHlsl, lang, glslang::EShClientVulkan, defaultVersion);
            shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
            shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
            if (!shader.parse(GetDefaultResources(), defaultVersion, false, messages, includer))
            {
                const auto msg = shader.getInfoLog();
                m_context->onError(msg);
                return false;
            }
            program.addShader(&shader);

            // Generate reflection information
            if (!program.link(messages))
            {
                const auto msg = program.getInfoLog();
                m_context->onError(msg);
                return false;
            }
            if (!program.getIntermediate(lang))
            {
                const auto msg = program.getInfoLog();
                m_context->onError(msg);
                return false;
            }
            // Process shader reflection data
            void *additionalData = nullptr;
            SpirvShaderHeader header;
            TkMemory::memoryClear(&header, sizeof(header));
            ShaderBindings bindings = {0, 0, 0, 0};
            if (type == ShaderStage::kVertex)
            {
                additionalData = &additionalDataVS;
                additionalDataVS.inputs.clear();
            }

            // Generate SPIR-V (optimize it at the same time)
            std::vector<unsigned> spirv;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;
            spvOptions.generateDebugInfo = false;
            spvOptions.disassemble = false;
            spvOptions.disableOptimizer = options->noOptimize;
            spvOptions.optimizeSize = !options->noOptimize;
            spvOptions.stripDebugInfo = !options->generateDebugData;
#if TK_DEBUG
            spvOptions.validate = true;
#else
            spvOptions.validate = false;
#endif
            glslang::GlslangToSpv(*program.getIntermediate(lang), spirv, &logger, &spvOptions);
            const std::string spirvLogOutput = logger.getAllMessages();
            if (!spirvLogOutput.empty())
            {
                LogWarn("SPIR-V generator log:\n%s", spirvLogOutput.c_str());
            }
            if (spirv.empty())
            {
                LogWarn("SPIR-V generator failed");
                return false;
            }

            SpirvReflectionResult reflection;
            if (!reflectSpirv(spirv, type, (u32)stageSet, header, bindings, m_constantBuffers, reflection))
                return false;
            patchSpirvDescriptorBindings(spirv, reflection);
            bindings.instructionsCount = reflection.instructionsCount;
            if (type == ShaderStage::kVertex)
            {
                for (i32 inputIdx = 0; inputIdx < reflection.vertexInputs.count(); inputIdx++)
                {
                    const auto &input = reflection.vertexInputs[inputIdx];
                    additionalDataVS.inputs.add({parseVertexElementType(input.semantic.get(), input.semanticIndex), 0, 0, 0, input.format});
                }
            }

#if 0
        // Dump SPIR-V as text for debugging
        {
            spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
            std::string spirvText;
            tools.Disassemble(spirv, &spirvText);
            m_context->OnCollectDebugInfo(meta, permutationIndex, spirvText.c_str(), (i32)spirvText.size());
        }
#endif

            i32 spirvBytesCount = (i32)spirv.size() * sizeof(unsigned);
            header.type = SpirvShaderHeader::Types::Raw;

            if (!writeShaderFunctionPermutation(m_context, meta, permutationIndex, bindings, &header, sizeof(header), &spirv[0], spirvBytesCount))
                return false;

            if (customDataWrite && !customDataWrite(m_context, meta, permutationIndex, m_macros, additionalData))
                return false;
        }

        return writeShaderFunctionEnd(m_context, meta);
    }

    bool ShaderCompilerVulkan::onCompileBegin()
    {
        if (!ShaderCompiler::onCompileBegin())
            return false;

        //_globalMacros.add({ "VULKAN", "1" }); // glslang compiler adds VULKAN define if EShMsgVulkanRules flag is specified

        // TODO: handle options->TreatWarningsAsErrors

        return true;
    }

}
