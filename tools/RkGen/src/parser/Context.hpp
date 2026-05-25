/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
#pragma once

#include <toolkit/common.hpp>

#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit {

    struct ParserContextData
    {
        TkString rootPath;
        TkString outputDir;
        TkArray<TkString> sourceFiles;
    };

    class ParserContext 
    {
        friend class CXXSourceFile;
        friend class CXXParser;
        friend class Generator;

    public:
        ParserContext(ParserContextData dir);
        ~ParserContext();

        bool setUp();
        bool processAll();

        const ParserContextData& getData() {
            return m_data;
        }

        class MarcoPicker* parseMarcoOnlyFile(const TkStringView&);
        TkString parseRelivite(const TkStringView& v) const;
        bool hasMarco(const TkStringView& v);

    private:
        TkHashMap<TkString, class CXXSourceFile*> m_files;
        TkHashMap<TkString, MarcoPicker*> m_bufferedMarcos;
        ParserContextData m_data;
        class MemoryPurcBuffer* m_buffer;
    };

}
