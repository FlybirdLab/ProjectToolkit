#include "PurcCtx.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/types/DateTime.hpp"
#include "toolkit/types/StringView.hpp"

#include "purc/ASTNodes.hpp"
#include "purc/MarcoPicker.hpp"
#include "purc/Source.hpp"
#include "purc/buffering/PurcBuffer.hpp"
#include <filesystem>

namespace toolkit {

    TkString PurcCtx::parseRelivite(const TkStringView& v) const {
        TkString realPath{};
        bool r = std::filesystem::path(v.get()).is_relative(); //TODO use cbase api replace later
        if (!r) {
            realPath = v;
        }
        else {
            realPath = TkString::format("{0}/{1}", m_data.rootPath, v);
        }
        return realPath;
    }

    static bool compareFile(const TkStringView& path, PurcBuffer::FileInfo& ifo) {
        auto time = TkFileSystem::getLastEdit(path).getTicks();
        auto size = TkFileSystem::getFileSize(path);

        return ifo.fileSize == size && ifo.fileLastEdit == time;
    }

    PurcCtx::PurcCtx(PurcCtxData dir)
        : m_data{ std::move(dir) }
    {
        m_buffer = TkMemory::newObject<MemoryPurcBuffer>(TkMemory::getDefault(), TkStringView::Empty, this);
    }
    PurcCtx::~PurcCtx() {
        TkMemory::deletePtr(m_buffer);
        m_files.clearDelete();
        m_bufferedMarcos.clearDelete();
    }


    bool PurcCtx::setUp()
    {
        TkArray<TkString> files;
        if (!TkFileSystem::listDir(files, m_data.rootPath, "*.hpp")) {
            LogError("Failed to list source files in {0}", m_data.rootPath);
            return false;
        }

        for (auto& file : files) {
            LogDebug("[Source {0}]", file);

            auto& r = m_buffer->getFileInfo(file);
            if (r != PurcBuffer::Invalid) {
                auto rs = compareFile(file, r);
                if (rs) {
                    auto* data = m_buffer->getFileData(file);
                    if (data == nullptr) {
                        m_buffer->removeItem(file);
                        goto normalProcessing;
                    }

                    auto* unit = static_cast<ASTTransUnit*>(data);
                    if (unit == nullptr) {
                        m_buffer->removeItem(file);
                        goto normalProcessing;
                    }

                    auto* f = TkMemory::newObject<CXXSourceFile>(TkMemory::getDefault(), file, this);
                    f->m_isProcessed = true;
                    f->m_src = TkString::Empty;
                    f->m_lexer = nullptr;
                    f->m_parser = nullptr;
                    f->m_rootNode = unit;

                    m_files.add(file, f);
                }
                else {
                    m_buffer->removeItem(file);
                    goto normalProcessing;
                }
            }

        normalProcessing:
            auto* sf = TkMemory::newObject<CXXSourceFile>(TkMemory::getDefault(), file, this);
            if (!sf->tryRead()) {
                LogError("Failed to raed file {0} Moc will remove it from queue", file);
                TkMemory::deletePtr(sf);
                continue;
            }
            if (!sf->m_src.startsWith(TkStringView{"#pragma cobj"})) {
                LogDebug("[Not a cobj file]");
                TkMemory::deletePtr(sf);
                continue;
            }

            m_files.add(file, sf);
        }

        return true;
        // System::getFileSize(const TkStringView &path)
    }


    bool PurcCtx::processAll()
    {
        for (auto& item : m_files) {
            if (item.value->m_isProcessed) {
                continue;
            }
            if (!item.value->processFile()) {
                LogError("Failed to process file {0} Moc will remove it from queue", item.key);
                TkMemory::deletePtr(item.value);
                continue;
            }
            PurcBuffer::FileInfo ifo{
                .fileSize = TkFileSystem::getFileSize(item.key),
                .fileLastEdit = TkFileSystem::getLastEdit(item.key).getTicks(),
            };

            if (!m_buffer->tryAddToBuf(item.key, ifo, item.value->m_rootNode)) {
                LogError("Failed to buffer file {0} Moc will remove it from queue", item.key);
                TkMemory::deletePtr(item.value);
                continue;
            }
        }

        return true;
    }


    MarcoPicker* PurcCtx::parseMarcoOnlyFile(const TkStringView& p)
    {
        TkString realPath{};
        bool r = std::filesystem::path(p.get()).is_relative(); //TODO use cbase api replace later
        if (!r) {
            realPath = p;
        }
        else {
            realPath = TkString::format("{0}/{1}", m_data.rootPath, p);
        }

        if (m_bufferedMarcos.find(realPath)) {
            return m_bufferedMarcos[realPath];
        }

        auto* ptr = TkMemory::newObject<MarcoPicker>(TkMemory::getDefault(), realPath, this);
        if (!ptr->tryParse()) {
            LogError("Failed to parse file for marco definations -> {0}", realPath);
            TkMemory::deletePtr(ptr);
            return nullptr;
        }
        m_bufferedMarcos.add(realPath, ptr);

        return ptr;
    }


    bool PurcCtx::hasMarco(const TkStringView& v) {
        for (const auto& item : m_bufferedMarcos) {
            if (item.value->hasMarco(v)) {
                return true;
            }
        }
        return false;
    }

} // namespace cobalt