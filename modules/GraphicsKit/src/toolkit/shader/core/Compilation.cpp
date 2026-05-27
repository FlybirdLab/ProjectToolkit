/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Compilation.hpp"
#include "Compiler.hpp"
#include "Context.hpp"
#include "ShaderMeta.hpp"
#include "toolkit/shader/backends/ShaderCompilerDX.hpp"
#include "toolkit/shader/backends/ShaderCompilerVulkan.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/collection/HashSet.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/threading/Sync.hpp"
#include "toolkit/types/DateTime.hpp"
#include "DebugDataExporter.hpp"
#include "toolkit/shader/backends/ShaderCompilerD3D.hpp"
#include "toolkit/shader/backends/ShaderCompilerGLSL.hpp"
#include "toolkit/io/MemoryStream.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/types/Path.hpp"

#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>
#include <toolkit/shader/source/SourceManager.hpp>

namespace toolkit
{

    static TkMutex s_mtx;
    static TkArray<ShaderCompiler *> s_compilers;
    static TkArray<ShaderCompiler *> s_readyCompilers;

    static TkMutex s_incMtx;
    static TkHashSet<TkString> s_includePaths;
    static TkHashMap<TkString, FileWatcherHandle> s_includeWatchers;

    TkAction<const TkString &, FileAction> ShadersCompilation::onIncludeChanged;

    static void onIncludeWatcherEvent(const TkString &path, FileAction action)
    {
        if (action == FileAction::kDelete)
            return;

        TkString normalizedPath(path);
        TkPath::normalize(normalizedPath);

        {
            TkMutexGuard lock(s_incMtx);
            if (!s_includePaths.contains(normalizedPath))
                return;
        }

        ShadersCompilation::onIncludeChanged(normalizedPath, action);
    }

