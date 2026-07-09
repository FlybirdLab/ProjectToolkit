/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "PurcBuffer.hpp"
#include "parser/Context.hpp"

namespace toolkit
{

    PurcBuffer::FileInfo PurcBuffer::Invalid{0, 0};
    PurcBuffer::PurcBuffer(const TkStringView &path, ParserContext *ctx)
    {
        m_ctx = ctx;
    }
#if 1
    MemoryPurcBuffer::MemoryPurcBuffer(const TkStringView &bufPath, ParserContext *ctx)
        : PurcBuffer(TkStringView::Empty, ctx)
    {
        (void)bufPath;
    }
    MemoryPurcBuffer::~MemoryPurcBuffer()
    {
        m_buf.clear();
        m_datas.clear();
    }

    MemoryPurcBuffer::FileInfo &MemoryPurcBuffer::getFileInfo(const TkStringView &name)
    {
        if (!m_datas.find(name))
        {
            return Invalid;
        }
        return m_datas[name];
    }

    void *MemoryPurcBuffer::getFileData(const TkStringView &name)
    {
        if (!m_buf.find(name))
        {
            return nullptr;
        }
        return m_buf[name];
    }

    bool MemoryPurcBuffer::tryAddToBuf(const TkStringView &name, FileInfo &info, void *data)
    {
        removeItem(name);
        if (m_datas.find(name))
        {
            return false;
        }
        if (data == nullptr)
        {
            return false;
        }
        m_datas.add(name, info);
        m_buf.add(name, data);
        return (bool)m_datas.find(name);
    }

    void MemoryPurcBuffer::removeItem(const TkStringView &name)
    {
        if (m_buf.find(name))
        {
            m_buf.remove(name);
        }
        if (m_datas.find(name))
        {
            m_datas.remove(name);
        }
    }

    // MemoryPurcBuffer don't need saving -> just for debuging (deploy env should use LDBPurcBuffer instead)
    void MemoryPurcBuffer::flush() {}
    void MemoryPurcBuffer::save() {}
    void MemoryPurcBuffer::close()
    {
        clear();
    }
    void MemoryPurcBuffer::clear()
    {
        m_datas.clear();
        m_buf.clear();
    }
#endif
} // namespace cobalt
