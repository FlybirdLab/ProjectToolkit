/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Compiler.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/DateTime.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/io/MemoryStream.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "Context.hpp"
#include "Compilation.hpp"

namespace toolkit
{
    struct File
    {
        TkString path;
        TkDateTime lastEditTime;
        TkString source;
    };

    static TkMutex s_mtx;
    static TkHashMap<TkString, File *> s_files;

    bool ShaderCompiler::compile(ShaderCompilationContext *context)
    {
        // Clear cache
        m_globalMacros.clear();
        m_macros.clear();
        m_constantBuffers.clear();
        m_globalMacros.ensureCapacity(32);
        m_macros.ensureCapacity(32);
        m_context = context;

        // Prepare
        auto output = context->output;
        auto meta = context->meta;
        const i32 shadersCount = meta->getShadersCount();
        if (!onCompileBegin())
            return false;
        m_globalMacros.add({nullptr, nullptr});

        // Setup constant buffers cache
        m_constantBuffers.ensureCapacity(meta->cb.count(), false);
        for (i32 i = 0; i < meta->cb.count(); i++)
            m_constantBuffers.add({meta->cb[i].slot, false, 0});

        // [Output] Version number
        output->writeI32(GFX_SHADER_CACHE_VERSION);

        // [Output] Additional data start
        const i32 additionalDataStartPos = output->getPosition();
        output->writeI32(-1);

        // [Output] Amount of shaders
        output->writeI32(shadersCount);

        // Compile all shaders
        if (!compileShaders())
            return false;

        // [Output] Constant Buffers
        output->write((byte)m_constantBuffers.count());
        for (const ShaderResourceBuffer &cb : m_constantBuffers)
        {
            output->write((byte)cb.slot);
            output->write((u32)cb.size);
        }

        // Additional Data Start
        *(i32 *)(output->getHandle() + additionalDataStartPos) = output->getPosition();

        // [Output] Includes
        output->writeI32(context->includes.count());
        for (auto &include : context->includes)
        {
            TkString compactPath = ShadersCompilation::compactShaderPath(include.item);
            output->write(compactPath, 11);
            const auto date = TkFileSystem::getLastEdit(include.item);
            output->write(date);
        }

        return true;
    }

    bool ShaderCompiler::getIncludedFileSource(ShaderCompilationContext *context, const char *sourceFile, const char *includedFile, const char *&source, i32 &sourceLength)
    {
        PROFILE();
        source = nullptr;
        sourceLength = 0;

        // Get actual file path
        const TkString includedFileName(includedFile);
        TkString path = ShadersCompilation::resolveShaderPath(includedFileName);
        if (!TkFileSystem::isFileExist(path))
        {
            LogError("Unknown shader source file '%s' included in '%s'.%s", includedFileName, sourceFile, TkString::Empty);
            return false;
        }

        TkMutexGuard lock(toolkit::s_mtx);

        // Try to reuse file
        toolkit::File *result = nullptr;
        auto r = s_files.containsKey(path);
        if (r)
        {
            result = s_files[path];
        }
        if (!r || TkFileSystem::getLastEdit(path) > result->lastEditTime)
        {
            // Remove old one
            if (result)
            {
                TkMemory::deletePtr(result);
                toolkit::s_files.remove(path);
            }

            // Load file
            result = TkMemory::newObject<toolkit::File>();
            result->path = path;
            result->lastEditTime = TkFileSystem::getLastEdit(path);
            OpenFileParams openParams{
                .path = result->path,
                .mode = FileMode::kOpenExisting,
                .access = FileAccess::kRead,
                .share = FileShare::kRead,
            };
            auto handle = TkFileSystem::openFile(openParams);
            if (!TkFileSystem::readAsText(handle, result->source))
            {
                TkFileSystem::closeFile(handle);
                LogError("Failed to load shader source file '%s' included in '%s' (path: '%s')", includedFile, sourceFile, path);
                TkMemory::deletePtr(result);
                return false;
            }
            TkFileSystem::closeFile(handle);
            toolkit::s_files.add(path, result);
        }

        context->includes.add(path);

        // Copy to output
        source = result->source.get();
        sourceLength = result->source.length();
        return true;
    }

    void ShaderCompiler::disposeIncludedFilesCache()
    {
        TkMutexGuard lock(toolkit::s_mtx);

        toolkit::s_files.clearDelete();
    }