    bool ShadersCompilation::compile(ShaderCompilationOptions &options)
    {
        PROFILE_NAMED("Shader.Compile");

        DiagnosticSink *sink = options.diagnostics;
        SourceRange primaryRange = SourceRange::invalid();
        if (options.primaryFileId != 0)
        {
            SourceLocation loc;
            loc.fileId = options.primaryFileId;
            primaryRange.begin = loc;
            primaryRange.end = loc;
        }

        // Validate input options
        if (options.targetName.isEmpty() || !options.targetID.isValid())
        {
            LogWarn("Unknown target object.");
            if (sink != nullptr)
                sink->error(primaryRange, TKSHD_INVALID_OPTIONS, "unknown shader target name or id");
            return false;
        }
        if (options.output == nullptr)
        {
            LogWarn("Missing output.");
            if (sink != nullptr)
                sink->error(primaryRange, TKSHD_INVALID_OPTIONS, "missing output stream");
            return false;
        }
        if (options.profile == GfxShaderProfile::kUnknown)
        {
            LogWarn("Unknown shader profile.");
            if (sink != nullptr)
                sink->error(primaryRange, TKSHD_UNSUPPORTED_PROFILE, "unknown shader profile");
            return false;
        }
        if (options.source == nullptr || options.srcLen < 1)
        {
            LogWarn("Missing source code.");
            if (sink != nullptr)
                sink->error(primaryRange, TKSHD_INVALID_OPTIONS, "missing shader source");
            return false;
        }

        // Adjust input source length if it ends with null
        while (options.srcLen > 2 && options.source[options.srcLen - 1] == 0)
            options.srcLen--;

        const TkDateTime startTime = TkDateTime::nowUTC();

        // Process shader source to collect metadata.
        //
        // `.tkshader` is the only authoring path now, and ShaderManifestCompiler
        // always populates `prebuiltEntries` from each pass's stage declarations
        // (or the embedded GLSL VSMain/PSMain convention). The legacy META_VS /
        // META_PS token scanner was removed in the cleanup phase.
        ShaderMeta meta;
        if (options.prebuiltEntries.isEmpty())
        {
            LogWarn("ShaderCompilationOptions::prebuiltEntries is empty -- caller must populate entry points.");
            if (sink != nullptr)
            {
                Diagnostic d;
                d.severity = DiagnosticSeverity::Error;
                d.code = TKSHD_PARSE_FAILED;
                d.message = TkString::format("shader '%s' has no entry points (prebuiltEntries empty)", options.targetName);
                d.range = primaryRange;
                d.backend = TkString(toolkit::toString(options.profile));
                sink->report(d);
            }
            return false;
        }
        for (i32 i = 0; i < options.prebuiltEntries.count(); i++)
        {
            const PrebuiltShaderEntry &e = options.prebuiltEntries[i];
            switch (e.stage)
            {
            case ShaderStage::kVertex:
            {
                VertexShaderMeta vsMeta;
                vsMeta.name = e.name;
                vsMeta.flags = GfxShaderFlags::kDefault;
                vsMeta.minFeatureLevel = GfxFeatureLevel::kSM4;
                vsMeta.permutations.add(ShaderPermutation());
                meta.vs.add(vsMeta);
                break;
            }
            case ShaderStage::kPixel:
            {
                PixelShaderMeta psMeta;
                psMeta.name = e.name;
                psMeta.flags = GfxShaderFlags::kDefault;
                psMeta.minFeatureLevel = GfxFeatureLevel::kSM4;
                psMeta.permutations.add(ShaderPermutation());
                meta.ps.add(psMeta);
                break;
            }
            case ShaderStage::kHull:
            {
                HullShaderMeta hsMeta;
                hsMeta.name = e.name;
                hsMeta.flags = GfxShaderFlags::kDefault;
                hsMeta.minFeatureLevel = GfxFeatureLevel::kSM5;
                hsMeta.permutations.add(ShaderPermutation());
                hsMeta.controlPointsCount = 0;
                meta.hs.add(hsMeta);
                break;
            }
            case ShaderStage::kDomain:
            {
                DomainShaderMeta dsMeta;
                dsMeta.name = e.name;
                dsMeta.flags = GfxShaderFlags::kDefault;
                dsMeta.minFeatureLevel = GfxFeatureLevel::kSM5;
                dsMeta.permutations.add(ShaderPermutation());
                meta.ds.add(dsMeta);
                break;
            }
            case ShaderStage::kGeometry:
            {
                GeometryShaderMeta gsMeta;
                gsMeta.name = e.name;
                gsMeta.flags = GfxShaderFlags::kDefault;
                gsMeta.minFeatureLevel = GfxFeatureLevel::kSM4;
                gsMeta.permutations.add(ShaderPermutation());
                meta.gs.add(gsMeta);
                break;
            }
            case ShaderStage::kCompute:
            {
                ComputeShaderMeta csMeta;
                csMeta.name = e.name;
                csMeta.flags = GfxShaderFlags::kDefault;
                csMeta.minFeatureLevel = GfxFeatureLevel::kSM5;
                csMeta.permutations.add(ShaderPermutation());
                meta.cs.add(csMeta);
                break;
            }
            default:
                break;
            }
        }
        const i32 shadersCount = meta.getShadersCount();
        if (shadersCount == 0)
        {
            LogWarn("Shader has no valid functions.");
            if (sink != nullptr)
                sink->warning(primaryRange, TKSHD_INVALID_OPTIONS, "shader has no valid functions");
        }

        // Perform actual compilation
        bool result;
        {
            ShaderCompilationContext context(&options, &meta);

            // Request shaders compiler
            auto compiler = requestCompiler(options.profile);
            if (compiler == nullptr)
            {
                LogError("Shader compiler request failed.");
                if (sink != nullptr)
                {
                    Diagnostic d;
                    d.severity = DiagnosticSeverity::Error;
                    d.code = TKSHD_UNSUPPORTED_PROFILE;
                    d.message = TkString::format("no compiler registered for profile '%s'", toolkit::toString(options.profile));
                    d.range = primaryRange;
                    d.backend = TkString(toolkit::toString(options.profile));
                    sink->report(d);
                }
                return false;
            }
            TK_ASSERT(compiler->getProfile() == options.profile);

            // Call compilation process
            result = compiler->compile(&context);

            // Dismiss compiler
            freeCompiler(compiler);

            // Hand the backend-populated reflection back to the caller via
            // `options`. The context is destroyed at the end of this scope --
            // copying here is the seam that keeps backends from needing a
            // const_cast on the options pointer (they write into the
            // context's own arrays instead). Other backends that don't
            // produce reflection leave the context arrays empty, which is
            // the same observable state as before.
            options.reflectionBindings = context.reflectionBindings;
            options.reflectionVertexInputs = context.reflectionVertexInputs;

#if GFX_USE_SHADERS_DEBUG_LAYER
            // Export debug data
            ShaderDebugDataExporter::exportData(&context);
#endif

            for (auto &include : context.includes)
                registerIncludeForReloads(include.item);
        }

        if (!result)
        {
            LogError("Shader compilation '%s' failed (profile: %s)", options.targetName, toolkit::toString(options.profile));
            if (sink != nullptr)
            {
                Diagnostic d;
                d.severity = DiagnosticSeverity::Error;
                d.code = TKSHD_BACKEND_COMPILE_FAILED;
                d.message = TkString::format("shader '%s' compilation failed", options.targetName);
                d.range = primaryRange;
                d.backend = TkString(toolkit::toString(options.profile));
                sink->report(d);
            }
        }
        else
        {
            // Success
            const TkDateTime endTime = TkDateTime::nowUTC();
            LogInfo("Shader compilation '%s' succeed in %i ms (profile: %s)", options.targetName, TkMath::ceilToInt(static_cast<float>((endTime - startTime).getTotalMilliseconds())), toolkit::toString(options.profile));
        }

        return result;
    }

