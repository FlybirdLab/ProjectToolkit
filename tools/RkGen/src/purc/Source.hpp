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
        friend class PurcCtx;
        
    public:
        CXXSourceFile(const TkStringView& path, class PurcCtx*);
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

    private:
        ASTNode* m_rootNode;
        class PurcCtx* m_ctx;
        class CXXLexer* m_lexer;
        class CXXParser* m_parser;
        
        TkString m_src;
        TkArray<TkString> m_includes;
        const TkStringView m_path;
        bool m_isProcessed;
        
    };

}