    bool ShaderCompiler::compileShaders()
    {
        auto meta = m_context->meta;
#if TK_DEBUG && TK_PROFILE
#define PROFILE_COMPILE_SHADER(s) ZoneTransientN(___tracy_scoped_zone, s.name.get(), true);
#else
#define PROFILE_COMPILE_SHADER(s)
#endif

        // Generate vertex shaders cache
        for (i32 i = 0; i < meta->vs.count(); i++)
        {
            auto &shader = meta->vs[i];
            TK_ASSERT(shader.getStage() == ShaderStage::kVertex && (shader.flags & GfxShaderFlags::kHidden) == (GfxShaderFlags)0);
            PROFILE_COMPILE_SHADER(shader);
            if (!compileShader(shader, &writeCustomDataVS))
            {
                LogError("Failed to compile \'%s\'", shader.name);
                return false;
            }
        }

        // Generate hull shaders cache
        for (i32 i = 0; i < meta->hs.count(); i++)
        {
            auto &shader = meta->hs[i];
            TK_ASSERT(shader.getStage() == ShaderStage::kHull && (shader.flags & GfxShaderFlags::kHidden) == (GfxShaderFlags)0);
            PROFILE_COMPILE_SHADER(shader);
            if (!compileShader(shader, &writeCustomDataHS))
            {
                LogError("Failed to compile \'%s\'", shader.name);
                return false;
            }
        }

        // Generate domain shaders cache
        for (i32 i = 0; i < meta->ds.count(); i++)
        {
            auto &shader = meta->ds[i];
            TK_ASSERT(shader.getStage() == ShaderStage::kDomain && (shader.flags & GfxShaderFlags::kHidden) == (GfxShaderFlags)0);
            PROFILE_COMPILE_SHADER(shader);
            if (!compileShader(shader))
            {
                LogError("Failed to compile \'%s\'", shader.name);
                return false;
            }
        }

        // Generate geometry shaders cache
        for (i32 i = 0; i < meta->gs.count(); i++)
        {
            auto &shader = meta->gs[i];
            TK_ASSERT(shader.getStage() == ShaderStage::kGeometry && (shader.flags & GfxShaderFlags::kHidden) == (GfxShaderFlags)0);
            PROFILE_COMPILE_SHADER(shader);
            if (!compileShader(shader))
            {
                LogError("Failed to compile \'%s\'", shader.name);
                return false;
            }
        }

        // Generate pixel shaders cache
        for (i32 i = 0; i < meta->ps.count(); i++)
        {
            auto &shader = meta->ps[i];
            TK_ASSERT(shader.getStage() == ShaderStage::kPixel && (shader.flags & GfxShaderFlags::kHidden) == (GfxShaderFlags)0);
            PROFILE_COMPILE_SHADER(shader);
            if (!compileShader(shader))
            {
                LogError("Failed to compile \'%s\'", shader.name);
                return false;
            }
        }

        // Generate compute shaders cache
        for (i32 i = 0; i < meta->cs.count(); i++)
        {
            auto &shader = meta->cs[i];
            TK_ASSERT(shader.getStage() == ShaderStage::kCompute && (shader.flags & GfxShaderFlags::kHidden) == (GfxShaderFlags)0);
            PROFILE_COMPILE_SHADER(shader);
            if (!compileShader(shader))
            {
                LogError("Failed to compile \'%s\'", shader.name);
                return false;
            }
        }

#undef PROFILE_COMPILE_SHADER
        return true;
    }

    bool ShaderCompiler::onCompileBegin()
    {
        // Setup global macros
        static const char *Numbers[] =
            {
                // @formatter:off
                "0",
                "1",
                "2",
                "3",
                "4",
                "5",
                "6",
                "7",
                "8",
                "9",
                "10",
                // @formatter:on
            };
        const auto profile = getProfile();
        const auto featureLevel = GfxUtils::getFeatureLevel(profile);
        m_globalMacros.add({"FEATURE_LEVEL", Numbers[(i32)featureLevel]});

        return true;
    }

    bool ShaderCompiler::onCompileEnd()
    {
        return true;
    }

    bool ShaderCompiler::writeShaderFunctionBegin(ShaderCompilationContext *context, ShaderFunctionMeta &meta)
    {
        auto output = context->output;
        output->write((byte)meta.getStage());
        output->write((byte)meta.permutations.count());
        output->write(meta.name, 11);
        output->write((u32)meta.flags);
        return true;
    }

    bool ShaderCompiler::writeShaderFunctionPermutation(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, const ShaderBindings &bindings, const void *header, i32 headerSize, const void *cache, i32 cacheSize)
    {
        auto output = context->output;
        output->write((u32)(cacheSize + headerSize));
        output->writeBytes(header, headerSize);
        output->writeBytes(cache, cacheSize);
        output->writeBytes(&bindings, sizeof(bindings));
        return true;
    }