    ShaderCompiler *ShadersCompilation::createCompiler(GfxShaderProfile profile)
    {
        ShaderCompiler *result = nullptr;

        switch (profile)
        {
        case GfxShaderProfile::kDirectX_SM4:
        case GfxShaderProfile::kDirectX_SM5:
            result = TkMemory::newObject<ShaderCompilerD3D>(TkMemory::getDefault(), profile);
            // Legacy split marker:
            // TK_ASSERT(false && "Not impl.");
            break;
        case GfxShaderProfile::kDirectX_SM6:
            result = TkMemory::newObject<ShaderCompilerDX>(TkMemory::getDefault(), profile);
            break;
        case GfxShaderProfile::kVulkan_SM5:
            result = TkMemory::newObject<ShaderCompilerVulkan>(TkMemory::getDefault(), profile);
            break;
        case GfxShaderProfile::kGLSL_410:
            result = TkMemory::newObject<ShaderCompilerGLSL>(TkMemory::getDefault(), profile);
            break;
        default:
            break;
        }
        TK_ASSERT(result == nullptr || result->getProfile() == profile);

        return result;
    }

    ShaderCompiler *ShadersCompilation::requestCompiler(GfxShaderProfile profile)
    {
        ShaderCompiler *compiler;
        TkMutexGuard lock(s_mtx);

        // Try to find ready compiler
        for (i32 i = 0; i < s_readyCompilers.count(); i++)
        {
            compiler = s_readyCompilers[i];
            if (compiler->getProfile() == profile)
            {
                // Use it
                s_readyCompilers.removeAt(i);
                return compiler;
            }
        }

        // Create new compiler for a target profile
        compiler = createCompiler(profile);
        if (compiler == nullptr)
        {
            LogError("Cannot create Shader Compiler for profile %i", profile);
            return nullptr;
        }

        // Register new compiler
        s_compilers.add(compiler);

        return compiler;
    }

    void ShadersCompilation::freeCompiler(ShaderCompiler *compiler)
    {
        TkMutexGuard lock(s_mtx);
        TK_ASSERT(compiler && s_readyCompilers.contains(compiler) == false);

        // Check if service has been disposed (this compiler is not in the compilers list)
        if (!s_compilers.contains(compiler))
        {
            // Delete it manually
            TkMemory::deletePtr(compiler);
        }
        else
        {
            // Make compiler free again
            s_readyCompilers.add(compiler);
        }
    }

