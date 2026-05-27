/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Compilation.hpp"
#include "Compiler.hpp"
#include "Context.hpp"
#include "ShaderMeta.hpp"
#include "backends/ShaderCompilerDX.hpp"
#include "backends/ShaderCompilerVulkan.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/collection/HashSet.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/threading/Sync.hpp"
#include "toolkit/types/DateTime.hpp"
#include "DebugDataExporter.hpp"
#include "ShaderProcessing.hpp"
#include "backends/ShaderCompilerD3D.hpp"
#include "backends/ShaderCompilerGLSL.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "toolkit/io/MemoryStream.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/types/Path.hpp"

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

        // Validate input options
        if (options.targetName.isEmpty() || !options.targetID.isValid())
        {
            LogWarn("Unknown target object.");
            return false;
        }
        if (options.output == nullptr)
        {
            LogWarn("Missing output.");
            return false;
        }
        if (options.profile == GfxShaderProfile::kUnknown)
        {
            LogWarn("Unknown shader profile.");
            return false;
        }
        if (options.source == nullptr || options.srcLen < 1)
        {
            LogWarn("Missing source code.");
            return false;
        }

        // Adjust input source length if it ends with null
        while (options.srcLen > 2 && options.source[options.srcLen - 1] == 0)
            options.srcLen--;

        const TkDateTime startTime = TkDateTime::nowUTC();
        const GfxFeatureLevel featureLevel = GfxUtils::getFeatureLevel(options.profile);

        // Process shader source to collect metadata
        ShaderMeta meta;
        if (!Parser::process(options.targetName, options.source, options.srcLen, options.macros, featureLevel, &meta))
        {
            LogWarn("Failed to parse source code.");
            return false;
        }
        const i32 shadersCount = meta.getShadersCount();
        if (shadersCount == 0)
        {
            LogWarn("Shader has no valid functions.");
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
                return false;
            }
            TK_ASSERT(compiler->getProfile() == options.profile);

            // Call compilation process
            result = compiler->compile(&context);

            // Dismiss compiler
            freeCompiler(compiler);

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
