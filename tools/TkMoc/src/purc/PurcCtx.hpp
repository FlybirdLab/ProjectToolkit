/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
#pragma once

#include <toolkit/common.hpp>

#include <toolkit/types/String.hpp>
#include <toolkit/collection/HashMap.hpp>

namespace toolkit {

    struct PurcCtxData
    {
        TkString rootPath;
        TkString outputDir;
    };

    class PurcCtx 
    {
        friend class CXXSourceFile;
        friend class CXXParser;

    public:
        PurcCtx(PurcCtxData dir);
        ~PurcCtx();

        bool setUp();
        bool processAll();

        const PurcCtxData& getData() {
            return m_data;
        }

        class MarcoPicker* parseMarcoOnlyFile(const TkStringView&);
        TkString parseRelivite(const TkStringView& v) const;
        bool hasMarco(const TkStringView& v);

    private:
        TkHashMap<TkString, class CXXSourceFile*> m_files;
        TkHashMap<TkString, MarcoPicker*> m_bufferedMarcos;
        PurcCtxData m_data;
        class MemoryPurcBuffer* m_buffer;
    };

}