    /*
    Legacy engine asset hot-reload path.
    Disabled after ShaderCompiler was split into a standalone tool. The standalone
    compiler keeps include dependency data in the cache, but no longer owns engine
    assets or editor reload/import behavior.

        TkMutex s_incMtx;
        TkHashMap<TkString, Array<Asset *>> ShaderIncludesMap;
        TkHashMap<TkString, FileSystemWatcher *> ShaderIncludesWatcher;

        void OnShaderIncludesWatcherEvent(const String &path, FileSystemAction action)
        {
            if (action == FileSystemAction::Delete)
                return;

            TkArray<Asset *> toReload;
            {
                TkMutexGuard lock(s_incMtx);

                auto file = ShaderIncludesMap.Find(path);
                if (file == ShaderIncludesMap.End())
                    return;
                toReload = file->Value;
            }

            for (Asset *asset : Content::GetAssets())
            {
                if (asset->LastLoadFailed() && !toReload.Contains(asset))
                {
                    if (asset->Is<Shader>() || asset->Is<Material>() || asset->Is<ParticleEmitter>())
                        toReload.Add(asset);
                }
            }

            LogInfo("Shader include \'{0}\' has been modified.", path);
            TkThread::sleep(100);

            for (Asset *asset : toReload)
            {
                asset->Reload();
            }
        }

        void ShadersCompilation::RegisterForShaderReloads(Asset *asset, const String &includedPath)
        {
            TkMutexGuard lock(s_incMtx);

            const bool alreadyAdded = ShaderIncludesMap.ContainsKey(includedPath);
            auto &file = ShaderIncludesMap[includedPath];
            TK_ASSERT(!file.Contains(asset));
            file.Add(asset);

            if (!alreadyAdded)
            {
                const String directory = StringUtils::GetDirectoryName(includedPath);
                if (FileSystem::DirectoryExists(directory) && !ShaderIncludesWatcher.ContainsKey(directory))
                {
                    auto watcher = New<FileSystemWatcher>(directory, false);
                    watcher->OnEvent.Bind<OnShaderIncludesWatcherEvent>();
                    ShaderIncludesWatcher.Add(directory, watcher);
                }
            }
        }

        void ShadersCompilation::UnregisterForShaderReloads(Asset *asset)
        {
            TkMutexGuard lock(s_incMtx);

            for (auto &file : ShaderIncludesMap)
            {
                file.Value.remove(asset);
            }
        }
    */

    void ShadersCompilation::registerIncludeForReloads(TkStringView includedPath)
    {
        TkString path = resolveShaderPath(includedPath);
        if (path.isEmpty())
            return;

        TkString directory(TkPath::getDir(path));
        if (directory.isEmpty())
            return;

        TkMutexGuard lock(s_incMtx);

        s_includePaths.add(path);
        if (s_includeWatchers.containsKey(directory))
            return;

        FileWatcherHandle watcher = TkFileSystem::openFileWatcher(directory, false);
        if (!TkFileSystem::isEnabled(watcher))
            return;

        TkFileSystem::getEvent(watcher).bind(onIncludeWatcherEvent);
        s_includeWatchers.add(directory, watcher);
    }

    void ShadersCompilation::unregisterIncludeForReloads(TkStringView includedPath)
    {
        TkString path = resolveShaderPath(includedPath);

        TkMutexGuard lock(s_incMtx);
        s_includePaths.remove(path);
    }

    void ShadersCompilation::extractShaderIncludes(byte *shaderCache, i32 shaderCacheLength, TkArray<TkString> &includes)
    {
        MemoryIStream stream(shaderCache, shaderCacheLength);

        // Read cache format version
        i32 version;
        stream.readI32(&version);
        if (version != GFX_SHADER_CACHE_VERSION)
        {
            return;
        }

        // Read the location of additional data that contains list of included source files
        i32 additionalDataStart;
        stream.readI32(&additionalDataStart);
        stream.setPos(additionalDataStart);

        // Read all includes
        i32 includesCount;
        stream.readI32(&includesCount);
        includes.clear();
        for (i32 i = 0; i < includesCount; i++)
        {
            TkString &include = includes.addOne();
            stream.read(include, 11);
            include = ShadersCompilation::resolveShaderPath(include);
            TkDateTime lastEditTime;
            stream.read(lastEditTime);
        }
    }

