#include "Context.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/types/DateTime.hpp"
#include "toolkit/types/StringView.hpp"

#include "parser/ASTNodes.hpp"
#include "parser/MarcoPicker.hpp"
#include "parser/Source.hpp"
#include "parser/buffering/PurcBuffer.hpp"
#include <filesystem>
#include <string>

namespace toolkit
{
    namespace
    {
        TkString fileStem(const TkString &path)
        {
            return TkString(std::filesystem::path(path.getText()).stem().string().c_str());
        }

        TkString joinPath(const TkString &lhs, const TkString &rhs)
        {
            return TkString(std::filesystem::path(lhs.getText()).append(rhs.getText()).string().c_str());
        }

        TkString normalizeFsPath(const std::filesystem::path &path)
        {
            return TkString(path.lexically_normal().string().c_str());
        }

        bool containsPath(const TkArray<TkString> &paths, const TkString &path)
        {
            for (const TkString &existing : paths)
            {
                if (existing == path)
                    return true;
            }
            return false;
        }

        void addUniquePath(TkArray<TkString> &paths, const TkString &path)
        {
            if (!path.isEmpty() && !containsPath(paths, path))
                paths.add(path);
        }

        TkString makeOutputPath(const ParserContextData &data, const TkString &sourceFile)
        {
            const TkString stem = data.outputName.isEmpty() ? fileStem(sourceFile) : data.outputName;
            return joinPath(data.outputDir, stem + ".rkgen.cpp");
        }

        bool resolveIncludePath(
            const TkString &rootPath,
            const TkString &sourceFile,
            const TkString &includePath,
            TkString &out)
        {
            if (includePath.isEmpty())
                return false;

            const std::filesystem::path includeFs(includePath.getText());
            if (includeFs.is_absolute())
            {
                out = normalizeFsPath(includeFs);
                return TkFileSystem::isFileExist(out);
            }

            const std::filesystem::path sourceDir = std::filesystem::path(sourceFile.getText()).parent_path();
            const TkString sourceRelative = normalizeFsPath(sourceDir / includeFs);
            if (TkFileSystem::isFileExist(sourceRelative))
            {
                out = sourceRelative;
                return true;
            }

            const TkString rootRelative = normalizeFsPath(std::filesystem::path(rootPath.getText()) / includeFs);
            if (TkFileSystem::isFileExist(rootRelative))
            {
                out = rootRelative;
                return true;
            }

            return false;
        }

        void scanIncludes(const TkString &source, TkArray<TkString> &out)
        {
            const std::string text(source.getText(), static_cast<size_t>(source.length()));
            size_t cursor = 0;
            while (cursor < text.size())
            {
                const size_t lineEnd = text.find_first_of("\r\n", cursor);
                const std::string line = text.substr(cursor, lineEnd == std::string::npos ? std::string::npos : lineEnd - cursor);
                cursor = lineEnd == std::string::npos ? text.size() : lineEnd + 1;

                const size_t hash = line.find('#');
                if (hash == std::string::npos)
                    continue;
                const size_t include = line.find("include", hash + 1);
                if (include == std::string::npos)
                    continue;

                const size_t quoteStart = line.find_first_of("\"<", include + 7);
                if (quoteStart == std::string::npos)
                    continue;
                const char closeChar = line[quoteStart] == '"' ? '"' : '>';
                const size_t quoteEnd = line.find(closeChar, quoteStart + 1);
                if (quoteEnd == std::string::npos || quoteEnd <= quoteStart + 1)
                    continue;

                addUniquePath(out, TkString(line.substr(quoteStart + 1, quoteEnd - quoteStart - 1).c_str()));
            }
        }

        void collectDependenciesRecursive(
            const ParserContextData &data,
            const TkString &path,
            TkArray<TkString> &dependencies,
            TkArray<TkString> &visited)
        {
            if (containsPath(visited, path))
                return;
            addUniquePath(visited, path);
            addUniquePath(dependencies, path);

            TkString source;
            OpenFileParams params{ .path = path };
            auto *file = TkFileSystem::openFile(params);
            if (file == nullptr)
                return;
            const bool readOk = TkFileSystem::readAsText(file, source);
            TkFileSystem::closeFile(file);
            if (!readOk)
                return;

            TkArray<TkString> includes;
            scanIncludes(source, includes);
            for (const TkString &includePath : includes)
            {
                TkString resolved;
                if (resolveIncludePath(data.rootPath, path, includePath, resolved))
                    collectDependenciesRecursive(data, resolved, dependencies, visited);
            }
        }

        bool isOutputUpToDate(const ParserContextData &data, const TkString &sourceFile)
        {
            const TkString outputPath = makeOutputPath(data, sourceFile);
            if (!TkFileSystem::isFileExist(outputPath))
                return false;

            const i64 outputTime = TkFileSystem::getLastEdit(outputPath).getTicks();
            TkArray<TkString> dependencies;
            TkArray<TkString> visited;
            collectDependenciesRecursive(data, sourceFile, dependencies, visited);
            if (dependencies.isEmpty())
                return false;

            for (const TkString &dependency : dependencies)
            {
                if (!TkFileSystem::isFileExist(dependency))
                    return false;
                if (TkFileSystem::getLastEdit(dependency).getTicks() > outputTime)
                    return false;
            }
            return true;
        }
    }

    TkString ParserContext::parseRelivite(const TkStringView &v) const
    {
        TkString realPath{};
        bool r = std::filesystem::path(v.get()).is_relative(); // TODO use cbase api replace later
        if (!r)
            realPath = v;
        else
            realPath = TkString::format("%s/%s", m_data.rootPath.getText(), v.getText());
        return realPath;
    }

