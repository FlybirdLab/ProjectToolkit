/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/source/SourceManager.hpp>

#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/types/Path.hpp>

namespace toolkit
{
    SourceManager::~SourceManager()
    {
        for (i32 i = 0; i < m_files.count(); i++)
        {
            if (m_files[i] != nullptr)
                TkMemory::deletePtr(m_files[i]);
        }
        m_files.clear();
    }

    SourceFile *SourceManager::loadFile(TkStringView path)
    {
        TkString normalized(path);
        TkPath::normalize(normalized);

        // De-dup by absolute path.
        for (i32 i = 1; i < m_files.count(); i++)
        {
            if (m_files[i] != nullptr && !m_files[i]->isVirtual() && m_files[i]->getPath() == normalized)
                return m_files[i];
        }

        OpenFileParams params{
            .path = normalized,
            .mode = FileMode::kOpenExisting,
            .access = FileAccess::kRead,
            .share = FileShare::kRead,
        };
        FileHandle handle = TkFileSystem::openFile(params);
        if (!TkFileSystem::isFileOpened(handle))
            return nullptr;

        TkString content;
        const bool ok = TkFileSystem::readAsText(handle, content);
        TkFileSystem::closeFile(handle);
        if (!ok)
            return nullptr;

        if (m_files.isEmpty())
            m_files.add(nullptr); // reserve id 0 as invalid

        const u32 id = static_cast<u32>(m_files.count());
        SourceFile *file = TkMemory::newObject<SourceFile>(TkMemory::getDefault(), id, TkStringView(normalized), TkStringView(content), false);
        m_files.add(file);
        return file;
    }

    SourceFile *SourceManager::addVirtualFile(TkStringView name, TkStringView content)
    {
        if (m_files.isEmpty())
            m_files.add(nullptr);

        const u32 id = static_cast<u32>(m_files.count());
        SourceFile *file = TkMemory::newObject<SourceFile>(TkMemory::getDefault(), id, name, content, true);
        m_files.add(file);
        return file;
    }

    SourceFile *SourceManager::getFile(u32 fileId)
    {
        if (fileId == 0 || static_cast<i32>(fileId) >= m_files.count())
            return nullptr;
        return m_files[static_cast<i32>(fileId)];
    }

    const SourceFile *SourceManager::getFile(u32 fileId) const
    {
        if (fileId == 0 || static_cast<i32>(fileId) >= m_files.count())
            return nullptr;
        return m_files[static_cast<i32>(fileId)];
    }

    SourceLocation SourceManager::getLocation(u32 fileId, u32 offset) const
    {
        const SourceFile *file = getFile(fileId);
        if (file == nullptr)
            return SourceLocation::invalid();
        return file->getLocation(offset);
    }

    TkStringView SourceManager::getFilePath(u32 fileId) const
    {
        const SourceFile *file = getFile(fileId);
        if (file == nullptr)
            return TkStringView();
        return TkStringView(file->getPath());
    }
}