    TkString ShadersCompilation::resolveShaderPath(TkStringView path)
    {
        TkString result(path);

        // Keep paths from old compact caches usable by stripping nested './'.
        for (i32 i = path.length() - 2; i >= 2; i--)
        {
            if (path[i] == '.' && path[i + 1] == '/')
            {
                result = path.substring(i);
                break;
            }
        }

        /*
        Legacy engine project path resolver.
        Disabled in standalone mode because it depends on Globals::StartupFolder,
        Editor::Project, ProjectInfo and Flax platform project aliases.

            ./<ProjectName>/ShaderFile.hlsl -> <Project>/Source/Shaders/ShaderFile.hlsl
            ./Flax/...                     -> Globals::StartupFolder/Source/Shaders/...

            String result;
            if (path.StartsWith(StringView(TEXT("./"), 2)))
            {
                i32 projectNameEnd = -1;
                for (i32 i = 2; i < path.Length(); i++)
                {
                    if (path[i] == '/')
                    {
                        projectNameEnd = i;
                        break;
                    }
                }
                if (projectNameEnd == -1)
                    return String::Empty;
                StringView projectName = path.Substring(2, projectNameEnd - 2);
                if (projectName.StartsWith(StringView(TEXT("FlaxPlatforms"))))
                {
                    result = Globals::StartupFolder / TEXT("Source/Platforms");
                }
                else
                {
                    HashSet<const ProjectInfo *> projects;
                    const ProjectInfo *project = FindProjectByName(Editor::Project, projects, StringView(projectName.get(), projectNameEnd - 2));
                    if (project)
                        result = project->ProjectFolderPath / TEXT("/Source/Shaders/");
                    else
                        return String::Empty;
                }
                result /= path.Substring(projectNameEnd + 1);
            }
            else if (path.StartsWith(StringView(TEXT("./Flax/"), 7)))
            {
                result = Globals::StartupFolder / TEXT("Source/Shaders") / path.Substring(6);
            }
            else
            {
                result = path;
            }
        */

        if (TkPath::isRelative(result))
            result = System::getWorkDir() / result;

        TkPath::normalize(result);

        return result;
    }

    TkString ShadersCompilation::compactShaderPath(TkStringView path)
    {
        /*
        Legacy engine project path compactor.
        Disabled in standalone mode because there is no engine project registry to
        map absolute paths back to ./<ProjectName>/... aliases.

            HashSet<const ProjectInfo *> projects;
            const ProjectInfo *project = FindProjectByPath(Editor::Project, projects, path);
            if (project)
            {
                String projectSourcesPath = project->ProjectFolderPath / TEXT("/Source/Shaders/");
                if (path.StartsWith(projectSourcesPath))
                    return String::Format(TEXT("./{}/{}"), project->Name, path.Substring(projectSourcesPath.Length()));
            }
        */

        TkString result(path);
        TkPath::normalize(result);
        return result;
    }

    /*
    Legacy editor source watcher and asset import path.
    Disabled after ShaderCompiler became a standalone tool. The old behavior
    watched Source/Shaders, generated stable asset ids and imported .shader files
    into Content/Shaders through AssetsImportingManager.

    namespace
    {
        Array<FileSystemWatcher *> ShadersSourcesWatchers;

        // Tries to generate a stable and unique ID for the given shader name.
        // Used in order to keep the same shader IDs and reduce version control issues with binary diff on ID.
        Guid GetShaderAssetId(const String &name)
        {
            Guid result;
            result.A = name.Length() * 100;
            result.B = GetHash(name);
            result.C = name.HasChars() ? name[0] : 0;
            result.D = name.HasChars() ? name[name.Length() - 1] : 0;
            return result;
        }

        void OnWatcherShadersEvent(const String &path, FileSystemAction action)
        {
            if (action == FileSystemAction::Delete || !path.EndsWith(TEXT(".shader")))
                return;

            LogInfo("Shader \'{0}\' has been modified.", path);

            // Wait a little so app that was editing the file (e.g. Visual Studio, Notepad++) has enough time to flush whole file change
            Platform::Sleep(100);

            // Perform hot reload
            const i32 srcSubDirStart = path.FindLast(TEXT("/Source/Shaders"));
            if (srcSubDirStart == -1)
                return;
            String projectFolderPath = path.Substring(0, srcSubDirStart);
            FileSystem::NormalizePath(projectFolderPath);
            const String shadersAssetsPath = projectFolderPath / TEXT("/Content/Shaders");
            const String shadersSourcePath = projectFolderPath / TEXT("/Source/Shaders");
            const String localPath = FileSystem::ConvertAbsolutePathToRelative(shadersSourcePath, path);
            const String name = StringUtils::GetPathWithoutExtension(localPath);
            const String outputPath = shadersAssetsPath / name + ASSET_FILES_EXTENSION_WITH_DOT;
            Guid id = GetShaderAssetId(name);
            AssetsImportingManager::ImportIfEdited(path, outputPath, id);
        }

        void RegisterShaderWatchers(const ProjectInfo *project, HashSet<const ProjectInfo *> &projects)
        {
            if (projects.Contains(project))
                return;
            projects.Add(project);

            // Check if project uses shaders sources
            const String shadersSourcePath = project->ProjectFolderPath / TEXT("/Source/Shaders");
            if (FileSystem::DirectoryExists(shadersSourcePath))
            {
                // Track engine shaders editing
                auto sourceWatcher = New<FileSystemWatcher>(shadersSourcePath, true);
                sourceWatcher->OnEvent.Bind<OnWatcherShadersEvent>();
                ShadersSourcesWatchers.Add(sourceWatcher);

                // Reimport modified or import added shaders
                Array<String> files(64);
                const String shadersAssetsPath = project->ProjectFolderPath / TEXT("/Content/Shaders");
                FileSystem::DirectoryGetFiles(files, shadersSourcePath, TEXT("*.shader"), DirectorySearchOption::AllDirectories);
                for (i32 i = 0; i < files.count(); i++)
                {
                    const String &path = files[i];
                    const String localPath = FileSystem::ConvertAbsolutePathToRelative(shadersSourcePath, path);
                    const String name = StringUtils::GetPathWithoutExtension(localPath);
                    const String outputPath = shadersAssetsPath / name + ASSET_FILES_EXTENSION_WITH_DOT;
                    Guid id = GetShaderAssetId(name);
                    AssetsImportingManager::ImportIfEdited(path, outputPath, id);
                }
            }

            // Initialize referenced projects
            for (const auto &reference : project->References)
            {
                if (reference.Project)
                    RegisterShaderWatchers(reference.Project, projects);
            }
        }
    }
    */

