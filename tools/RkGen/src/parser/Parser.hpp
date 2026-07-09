/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
#pragma once

#include "parser/Tokens.hpp"
#include "toolkit/types/StringView.hpp"
#include "parser/diagnose/Diagnostic.hpp"

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
        struct ASTTransUnit* takeRoot();

    private:
        Token& current() const;
        Token& peek(i32 offset = 1) const;
        Token& peekPrevious() const;
        Token& advance();
        Token& previous();
        bool match(TokenType) const;
        bool expect(TokenType);
        bool isIdentifier(const char* text = nullptr) const;
        bool consume(TokenType);
        bool consumeIdentifier(const char* text = nullptr);

    private:
        Expected<void> parseRoot();

        Expected<TkString> parseInclude();
        Expected<struct ASTNamespace*> parseNamespace();

        Expected<TkString> parseDeclName();
        void parseBaseList(TkArray<TkString>* out);

        Expected<struct ASTTypeDecl*> parseCClass();
        Expected<struct ASTTypeDecl*> parseCStruct();
        Expected<struct ASTEnum*> parseCEnum();

        Expected<struct ASTFunction*> parseCFunction(bool isStatic = false);
        Expected<struct ASTFunction*> parseCtor(const TkString& name, bool isReflectConstructor = false);
        Expected<void> parseFuncTypeAndName(TkString* s, TkString* name, bool& isStatic);
        Expected<void> parseFuncArgs(TkArray<struct ASTParam>* out);

        Expected<struct ASTField*> parseCField(bool isStatic = false);
        Expected<struct ASTField*> parseImplicitStructField(bool isStatic = false);
        Expected<void> parsePendingAttributes();
        void applyPendingAttributes(struct ASTTypeDecl* type);
        void applyPendingAttributes(struct ASTField* field);
        void applyPendingAttributes(struct ASTFunction* function);

        Expected<void> jumpUnneed();
        void jumpLine();
        void skipConditionalBlock(bool stopAtElse);
        bool evalPreprocessorCondition();

        bool isMarco(const TkStringView& c) const;

    private:
        CXXSourceFile* m_src;
        CXXLexer* m_lex;
        TkArray<TkString> m_marcos;
        ASTTransUnit* m_root;
        DiagnosticCollector m_diag;
        mutable Token m_current;
        TkArray<TkString> m_pendingAttributes;

        i32 m_tk_idx = 0;
        bool m_skipNextClass = false;  // set when template<> precedes rkclass/rkstruct
    };
}
