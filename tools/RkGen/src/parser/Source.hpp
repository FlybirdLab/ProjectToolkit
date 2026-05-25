/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
#pragma once

#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit {

    struct ASTNode;

    // Wrapper for CXX languages header file, including processing tokenize and parse
    class CXXSourceFile 
    {
        friend class CXXParser;
        friend class ParserContext;
        
    public:
        CXXSourceFile(const TkStringView& path, class ParserContext*);
        ~CXXSourceFile();

        bool tryRead();
        bool processFile();

        TK_FORCE_INLINE ASTNode* getASTRoot() {
            if (!m_isProcessed) 
                return nullptr;
            return m_rootNode;
        }
        TK_FORCE_INLINE TkString& getSource() {
            return m_src;
        }
        TK_FORCE_INLINE const TkString& getPath() const {
            return m_path;
        }

    private:
        ASTNode* m_rootNode;
        class ParserContext* m_ctx;
        class CXXLexer* m_lexer;
        class CXXParser* m_parser;
        
        TkString m_src;
        TkArray<TkString> m_includes;
        TkString m_path;
        bool m_isProcessed;
        
    };

}