    void ShadersCompilation::init()
    {
    }

    void ShadersCompilation::dispose()
    {
        s_mtx.lock();

        // Check if any compilation is running
        if (s_readyCompilers.count() != s_compilers.count())
        {
            LogError("Cannot dispose Shaders Compilation Service. One or more compilers are still in use.");
        }

        // Cleanup all compilers (delete only those which are not in use)
        s_readyCompilers.clearDelete();
        s_compilers.clear();

        s_mtx.unlock();

        // Cleanup shader includes
        ShaderCompiler::disposeIncludedFilesCache();

        s_incMtx.lock();
        for (auto it = s_includeWatchers.Begin(); it.isNotEnd(); ++it)
            TkFileSystem::closeFileWatcher(it->value);
        s_includeWatchers.clear();
        s_includePaths.clear();
        s_incMtx.unlock();
    }

    void ShaderCompilationContext::onError(const char *message)
    {
        LogError("Failed to compile '%s'. %s", options->targetName, message);

        // Route the backend's textual error through the structured diagnostic
        // sink when one is wired up. The location is anchored at the primary
        // input file (we usually don't have a finer range from the backend).
        if (options != nullptr && options->diagnostics != nullptr)
        {
            SourceRange range = SourceRange::invalid();
            if (options->primaryFileId != 0)
            {
                SourceLocation loc;
                loc.fileId = options->primaryFileId;
                range.begin = loc;
                range.end = loc;
            }

            Diagnostic d;
            d.severity = DiagnosticSeverity::Error;
            d.code = TKSHD_BACKEND_COMPILE_FAILED;
            d.message = TkString(message);
            d.range = range;
            d.backend = TkString(toolkit::toString(options->profile));
            options->diagnostics->report(d);
        }
    }

    DiagnosticSink *ShaderCompilationContext::diagnostics() const
    {
        return options != nullptr ? options->diagnostics : nullptr;
    }

    SourceManager *ShaderCompilationContext::sourceManager() const
    {
        return options != nullptr ? options->sourceManager : nullptr;
    }

    u32 ShaderCompilationContext::primaryFileId() const
    {
        return options != nullptr ? options->primaryFileId : 0;
    }

    void ShaderCompilationContext::onCollectDebugInfo(ShaderFunctionMeta &meta, i32 permutationIndex, const char *data, const i32 dataLength)
    {
#ifdef GFX_USE_SHADERS_DEBUG_LAYER

        // Cache data
        meta.permutations[permutationIndex].debugData.set(data, dataLength);

#endif
    }

    ShaderCompilationContext::ShaderCompilationContext(const ShaderCompilationOptions *options, ShaderMeta *meta)
        : options(options), meta(meta), output(options->output)
    {
        const i32 nameLen = options->targetName.length();
        const i32 copyLen = nameLen < static_cast<i32>(lengthOf(targetNameAnsi) - 1) ? nameLen : static_cast<i32>(lengthOf(targetNameAnsi) - 1);
        for (i32 i = 0; i < copyLen; i++)
            targetNameAnsi[i] = options->targetName[i];
        targetNameAnsi[copyLen] = 0;
    }

}