    bool ShaderCompiler::writeShaderFunctionPermutation(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, const ShaderBindings &bindings, const void *cache, i32 cacheSize)
    {
        auto output = context->output;
        output->write((u32)cacheSize);
        output->writeBytes(cache, cacheSize);
        output->writeBytes(&bindings, sizeof(bindings));
        return true;
    }

    bool ShaderCompiler::writeShaderFunctionEnd(ShaderCompilationContext *context, ShaderFunctionMeta &meta)
    {
        return true;
    }

    bool ShaderCompiler::writeCustomDataVS(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, const TkArray<ShaderMacro> &macros, void *additionalData)
    {
        auto output = context->output;

        // Write vertex shader inputs (based on compiled shader reflection) to bind any missing vertex buffer streaming at runtime (during drawing - see usage of GPUVertexLayout::Merge)
        if (auto *additionalDataVS = (AdditionalDataVS *)additionalData)
        {
            output->writeI32(additionalDataVS->inputs.count());
            for (i32 i = 0; i < additionalDataVS->inputs.count(); i++)
                output->write(additionalDataVS->inputs[i]);
        }
        else
            output->writeI32(0);

        // [Deprecated in v1.10]
        auto &metaVS = *(VertexShaderMeta *)&meta;
        auto &layout = metaVS.inputLayout;

        // Get visible entries (based on `visible` flag switch)
        i32 layoutSize = 0;
        bool layoutVisible[GFX_MAX_VS_ELEMENTS];
        for (i32 i = 0; i < layout.count(); i++)
        {
            auto &element = layout[i];
            layoutVisible[i] = false;

            // Parse using all input macros
            TkString value = element.visibleFlag;
            for (i32 j = 0; j < macros.count() - 1; j++)
            {
                if (macros[j].name == value)
                {
                    value = macros[j].definition;
                    break;
                }
            }

            if (value == "true" || value == "1")
            {
                // Visible
                layoutSize++;
                layoutVisible[i] = true;
            }
            else if (value == "false" || value == "0")
            {
                // Hidden
            }
            else
            {
                LogError("Invalid option value \'%s\' for  layout element \'visible\' flag on vertex shader \'%s\'.", metaVS.name, value);
                return false;
            }
        }

        // [Output] Input Layout
        output->writeByte(static_cast<byte>(layoutSize));
        for (i32 a = 0; a < layout.count(); a++)
        {
            auto &element = layout[a];
            if (!layoutVisible[a])
                continue;
            VertexElement data;
            switch (element.type)
            {
            case VertexShaderMeta::InputType::kPOSITION:
                data.type = VertexElementTypes::kPosition;
                break;
            case VertexShaderMeta::InputType::kCOLOR:
                data.type = VertexElementTypes::kColor;
                break;
            case VertexShaderMeta::InputType::kTEXCOORD:
                switch (element.index)
                {
                case 0:
                    data.type = VertexElementTypes::kTexCoord0;
                    break;
                case 1:
                    data.type = VertexElementTypes::kTexCoord1;
                    break;
                case 2:
                    data.type = VertexElementTypes::kTexCoord2;
                    break;
                case 3:
                    data.type = VertexElementTypes::kTexCoord3;
                    break;
                case 4:
                    data.type = VertexElementTypes::kTexCoord4;
                    break;
                case 5:
                    data.type = VertexElementTypes::kTexCoord5;
                    break;
                case 6:
                    data.type = VertexElementTypes::kTexCoord6;
                    break;
                case 7:
                    data.type = VertexElementTypes::kTexCoord7;
                    break;
                default:
                    LogError("Vertex Shader '%s' (asset '%s') uses deprecated texcoord attribute index. Valid range is 0-7.", meta.name, context->options->targetName);
                    data.type = VertexElementTypes::kTexCoord;
                    break;
                }
                break;
            case VertexShaderMeta::InputType::kNORMAL:
                data.type = VertexElementTypes::kNormal;
                break;
            case VertexShaderMeta::InputType::kTANGENT:
                data.type = VertexElementTypes::kTangent;
                break;
            case VertexShaderMeta::InputType::kBITANGENT:
                LogError("Vertex Shader '%s' (asset '%s') uses deprecated attribute 'BITANGENT'. Remapping it to `ATTRIBUTE`.", meta.name, context->options->targetName);
                data.type = VertexElementTypes::kAttribute;
                break;
            case VertexShaderMeta::InputType::kATTRIBUTE:
                switch (element.index)
                {
                case 0:
                    data.type = VertexElementTypes::kAttribute0;
                    break;
                case 1:
                    data.type = VertexElementTypes::kAttribute1;
                    break;
                case 2:
                    data.type = VertexElementTypes::kAttribute2;
                    break;
                case 3:
                    data.type = VertexElementTypes::kAttribute3;
                    break;
                default:
                    LogError("Vertex Shader '%s' (asset '%s') uses deprecated attribute index. Valid range is 0-3.", meta.name, context->options->targetName);
                    data.type = VertexElementTypes::kAttribute;
                    break;
                }
                break;
            case VertexShaderMeta::InputType::kBLENDINDICES:
                data.type = VertexElementTypes::kBlendIndices;
                break;
            case VertexShaderMeta::InputType::kBLENDWEIGHT:
                data.type = VertexElementTypes::kBlendWeights;
                break;
            default:
                data.type = VertexElementTypes::kUnknown;
                break;
            }
            data.slot = element.inputSlot;
            data.offset = element.alignedByteOffset != INPUT_LAYOUT_ELEMENT_ALIGN && element.alignedByteOffset <= kMaxU8 ? element.alignedByteOffset : 0;
            data.perInstance = element.inputSlotClass == INPUT_LAYOUT_ELEMENT_PER_INSTANCE_DATA;
            data.format = element.format;
            output->write(data);
        }

        return true;
    }

