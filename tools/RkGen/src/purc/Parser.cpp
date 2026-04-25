#include "Parser.hpp"

#include "toolkit/collection/StaticArray.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/types/StringBuilder.hpp"
#include "purc/ASTNodes.hpp"
#include "purc/Lexer.hpp"
#include "purc/MarcoPicker.hpp"
#include "purc/PurcCtx.hpp"
#include "purc/Source.hpp"
#include "purc/Tokens.hpp"

namespace toolkit {

    template <typename T>
    struct ScopeGuard
    {
        ScopeGuard(T* ptr)
            : m_used{ false }
            , m_ptr{ ptr }
        {
        }
        ~ScopeGuard() {
            if (m_used) return;
            TkMemory::deletePtr(m_ptr);
        }

        T* getPtr() {
            m_used = true;
            return m_ptr;
        }

        T* operator->() {
            return m_ptr;
        }

    private:
        bool m_used;
        T* m_ptr;
    };


#define ERRR() &m_diag, current(), m_src
#define CLEARUP(V) \
do { \
    for (auto* ptr : V) { \
        delete ptr; \
    } \
    V.clear();\
} while(false)

#define CHECK_RESULT(NAME, EXPR) \
    const auto NAME = EXPR; \
    if (!NAME) { \
         return NAME.error();\
    }

    ///////////////////////////////////////////////////////////////////
    /// Parser util functions for tokens
    ///////////////////////////////////////////////////////////////////
    Token& CXXParser::current() const {
        return m_current;
    }


    Token& CXXParser::peek() const {
        if (m_tk_idx + 1 >= m_lex->m_tokens.count()) {
            return Token::Empty;
        }
        return m_lex->m_tokens[m_tk_idx + 1];
    }


    Token& CXXParser::peekPrevious() const {
        if (m_tk_idx - 1 < 0) {
            return Token::Empty;
        }
        return m_lex->m_tokens[m_tk_idx - 1];
    }


    Token& CXXParser::advance() {
        if (m_tk_idx + 1 >= m_lex->m_tokens.count()) {
            return Token::Empty;
        }
        m_tk_idx += 1;
        return m_current = m_lex->m_tokens[m_tk_idx];
    }


    Token& CXXParser::previous() {
        if (m_tk_idx - 1 < 0) {
            return Token::Empty;
        }
        m_tk_idx -= 1;
        return m_current = m_lex->m_tokens[m_tk_idx];
    }


    bool CXXParser::match(const TokenType t) const {
        return m_current.type == t;
    }


    bool CXXParser::expect(const TokenType t) {
        return peek().type == t;
    }
    ///////////////////////////////////////////////////////////////////

    CXXParser::CXXParser(CXXLexer* lex, CXXSourceFile* f)
        : m_root{ nullptr }
        , m_current{ Token::Empty }
    {
        TK_ASSERT(lex);
        m_lex = lex;
        m_src = f;
    }


    CXXParser::~CXXParser() {
        if (m_root) {
            TkMemory::deletePtr(m_root);
        }
    }


    bool CXXParser::parse()
    {
        if (m_root) {
            return true;
        }
        m_root = TkMemory::newObject<ASTTransUnit>();
        m_current = m_lex->m_tokens[0];

        // parse entry
        if (const auto r = parseRoot(); !r) {
            m_diag.printAll();
            return false;
        }

        auto* ptr = TkMemory::newObject<MarcoPicker>();
        ptr->m_marcos.swap(m_marcos);

        m_src->m_ctx->m_bufferedMarcos.add(m_src->m_ctx->parseRelivite(m_src->m_path), ptr);

        // dump ast logic

        return true;

    }


