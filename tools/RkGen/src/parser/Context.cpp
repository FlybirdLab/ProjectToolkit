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

namespace toolkit
{

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
                LogError("[Parser] Failed to list source files in %s", m_data.rootPath.getText());
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
            LogDebug("[Parser] Processing source file %s", file.getText());

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
                LogError("[Parser] Failed to read file %s. Parser will remove it from queue", file.getText());
                TkMemory::deletePtr(sf);
                continue;
            }
            // if (!sf->m_src.startsWith(TkStringView{"#pragma rkgen"}))
            // {
            //     LogDebug("[Parser] %s is not an rkgen file", file.getText());
            //     TkMemory::deletePtr(sf);
            //     continue;
            // }

            m_files.add(file, sf);
        }

        return true;
    }

    bool ParserContext::processAll()
    {
        for (auto &item : m_files)
        {
            if (item.value->m_isProcessed)
            {
                continue;
            }
            if (!item.value->processFile())
            {
                LogError("[Parser] Failed to process file %s. Parser will remove it from queue", item.key.getText());
                TkMemory::deletePtr(item.value);
                item.value = nullptr;
                return false;
            }
            PurcBuffer::FileInfo ifo{
                .fileSize = TkFileSystem::getFileSize(item.key),
                .fileLastEdit = TkFileSystem::getLastEdit(item.key).getTicks(),
            };

            if (!m_buffer->tryAddToBuf(item.key, ifo, item.value->m_rootNode))
            {
                LogError("[Parser] Failed to buffer file %s. Parser will remove it from queue", item.key.getText());
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

        auto *ptr = TkMemory::newObject<MarcoPicker>(TkMemory::getDefault(), realPath, this);
        if (!ptr->tryParse())
        {
            LogError("[Parser] Failed to parse file for marco definations -> %s", realPath.getText());
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