    bool ShaderCompiler::writeCustomDataHS(ShaderCompilationContext *context, ShaderFunctionMeta &meta, i32 permutationIndex, const TkArray<ShaderMacro> &macros, void *additionalData)
    {
        auto output = context->output;
        auto &metaHS = *(HullShaderMeta *)&meta;

        // [Output] Control Points Count
        output->writeI32(metaHS.controlPointsCount);

        return true;
    }

    const char *ShaderCompiler::getShaderMetaDefinesPreamble()
    {
        return
            "#define META_CB_BEGIN(slot,name)\n"
            "#define META_CB_END\n"
            "#define META_VS(visible,level)\n"
            "#define META_HS(visible,level)\n"
            "#define META_DS(visible,level)\n"
            "#define META_GS(visible,level)\n"
            "#define META_PS(visible,level)\n"
            "#define META_CS(visible,level)\n"
            "#define META_VS_IN_ELEMENT(type,index,format,inputSlot,offset,slotClass,stepRate,visible)\n"
            "#define META_HS_PATCH(controlPoints)\n"
            "#define META_PERMUTATION_1(a)\n"
            "#define META_PERMUTATION_2(a,b)\n"
            "#define META_PERMUTATION_3(a,b,c)\n"
            "#define META_PERMUTATION_4(a,b,c,d)\n"
            "#define META_FLAG(flag)\n";
    }

    void ShaderCompiler::getDefineForFunction(ShaderFunctionMeta &meta, TkArray<ShaderMacro> &macros)
    {
        auto &functionName = meta.name;
        const i32 functionNameLength = functionName.length();
        m_funcNameDefineBuffer.clear();
        m_funcNameDefineBuffer.ensureCapacity(functionNameLength + 2);
        m_funcNameDefineBuffer.add('_');
        m_funcNameDefineBuffer.add(functionName.get(), functionNameLength);
        m_funcNameDefineBuffer.add('\0');
        macros.add({m_funcNameDefineBuffer.get(), "1"});
    }

    VertexElementTypes ShaderCompiler::parseVertexElementType(TkStringView semantic, u32 index)
    {
        if (semantic.hasChars() && TkStringUtils::isDigit(semantic[semantic.length() - 1]))
        {
            // Get index from end of the name
            index = semantic[semantic.length() - 1] - '0';
            semantic = TkStringView(semantic.get(), semantic.length() - 1);
        }

        if (semantic == "POSITION")
            return VertexElementTypes::kPosition;
        if (semantic == "COLOR")
            return VertexElementTypes::kColor;
        if (semantic == "NORMAL")
            return VertexElementTypes::kNormal;
        if (semantic == "TANGENT")
            return VertexElementTypes::kTangent;
        if (semantic == "BLENDINDICES")
            return VertexElementTypes::kBlendIndices;
        if (semantic == "LIGHTMAP")
            return VertexElementTypes::kLightmap;
        if (semantic == "BLENDWEIGHTS" ||
            semantic == "BLENDWEIGHT") // [Deprecated in v1.10]
            return VertexElementTypes::kBlendWeights;
        if (semantic == "TEXCOORD" && index < 8)
            return (VertexElementTypes)((i32)VertexElementTypes::kTexCoord0 + index);
        if (semantic == "ATTRIBUTE" && index < 4)
            return (VertexElementTypes)((i32)VertexElementTypes::kAttribute0 + index);

        LogWarn("Unsupported vertex shader input element semantic %s%i", semantic.toString(), index);
        return VertexElementTypes::kUnknown;
    }

}