    Expected<void> CXXParser::parseRoot()
    {
        while (!expect(TokenType::kEOF)) {
            if (match(TokenType::kPreprocessor)) {
                if (expect(TokenType::kIdentifier) && peek().value == "include") {
                    advance();
                    auto r = parseInclude();
                    if (!r) {
                        return r.error();
                    }
                    const auto& vl = r.value();
                    m_src->m_includes.add(vl);
                    m_src->m_ctx->parseMarcoOnlyFile(vl);
                }
                else if (expect(TokenType::kIdentifier) && peek().value == "define") {
                    CHECK_RESULT(rs, jumpUnneed());
                }
                else if (expect(TokenType::kIdentifier) && peek().value == "undef") {
                    CHECK_RESULT(rs, jumpUnneed());
                }
                else {
                    do {
                        advance();
                    } while (!match(TokenType::kEOL));
                    advance();
                }
            }
            else if (match(TokenType::kIdentifier))
            {
                if (current().value == "namespace")
                {
                    CHECK_RESULT(rs, parseNamespace());
                    m_root->decls.add(rs.value());
                }
            }
            else if (match(TokenType::kEOL)) {
                advance();
            }
            else {
                return Error::failure("Unexpected token at top level.", ERRR());
            }
            // else {
            //     return Error::failure("", ERRR());
            // }
        }

        return Error::success();
    }


    Expected<TkString> CXXParser::parseInclude() {
        if (!match(TokenType::kIdentifier) || current().value != "include") {
            return Error::failure("Invalid include expression", ERRR());
        }
        advance();
        if (!match(TokenType::kString)) {
            advance();
            return Error::failure(TkString::format("Invalid include expression, include need file path but got %s", peek().getTypeName()), ERRR());
        }

        TkString val = current().value;
        advance();
        return val;
    }


    Expected<ASTNamespace*> CXXParser::parseNamespace() {
        if (!expect(TokenType::kIdentifier)) {
            return Error::failure(TkString::format("expect namespace name, but got {0}", peek().getTypeName()), ERRR());
        }
        TkStringBuilder nm{};
        advance();
        do {
            if (match(TokenType::kIdentifier)) {
                nm.append(current().value);
                advance();
                if (match(TokenType::kDoubleColon)) {
                    nm.append("::");
                    advance();
                }
            }
            else {
                return Error::failure(TkString::format("expect namespace name, but got {0}", peek().getTypeName()), ERRR());
            }
        } while (match(TokenType::kIdentifier));

        if (!match(TokenType::kLCurlyBrace)) {
            return Error::failure("Expect namespace body but not found", ERRR());
        }
        advance();

        ScopeGuard<ASTNamespace> np = TkMemory::newObject<ASTNamespace>(TkMemory::getDefault(), nm.toString());
        // np->decls

        do {
            if (match(TokenType::kIdentifier))
            {
                if (current().value == "namespace")
                {
                    auto r = parseNamespace();
                    if (!r) {
                        return r.error();
                    }
                    np->decls.add(r.value());
                    advance();
                }
                if (current().value == "cclass")
                {
                    auto r = parseCClass();
                    if (!r) {
                        return r.error();
                    }
                    np->decls.add(r.value());
                    advance();
                }
                else if (current().value == "cstruct")
                {

                }
                else if (current().value == "cenum")
                {

                }
                else if (current().value == "cinterface")
                {

                }
                else {
                    auto r = jumpUnneed();
                    if (!r) return r.error();
                }
            }
            else if (match(TokenType::kEOL)) {
                advance();
            }
            else if (match(TokenType::kEOF)) {
                return Error::failure("Expect -> } but got nothing", ERRR());
            }
            else {
                auto r = jumpUnneed();
                if (!r) return r.error();
            }
        } while (!match(TokenType::kRCurlyBrace));

        return np.getPtr();
    }


