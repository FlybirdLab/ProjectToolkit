/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
#include "toolkit/types/StringView.hpp"
#pragma cobjective
#pragma once

#include "purc/Tokens.hpp"
#include "purc/diagnose/Diagnostic.hpp"

namespace toolkit {

    struct ASTTransUnit;

    class CXXParser
    {
    public:
        CXXParser(class CXXLexer*, class CXXSourceFile*);
        ~CXXParser();

        bool parse();

        TK_FORCE_INLINE struct ASTTransUnit* getRoot() {
            return m_root;
        }

    private:
        Token& current() const;
        Token& peek() const;
        Token& peekPrevious() const;
        Token& advance();
        Token& previous();
        bool match(TokenType) const;
        bool expect(TokenType);

    private:
        Expected<void> parseRoot();

        Expected<TkString> parseInclude();
        Expected<struct ASTNamespace*> parseNamespace();

        Expected<TkString> parseDeclName();

        Expected<struct ASTTypeDecl*> parseCClass();
        Expected<struct ASTTypeDecl*> parseCStruct();
        Expected<struct ASTEnum*> parseCEnum();

        Expected<struct ASTFunction*> parseCFunction(bool isStatic = false);
        Expected<struct ASTFunction*> parseCtor(const TkString& name);
        Expected<void> parseFuncTypeAndName(TkString* s, TkString* name, bool& isStatic);
        Expected<void> parseFuncArgs(TkHashMap<TkString, TkString>* out); 

        Expected<struct ASTField*> parseCField(bool isStatic = false);

        Expected<void> jumpUnneed();
        void jumpLine();

        bool isMarco(const TkStringView& c) const;

    private:
        CXXSourceFile* m_src;
        CXXLexer* m_lex;
        TkArray<TkString> m_marcos;
        ASTTransUnit* m_root;
        DiagnosticCollector m_diag;
        Token& m_current;

        i32 m_tk_idx = 0;
    };
}