    static bool compareFile(const TkStringView &path, PurcBuffer::FileInfo &ifo)
    {
        auto time = TkFileSystem::getLastEdit(path).getTicks();
        auto size = TkFileSystem::getFileSize(path);

        return ifo.fileSize == size && ifo.fileLastEdit == time;
    }

    ParserContext::ParserContext(ParserContextData dir)
        : m_data{std::move(dir)}
    {
        m_buffer = TkMemory::newObject<MemoryPurcBuffer>(TkMemory::getDefault(), TkStringView::Empty, this);
    }

    ParserContext::~ParserContext()
    {
        TkMemory::deletePtr(m_buffer);
        m_files.clearDelete();
        m_bufferedMarcos.clearDelete();
    }

    bool ParserContext::setUp()
    {
        TkArray<TkString> files;
        if (!m_data.sourceFiles.isEmpty())
        {
            files.add(m_data.sourceFiles);
        }
        else
        {
            if (!TkFileSystem::listDir(files, m_data.rootPath, "*.hpp"))
            {
                LogError("Failed to list source files in %s", m_data.rootPath.getText());
                return false;
            }

            TkArray<TkString> hFiles;
            if (TkFileSystem::listDir(hFiles, m_data.rootPath, "*.h"))
            {
                files.add(hFiles);
            }
        }

        for (auto &file : files)
        {
            LogDebug("Processing source file %s", file.getText());

            auto &r = m_buffer->getFileInfo(file);
            if (r != PurcBuffer::Invalid)
            {
                auto rs = compareFile(file, r);
                if (rs)
                {
                    auto *data = m_buffer->getFileData(file);
                    if (data == nullptr)
                    {
                        m_buffer->removeItem(file);
                    }
                    else if (auto *unit = static_cast<ASTTransUnit *>(data))
                    {
                        auto *f = TkMemory::newObject<CXXSourceFile>(TkMemory::getDefault(), file, this);
                        f->m_isProcessed = true;
                        f->m_src = TkString::Empty;
                        f->m_lexer = nullptr;
                        f->m_parser = nullptr;
                        f->m_rootNode = unit;

                        m_files.add(file, f);
                        continue;
                    }
                    else
                    {
                        m_buffer->removeItem(file);
                    }
                }
                else
                {
                    m_buffer->removeItem(file);
                }
            }

            auto *sf = TkMemory::newObject<CXXSourceFile>(TkMemory::getDefault(), file, this);
            if (!sf->tryRead())
            {
                LogError("Failed to read file %s. Parser will remove it from queue", file.getText());
                TkMemory::deletePtr(sf);
                continue;
            }
            // if (!sf->m_src.startsWith(TkStringView{"#pragma rkgen"}))
            // {
            //     LogDebug("%s is not an rkgen file", file.getText());
            //     TkMemory::deletePtr(sf);
            //     continue;
            // }

            m_files.add(file, sf);
        }

        return true;
    }

    i32 ParserContext::filterUpToDateOutputs()
    {
        TkArray<TkString> skippedFiles;
        for (auto &item : m_files)
        {
            if (item.value == nullptr)
                continue;
            if (!isOutputUpToDate(m_data, item.key))
                continue;

            LogDebug("skipped up-to-date %s", makeOutputPath(m_data, item.key).getText());
            skippedFiles.add(item.key);
        }

        for (const TkString &file : skippedFiles)
        {
            TkMemory::deletePtr(m_files[file]);
            m_files.remove(file);
        }

        return skippedFiles.count();
    }

    bool ParserContext::processAll()
    {
        for (auto &item : m_files)
        {
            if (item.value->m_isProcessed)
            {
                continue;
            }
            LogDebug("parsing %s", item.key.getText());
            if (!item.value->processFile())
            {
                LogError("Failed to process file %s. Parser will remove it from queue", item.key.getText());
                TkMemory::deletePtr(item.value);
                item.value = nullptr;
                return false;
            }
            LogDebug("parsed %s", item.key.getText());
            PurcBuffer::FileInfo ifo{
                .fileSize = TkFileSystem::getFileSize(item.key),
                .fileLastEdit = TkFileSystem::getLastEdit(item.key).getTicks(),
            };

            if (!m_buffer->tryAddToBuf(item.key, ifo, item.value->m_rootNode))
            {
                LogError("Failed to buffer file %s. Parser will remove it from queue", item.key.getText());
                TkMemory::deletePtr(item.value);
                item.value = nullptr;
                return false;
            }
        }

        return true;
    }

    MarcoPicker *ParserContext::parseMarcoOnlyFile(const TkStringView &p)
    {
        TkString realPath{};
        bool r = std::filesystem::path(p.get()).is_relative(); // TODO use cbase api replace later
        if (!r)
        {
            realPath = p;
        }
        else
        {
            realPath = TkString::format("%s/%s", m_data.rootPath.getText(), p.getText());
        }

        if (m_bufferedMarcos.find(realPath))
        {
            return m_bufferedMarcos[realPath];
        }
        if (!TkFileSystem::isFileExist(realPath))
        {
            return nullptr;
        }

        LogDebug("scanning include macros %s", realPath.getText());
        auto *ptr = TkMemory::newObject<MarcoPicker>(TkMemory::getDefault(), realPath, this);
        if (!ptr->tryParse())
        {
            LogError("Failed to parse file for marco definations -> %s", realPath.getText());
            TkMemory::deletePtr(ptr);
            return nullptr;
        }
        m_bufferedMarcos.add(realPath, ptr);

        return ptr;
    }

    bool ParserContext::hasMarco(const TkStringView &v)
    {
        for (const auto &item : m_bufferedMarcos)
        {
            if (item.value->hasMarco(v))
            {
                return true;
            }
        }
        return false;
    }

} // namespace cobalt