    Expected<TkString> CXXParser::parseDeclName()
    {
        TkStaticArray<TkString, 10> tags{};

        do {
            // advance();
            if (match(TokenType::kIdentifier)) {
                if (current().value == "final") {
                    advance();
                }
                else if (current().value == "sealed") {
                    advance();
                }
                else if (current().value == "alignas") {
                    do {
                        advance();
                        if (match(TokenType::kRParen)) {
                            advance();
                            break;
                        }
                    } while (!match(TokenType::kEOF));
                }
                else if (current().value == "__declspec" && expect(TokenType::kLParen)) {
                    do {
                        advance();
                        if (match(TokenType::kRParen)) {
                            advance();
                            break;
                        }
                    } while (!match(TokenType::kEOF));
                }
                else if (current().value == "__attribute__" && expect(TokenType::kLParen)) {
                    advance();
                    if (!expect(TokenType::kLParen)) {
                        return Error::failure(TkString::format("expect ( after ( for attribute statement but got {0}", peek().toString()), ERRR());
                    }
                    advance(); // skip the second '('

                    do {
                        advance();
                        if (match(TokenType::kRParen)) {
                            if (!expect(TokenType::kRParen)) {
                                return Error::failure(TkString::format("expect ) after ) for attribute statement but got {0}", peek().toString()), ERRR());
                            }
                            advance();
                            advance();
                            break;
                        }
                    } while (!match(TokenType::kEOF));
                }
                else {
                    if (!isMarco(current().value)) {
                        tags.add(current().value);
                    }
                    advance();
                }
            }
            else if (match(TokenType::kLBracket) && expect(TokenType::kLBracket)) {
                do {
                    advance();
                    if (match(TokenType::kRBracket)) {
                        if (!expect(TokenType::kRBracket)) {
                            return Error::failure(TkString::format("expect ] after ] for attribute statement but got {0}", peek().toString()), ERRR());
                        }
                        advance();
                        advance();
                        break;
                    }
                } while (!match(TokenType::kEOF));
            }
            else if (match(TokenType::kColon)) {
                break;
            }
            else {
                return Error::failure(TkString::format("unexpected token find in class declearation {0}", current().toString()), ERRR());
            }
        } while (!match(TokenType::kLCurlyBrace));

#if TK_DEBUG
        for (auto& item : tags) {
            LogDebug("[TAG {0}]", item);
        }
#endif
        if (tags.count() != 1) {
            return Error::failure("Unknown token in name definations");
        }

        return tags[0];
    }


    Expected<ASTTypeDecl*> CXXParser::parseCClass()
    {
        if (!match(TokenType::kIdentifier) || current().value != "cclass") {
            return Error::failure(TkString::format("expected cclass for objective class but got {0}", current().value), ERRR());
        }
        advance(); // skip cclass token

        auto r = parseDeclName();
        if (!r) {
            return r.error();
        }
        auto name = r.value();
        ScopeGuard g = TkMemory::newObject<ASTTypeDecl>(TkMemory::getDefault(), name, ASTTypeDecl::DeclType::kClass);
        TkStringBuilder baseClassNameBuilder{};

        if (match(TokenType::kColon)) {
            // parse base classes
            advance();
            do {
                if (match(TokenType::kIdentifier)) {
                    if (current().value == "public" || current().value == "protected" || current().value == "private") {
                        advance();
                    }
                    else {
                        if (expect(TokenType::kDoubleColon)) {
                            baseClassNameBuilder.clear();
                            do {
                                if (match(TokenType::kIdentifier)) {
                                    baseClassNameBuilder.append(current().value);
                                    advance();
                                }
                                else if (match(TokenType::kDoubleColon)) {
                                    baseClassNameBuilder.append("::");
                                    advance();
                                }
                                else if (match(TokenType::kComma) || match(TokenType::kLCurlyBrace)) {
                                    break;
                                }
                                else if (match(TokenType::kLAngleBracket)) {
                                    do {
                                        advance();
                                    } while (!match(TokenType::kRAngleBracket));
                                    advance();
                                }
                                else {
                                    advance();
                                }
                            } while (!match(TokenType::kLCurlyBrace));
                            auto rd = baseClassNameBuilder.toString();
                            g->baseClasses.add(rd);
                            continue;
                        }
                        g->baseClasses.add(current().value);
                        advance();
                        if (match(TokenType::kLAngleBracket)) {
                            do {
                                advance();
                            } while (!match(TokenType::kRAngleBracket));
                            advance();
                        }
                    }
                }
                else if (match(TokenType::kComma)) {
                    advance();
                }
                else if (match(TokenType::kDoubleColon)) {
                    advance();
                }
                else if (match(TokenType::kEOL)) {
                    advance();
                }
                else {
                    return Error::failure(TkString::format("unexpected token in base classes {0}", current().toString()), ERRR());
                }
            } while (!match(TokenType::kLCurlyBrace));
            goto parseBody;
        }
        else if (match(TokenType::kLCurlyBrace)) {
            // parse body
        parseBody:
            bool isStatic = false;
            advance(); // skip {

            do {
                if (match(TokenType::kIdentifier)) {
                    // parse ctor
                    if (current().value == name) {
                        CHECK_RESULT(result, parseCtor(name));
                        g->members.add(result.value());
                    }
                    else if (current().value == "cfunc")
                    {
                        CHECK_RESULT(result, parseCFunction(isStatic));
                        g->members.add(result.value());
                    }
                    else if (current().value == "cfield")
                    {
                        CHECK_RESULT(result, parseCField(isStatic));
                        g->members.add(result.value());
                    }
                    else if (current().value == "cenum")
                    {
                        CHECK_RESULT(result, parseCEnum());
                        g->members.add(result.value());
                    }
                    else if (current().value == "cstruct")
                    {
                        CHECK_RESULT(result, parseCStruct());
                        g->members.add(result.value());
                    }
                    else if (current().value == "cclass")
                    {
                        CHECK_RESULT(result, parseCClass());
                        g->members.add(result.value());
                    }
                    else if (current().value == "static") {
                        isStatic = true;
                        advance();
                    }
                    else if (current().value == "C_OBJECT") {
                        advance();
                        // skip optional ';'
                        if (match(TokenType::kSemiColon)) {
                            advance();
                        }
                    }
                    else if (
                        current().value == "public" ||
                        current().value == "private" ||
                        current().value == "protected")
                    {
                        advance();
                        // skip optional ';'
                        if (match(TokenType::kColon)) {
                            advance();
                        }
                    }
                    else {
                    jumpId:
                        // skip unneed class member
                        CHECK_RESULT(result, jumpUnneed());
                    }

                    // reset isStatic state
                    isStatic = false;

                    // reduce cycle times for ';' after function/field declearation
                    if (match(TokenType::kSemiColon)) {
                        advance();
                    }
                    if (match(TokenType::kEOL)) {
                        advance();
                    }
                }
                else if (match(TokenType::kSemiColon)) {
                    advance();
                }
                else if (match(TokenType::kEOL)) {
                    advance();
                }
                else {
                    // jump unneed line
                    jumpLine();
                }
            } while (!match(TokenType::kRCurlyBrace));
            return g.getPtr();
        }
        else {
            return Error::failure("Invalid class declearation after class name declear", ERRR());
        }
    }


    Expected<struct ASTTypeDecl*> CXXParser::parseCStruct() {
        // cstruct is a c-styled struct with all public member defaultly marked cfield(,)
        if (!match(TokenType::kIdentifier) || current().value != "cstruct") {
            return Error::failure("Error scope for cstruct parsing", ERRR());
        }
        advance(); // skip cstruct token

        // parse struct name
        CHECK_RESULT(r, parseDeclName());
        auto name = r.value();

        if (!expect(TokenType::kLCurlyBrace)) {
            return Error::failure(TkString::format("cstruct not allow attching base classes on it ! got {0}", peek().toString()), ERRR());
        }
        advance(); // skip '{'
        TkStringBuilder tpBuild {};

        bool isStatic = false;
        do {
            if (match(TokenType::kIdentifier)) {
                if (current().value == "__declspec" && expect(TokenType::kLParen)) {
                    do {
                        advance();
                        if (match(TokenType::kRParen)) {
                            advance();
                            break;
                        }
                    } while (!match(TokenType::kEOF));
                }
                else if (current().value == "constexpr") {
                    jumpLine();
#if TK_DEBUG
                    LogWarn("[found constexpr, skip current]");
#endif
                }
                else if (current().value == "inline") {
                    jumpLine();
#if TK_DEBUG
                    LogWarn("[found inline, skip current]");
#endif
                }
                else if (current().value == "extern") {
                    jumpLine();
#if TK_DEBUG
                    LogWarn("[found extern, skip current]");
#endif
                }
                else if (current().value == "static") {
                    isStatic = true;
                    advance();
                }
                else if (current().value == "volatile") {
                    advance();
                }
                else if (current().value == "mutable") {
                    advance();
                }
                else if (current().value == "thread_local") {
                    advance();
                }
                else if (current().value == "restrict") {
                    advance();
                }
                else if (current().value == "alignas") {
                    do {
                        advance();
                    } while (!match(TokenType::kRParen));
                    advance(); // skip ')'
                }
                else if (current().value == "__attribute__" && expect(TokenType::kLParen)) {
                    advance();
                    if (!expect(TokenType::kLParen)) {
                        return Error::failure(TkString::format("expect ( after ( for attribute statement but got {0}", peek().toString()), ERRR());
                    }
                    advance(); // skip the second '('

                    do {
                        advance();
                        if (match(TokenType::kRParen)) {
                            if (!expect(TokenType::kRParen)) {
                                return Error::failure(TkString::format("expect ) after ) for attribute statement but got {0}", peek().toString()), ERRR());
                            }
                            advance();
                            advance();
                            break;
                        }
                    } while (!match(TokenType::kEOF));
                }
                else {
                    if (isMarco(current().value)) {
                        advance();
                    }
                    if (tpBuild.length() == 0) {
                        goto tpAppend;
                    }
                    tpBuild.append(' ');
                    tpAppend:
                    tpBuild.append(current().value);
                }
            }
            // for fallbacks
            else if (match(TokenType::kEOL)) {
                advance();
            }
            else if (match(TokenType::kEqual)) {
                do {
                    
                } while(!match(TokenType::kSemiColon));
            }
            else if (match(TokenType::kSemiColon)) {
                advance();
            }
        } while (!match(TokenType::kRCurlyBrace));
    }


    Expected<ASTEnum*> CXXParser::parseCEnum() {
        // enable text2value mapping for enum classes

        return Error::failure("TODO");
    }


    Expected<ASTFunction*> CXXParser::parseCFunction(bool isStatic)
    {
        if (!match(TokenType::kIdentifier) || current().value != "cfunc") {
            return Error::failure("Invalid scope for parsing cfunc");
        }
        advance(); // skip cfunc

        TkString type{};
        TkString name{};
        CHECK_RESULT(r1, parseFuncTypeAndName(&type, &name, isStatic));

        ScopeGuard g = TkMemory::newObject<ASTFunction>();
        g->name = name;
        g->returnType = type;
        g->isStatic = isStatic;

        TkHashMap<TkString, TkString> args;
        CHECK_RESULT(r2, parseFuncArgs(&args));
        args.swap(g->params);

        return g.getPtr();
    }


    Expected<ASTFunction*> CXXParser::parseCtor(const TkString& name)
    {
        if (!match(TokenType::kIdentifier)) {
            return Error::failure("unexpected token for constructor. expect identifier for function declearation", ERRR());
        }
        if (!expect(TokenType::kLParen)) {
            return Error::failure(TkString::format("expected ( for ctor defination but got {0}", peek().toString()), ERRR());
        }
        advance(); //skip ctor name

        TkHashMap<TkString, TkString> args{};

        ScopeGuard g = TkMemory::newObject<ASTFunction>();
        g->name = name;
        g->isConstructor = true;
        g->returnType = TkString::Empty;

        CHECK_RESULT(result, parseFuncArgs(&args))
            args.swap(g->params);

        return g.getPtr();
    }


    Expected<void> CXXParser::parseFuncTypeAndName(TkString* s, TkString* name, bool& isStatic)
    {
        if (s == nullptr || name == nullptr) {
            return Error::failure("Internal error : [parseType(TkString* s, name <- 'is nullptr']", ERRR());
        }
        s->clear();
        name->clear();

        do {
            if (match(TokenType::kIdentifier)) {
                if (current().value == "template") {
                    return Error::failure("cfunc unsupport template on it!");
                }
                else if (current().value == "static")
                {
                    advance();
                    isStatic = true;
                }
                else if (
                    current().value == "inline" ||
                    current().value == "constexpr" ||
                    current().value == "decltype")
                {
                    advance();
                }
                else {
                    if (isMarco(current().value)) {
                        advance();
                        continue;
                    }
                    if (s->isEmpty()) {
                        goto appendOnly;
                    }
                    s->append(' ');
                appendOnly:
                    s->append(current().value);
                    advance();
                }
            }
            else if (match(TokenType::kDoubleColon)) {
                advance();
            }
            else if (match(TokenType::kEOL)) {
                advance();
            }
            else if (match(TokenType::kLBracket) && expect(TokenType::kLBracket)) {
                do {
                    advance();
                    if (match(TokenType::kRBracket)) {
                        if (!expect(TokenType::kRBracket)) {
                            return Error::failure(TkString::format("expect ] after ] for attribute statement but got {0}", peek().toString()), ERRR());
                        }
                        advance();
                        advance();
                        break;
                    }
                } while (!match(TokenType::kEOF));
            }
            else {
                return Error::failure(TkString::format("Unexpected token in function declearation {0}", current().toString()), ERRR());
            }
        } while (!match(TokenType::kLParen));

        TkArray<TkString> out;
        s->split(' ', out);

        if (out.count() != 2) {
            return Error::failure("Invalid function declearation, need [type] [name]", ERRR());
        }

        *s = out[0];
        *name = out[1];

        LogDebug("[Parsed Type {0}]", *s);
        LogDebug("[Parsed Name {0}]", *name);

        return Error::success();
    }


    Expected<void> CXXParser::parseFuncArgs(TkHashMap<TkString, TkString>* out)
    {
        out->clear();

        if (!match(TokenType::kLParen)) {
            return Error::failure("Invalid scope for parsing function arguments", ERRR());
        }
        TkStringBuilder bd{};
        TkString name{};

        do {
            if (match(TokenType::kRParen)) {
                break;
            }
            else if (match(TokenType::kIdentifier))
            {
                bd.clear();
                name.clear();

                /// ....
                do {
                    if (match(TokenType::kIdentifier)) {
                        auto& v = current().value;
                        if (isMarco(v)) {
                            advance();
                            continue;
                        }
                        if (expect(TokenType::kEqual) || expect(TokenType::kComma) || expect(TokenType::kRParen)) {
                            name = current().value;
                        }
                        else {
                            if (bd.length() == 0) {
                                goto appendOnlyBd;
                            }
                            bd.append(' ');
                        appendOnlyBd:
                            bd.append(v);
                        }
                        advance();
                    }
                    else if (match(TokenType::kComma)) {
                        break;
                    }
                    else if (match(TokenType::kEqual)) {
                        do {
                            advance();
                            if (match(TokenType::kComma)) {
                                break;
                            }
                        } while (!match(TokenType::kRParen));
                        break;
                    }
                    else if (match(TokenType::kRParen)) {
                        break;
                    }
                    else {
                        advance();
                    }
                } while (!match(TokenType::kRParen));

                auto r = bd.toString();
                out->add(r, name);
                LogDebug("[Parsed arg TYPE: {0} NAME:{1}]", r, name);
            }
            else if (match(TokenType::kComma)) {
                advance();
            }
            else {
                advance();
            }
        } while (!match(TokenType::kRParen));

        advance(); // skip ')'
        return Error::success();
    }


    Expected<ASTField*> CXXParser::parseCField(bool isStatic) {
        return Error::failure("TODO");
    }


    Expected<void> CXXParser::jumpUnneed() {
        if (match(TokenType::kPreprocessor)) {
            advance();
            if (current().value == "pragma") {
                do {
                    advance();
                } while (!match(TokenType::kEOL));
                advance();
                return Error::success();
            }
            if (current().value == "undef") {
                if (!expect(TokenType::kIdentifier)) {
                    return Error::failure("Invalid #undef expression", ERRR());
                }
                advance();
                m_marcos.remove(current().value);
                advance();
            }
            else if (current().value == "define") {
                advance(); // skip define id

                if (!match(TokenType::kIdentifier)) {
                    return Error::failure(
                        "expected macro name after #define during jumpUnneed",
                        ERRR());
                }
                m_marcos.add(current().value);
                advance(); // skip marco name

                if (match(TokenType::kLParen)) {
                    int depth = 1;
                    advance(); // skip '('
                    while (depth > 0) {
                        if (match(TokenType::kLParen)) {
                            ++depth;
                        }
                        else if (match(TokenType::kRParen)) {
                            --depth;
                        }
                        else if (match(TokenType::kEOF)) {
                            return Error::failure(
                                "unterminated macro parameter list during jumpUnneed",
                                ERRR());
                        }
                        advance();
                    }
                }

                // skip marco body
                while (true) {
                    if (match(TokenType::kSlash) && expect(TokenType::kEOL)) {
                        advance();
                        advance();
                        continue;
                    }
                    if (match(TokenType::kEOL)) {
                        break;
                    }
                    if (match(TokenType::kEOF)) {
                        break;
                    }
                    advance();
                }

                return Error::success();
            }
        }

        enum ScopeType : u8 { kCurly, kParen };
        TkStaticArray<ScopeType, 8> scopes{};

        do {
            if (match(TokenType::kLCurlyBrace)) {
                scopes.enqueue(kCurly);
                advance();
            }
            else if (match(TokenType::kRCurlyBrace)) {
                if (scopes.isEmpty() || scopes.peek() != kCurly) {
                    return Error::failure("unexpected } during jumpUnneed",
                        ERRR());
                }
                scopes.pop();
                advance();
                if (match(TokenType::kSemiColon)) {
                    advance();
                }
                if (scopes.isEmpty()) {
                    return Error::success();
                } // ✅
            }
            else if (match(TokenType::kLParen)) {
                scopes.enqueue(kParen);
                advance();
            }
            else if (match(TokenType::kRParen)) {
                if (scopes.isEmpty() || scopes.peek() != kParen) {
                    return Error::failure("unexpected ) during jumpUnneed",
                        ERRR());
                }
                scopes.pop();
                advance();
            }
            else if (match(TokenType::kSemiColon)) {
                if (scopes.isEmpty()) {
                    advance();
                    return Error::success(); // ✅
                }
                advance();
            }
            else if (match(TokenType::kEOF)) {
                return Error::failure("unexpected EOF during jumpUnneed",
                    ERRR());
            }
            else {
                advance(); // skip token
            }
        } while (true);
    }


    void CXXParser::jumpLine() {
        do {
            advance();
        } while (!match(TokenType::kEOL) && !match(TokenType::kEOF));
    }


    bool CXXParser::isMarco(const TkStringView& c) const
    {
        if (m_marcos.find(c) != -1) {
            return true;
        }

        return m_src->m_ctx->hasMarco(c);
    }

} // namespace cobalt