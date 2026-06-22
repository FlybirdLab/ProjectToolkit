#include "Parser.hpp"

#include "toolkit/memory/Memory.hpp"
#include "toolkit/types/StringBuilder.hpp"
#include "parser/ASTNodes.hpp"
#include "parser/Lexer.hpp"
#include "parser/MarcoPicker.hpp"
#include "parser/Context.hpp"
#include "parser/Source.hpp"

#include <cctype>

namespace toolkit {

    template <typename T>
    struct ScopeGuard
    {
        ScopeGuard(T* ptr) : m_used(false), m_ptr(ptr) {}
        ~ScopeGuard() { if (!m_used) TkMemory::deletePtr(m_ptr); }
        T* getPtr() { m_used = true; return m_ptr; }
        T* operator->() { return m_ptr; }
    private:
        bool m_used;
        T* m_ptr;
    };

#define ERRR() &m_diag, current(), m_src
#define CHECK_RESULT(NAME, EXPR) \
    const auto NAME = EXPR; \
    if (!NAME) { return NAME.error(); }

    static bool isBuiltinSkippedMember(const TkString& text)
    {
        return text == "using" || text == "typedef" || text == "friend" || text == "template" ||
               text == "constexpr" || text == "inline" || text == "extern";
    }

    static bool isConstructorSpecifier(const TkString& text)
    {
        return text == "explicit" || text == "inline" || text == "constexpr" || text == "consteval";
    }

    static TkString trimCopy(const TkString& input)
    {
        i32 begin = 0;
        i32 end = input.length();
        while (begin < end && std::isspace(static_cast<unsigned char>(input[begin]))) begin++;
        while (end > begin && std::isspace(static_cast<unsigned char>(input[end - 1]))) end--;
        return input.substring(begin, end - begin);
    }

    static TkString tokenText(const Token& token)
    {
        if (token.type == TokenType::kEOL || token.type == TokenType::kEOF || token.type == TokenType::kUnknown)
            return TkString::Empty;
        if (!token.value.isEmpty())
            return token.value;

        switch (token.type)
        {
        case TokenType::kEqual: return "=";
        case TokenType::kComma: return ",";
        case TokenType::kSemiColon: return ";";
        case TokenType::kLCurlyBrace: return "{";
        case TokenType::kRCurlyBrace: return "}";
        case TokenType::kLParen: return "(";
        case TokenType::kRParen: return ")";
        case TokenType::kLBracket: return "[";
        case TokenType::kRBracket: return "]";
        case TokenType::kColon: return ":";
        case TokenType::kDoubleColon: return "::";
        case TokenType::kDot: return ".";
        case TokenType::kLessThan: return "<";
        case TokenType::kGreaterThan: return ">";
        case TokenType::kAnd: return "&";
        case TokenType::kLogicAnd: return "&&";
        case TokenType::kBitOr: return "|";
        case TokenType::kMultiply: return "*";
        case TokenType::kDivide: return "/";
        case TokenType::kAdd: return "+";
        case TokenType::kSub: return "-";
        case TokenType::kArrow: return "->";
        case TokenType::kNot: return "!";
        case TokenType::kBitNot: return "~";
        case TokenType::kQuestion: return "?";
        case TokenType::kPercent: return "%";
        case TokenType::kXor: return "^";
        case TokenType::kEllipsis: return "...";
        default: return TkString::Empty;
        }
    }

    static bool joinsTightly(TokenType prev, TokenType next)
    {
        if (prev == TokenType::kDoubleColon || next == TokenType::kDoubleColon)
            return true;
        if (next == TokenType::kComma || next == TokenType::kSemiColon || next == TokenType::kRParen ||
            next == TokenType::kRBracket || next == TokenType::kRAngleBracket)
            return true;
        if (prev == TokenType::kLParen || prev == TokenType::kLBracket || prev == TokenType::kLAngleBracket)
            return true;
        if (next == TokenType::kLParen || next == TokenType::kLBracket || next == TokenType::kLAngleBracket)
            return true;
        if (next == TokenType::kAnd || next == TokenType::kLogicAnd || next == TokenType::kBitOr || next == TokenType::kMultiply)
            return true;
        if (prev == TokenType::kAnd || prev == TokenType::kLogicAnd || prev == TokenType::kBitOr || prev == TokenType::kMultiply)
            return true;
        return false;
    }

    static TkString tokensToText(const TkArray<Token>& tokens, i32 begin, i32 end)
    {
        TkStringBuilder out;
        TokenType prev = TokenType::kUnknown;
        bool hasPrev = false;
        for (i32 i = begin; i < end; i++)
        {
            const TkString text = tokenText(tokens[i]);
            if (text.isEmpty())
                continue;
            if (hasPrev && !joinsTightly(prev, tokens[i].type))
                out.append(' ');
            out.append(text);
            prev = tokens[i].type;
            hasPrev = true;
        }
        return trimCopy(out.toString());
    }

    static bool parseIntegerLiteral(const TkString& text, i64& out)
    {
        if (text.isEmpty())
            return false;
        const char* raw = text.getText();
        int base = 10;
        i32 idx = 0;
        if (text.length() > 2 && raw[0] == '0' && (raw[1] == 'x' || raw[1] == 'X'))
        {
            base = 16;
            idx = 2;
        }
        i64 value = 0;
        for (; idx < text.length(); idx++)
        {
            const char c = raw[idx];
            if (c == 'u' || c == 'U' || c == 'l' || c == 'L')
                break;
            i32 digit = -1;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = 10 + c - 'a';
            else if (c >= 'A' && c <= 'F') digit = 10 + c - 'A';
            else return false;
            if (digit >= base)
                return false;
            value = value * base + digit;
        }
        out = value;
        return true;
    }

    Token& CXXParser::current() const { return m_current; }

    Token& CXXParser::peek(i32 offset) const
    {
        const i32 idx = m_tk_idx + offset;
        if (idx < 0 || idx >= m_lex->m_tokens.count())
            return Token::Empty;
        return m_lex->m_tokens[idx];
    }

    Token& CXXParser::peekPrevious() const { return peek(-1); }

    Token& CXXParser::advance()
    {
        if (m_tk_idx + 1 >= m_lex->m_tokens.count())
            return Token::Empty;
        m_tk_idx++;
        m_current = m_lex->m_tokens[m_tk_idx];
        return m_current;
    }

    Token& CXXParser::previous()
    {
        if (m_tk_idx - 1 < 0)
            return Token::Empty;
        m_tk_idx--;
        m_current = m_lex->m_tokens[m_tk_idx];
        return m_current;
    }

    bool CXXParser::match(TokenType t) const { return m_current.type == t; }
    bool CXXParser::expect(TokenType t) { return peek().type == t; }
    bool CXXParser::isIdentifier(const char* text) const { return current().type == TokenType::kIdentifier && (text == nullptr || current().value == text); }
    bool CXXParser::consume(TokenType t) { if (!match(t)) return false; advance(); return true; }
    bool CXXParser::consumeIdentifier(const char* text) { if (!isIdentifier(text)) return false; advance(); return true; }

    CXXParser::CXXParser(CXXLexer* lex, CXXSourceFile* f)
        : m_root(nullptr)
    {
        TK_ASSERT(lex);
        m_lex = lex;
        m_src = f;
    }

    CXXParser::~CXXParser()
    {
    }

    ASTTransUnit* CXXParser::takeRoot()
    {
        ASTTransUnit* result = m_root;
        m_root = nullptr;
        return result;
    }

    bool CXXParser::parse()
    {
        if (m_root)
            return true;
        if (m_lex->m_tokens.isEmpty())
            return false;

        m_root = TkMemory::newObject<ASTTransUnit>();
        m_tk_idx = 0;
        m_current = m_lex->m_tokens[0];

        if (const auto r = parseRoot(); !r)
        {
            m_diag.printAll();
            return false;
        }

        auto* ptr = TkMemory::newObject<MarcoPicker>();
        ptr->m_marcos.swap(m_marcos);
        m_src->m_ctx->m_bufferedMarcos.add(m_src->m_ctx->parseRelivite(m_src->m_path), ptr);
        return true;
    }

    Expected<void> CXXParser::parseRoot()
    {
        while (!match(TokenType::kEOF))
        {
            if (match(TokenType::kEOL) || match(TokenType::kSemiColon))
            {
                advance();
            }
            else if (match(TokenType::kPreprocessor))
            {
                if (expect(TokenType::kIdentifier) && peek().value == "include")
                {
                    advance();
                    CHECK_RESULT(r, parseInclude());
                    m_src->m_includes.add(r.value());
                    m_src->m_ctx->parseMarcoOnlyFile(r.value());
                }
                else
                {
                    CHECK_RESULT(r, jumpUnneed());
                }
            }
            else if (isIdentifier("rkattr"))
            {
                CHECK_RESULT(r, parsePendingAttributes());
            }
            else if (isIdentifier("template"))
            {
                m_skipNextClass = true;
                CHECK_RESULT(r, jumpUnneed());
            }
            else if (isIdentifier("namespace"))
            {
                m_skipNextClass = false;
                CHECK_RESULT(r, parseNamespace());
                m_root->decls.add(r.value());
            }
            else if (isIdentifier("rkclass"))
            {
                if (m_skipNextClass) { m_skipNextClass = false; CHECK_RESULT(r, jumpUnneed()); }
                else { CHECK_RESULT(r, parseCClass()); m_root->decls.add(r.value()); }
            }
            else if (isIdentifier("rkstruct"))
            {
                if (m_skipNextClass) { m_skipNextClass = false; CHECK_RESULT(r, jumpUnneed()); }
                else { CHECK_RESULT(r, parseCStruct()); m_root->decls.add(r.value()); }
            }
            else if (isIdentifier("rkenum"))
            {
                m_skipNextClass = false;
                CHECK_RESULT(r, parseCEnum());
                m_root->decls.add(r.value());
            }
            else
            {
                m_skipNextClass = false;
                CHECK_RESULT(r, jumpUnneed());
            }
        }
        return Error::success();
    }

    Expected<TkString> CXXParser::parseInclude()
    {
        if (!isIdentifier("include"))
            return Error::failure("Invalid include expression", ERRR());
        advance();

        if (match(TokenType::kString))
        {
            TkString val = current().value;
            jumpLine();
            if (match(TokenType::kEOL))
                advance();
            return val;
        }

        if (!match(TokenType::kLAngleBracket))
            return Error::failure("Invalid include expression, include needs a file path", ERRR());

        TkStringBuilder includePath;
        advance();
        while (!match(TokenType::kRAngleBracket) && !match(TokenType::kEOL) && !match(TokenType::kEOF))
        {
            includePath.append(tokenText(current()));
            advance();
        }
        if (!match(TokenType::kRAngleBracket))
            return Error::failure("Invalid include expression, include needs closing '>'", ERRR());

        TkString val = includePath.toString();
        jumpLine();
        if (match(TokenType::kEOL))
            advance();
        return val;
    }

    Expected<ASTNamespace*> CXXParser::parseNamespace()
    {
        if (!consumeIdentifier("namespace"))
            return Error::failure("Expected namespace", ERRR());

        TkStringBuilder name;
        while (!match(TokenType::kLCurlyBrace) && !match(TokenType::kEOF))
        {
            if (match(TokenType::kIdentifier) || match(TokenType::kDoubleColon))
                name.append(tokenText(current()));
            advance();
        }
        if (!consume(TokenType::kLCurlyBrace))
            return Error::failure("Expect namespace body", ERRR());

        ScopeGuard ns = TkMemory::newObject<ASTNamespace>(TkMemory::getDefault(), name.toString());
        while (!match(TokenType::kRCurlyBrace) && !match(TokenType::kEOF))
        {
            const i32 tokenIndexBefore = m_tk_idx;
            if (match(TokenType::kEOL) || match(TokenType::kSemiColon))
                advance();
            else if (isIdentifier("rkattr"))
            {
                CHECK_RESULT(r, parsePendingAttributes());
            }
            else if (isIdentifier("template"))
            {
                m_skipNextClass = true;
                CHECK_RESULT(r, jumpUnneed());
            }
            else if (isIdentifier("namespace"))
            {
                m_skipNextClass = false;
                CHECK_RESULT(r, parseNamespace());
                ns->decls.add(r.value());
            }
            else if (isIdentifier("rkclass"))
            {
                if (m_skipNextClass) { m_skipNextClass = false; CHECK_RESULT(r, jumpUnneed()); }
                else { CHECK_RESULT(r, parseCClass()); ns->decls.add(r.value()); }
            }
            else if (isIdentifier("rkstruct"))
            {
                if (m_skipNextClass) { m_skipNextClass = false; CHECK_RESULT(r, jumpUnneed()); }
                else { CHECK_RESULT(r, parseCStruct()); ns->decls.add(r.value()); }
            }
            else if (isIdentifier("rkenum"))
            {
                m_skipNextClass = false;
                CHECK_RESULT(r, parseCEnum());
                ns->decls.add(r.value());
            }
            else
            {
                m_skipNextClass = false;
                CHECK_RESULT(r, jumpUnneed());
            }
            if (m_tk_idx == tokenIndexBefore)
                advance();
        }
        consume(TokenType::kRCurlyBrace);
        return ns.getPtr();
    }

    Expected<TkString> CXXParser::parseDeclName()
    {
        TkString lastIdentifier;
        i32 parenDepth = 0;
        while (!match(TokenType::kLCurlyBrace) && !match(TokenType::kColon) && !match(TokenType::kEOF))
        {
            if (match(TokenType::kLParen)) parenDepth++;
            else if (match(TokenType::kRParen) && parenDepth > 0) parenDepth--;
            else if (parenDepth == 0 && match(TokenType::kIdentifier))
            {
                if (current().value != "final" && current().value != "sealed")
                    lastIdentifier = current().value;
            }
            advance();
        }
        if (lastIdentifier.isEmpty())
            return Error::failure("Expected declaration name", ERRR());
        return lastIdentifier;
    }

    void CXXParser::parseBaseList(TkArray<TkString>* bases)
    {
        TkArray<Token> chunk;
        i32 angleDepth = 0;
        while (!match(TokenType::kEOF) && !match(TokenType::kLCurlyBrace))
        {
            Token& t = current();
            if (t.type == TokenType::kLAngleBracket) angleDepth++;
            else if (t.type == TokenType::kRAngleBracket && angleDepth > 0) angleDepth--;
            if (t.type == TokenType::kComma && angleDepth == 0)
            {
                TkString text = tokensToText(chunk, 0, chunk.count());
                text.replace("public ", "");
                text.replace("protected ", "");
                text.replace("private ", "");
                if (!text.isEmpty()) bases->add(trimCopy(text));
                chunk.clear();
                advance();
                continue;
            }
            chunk.add(t);
            advance();
        }
        TkString text = tokensToText(chunk, 0, chunk.count());
        text.replace("public ", "");
        text.replace("protected ", "");
        text.replace("private ", "");
        if (!text.isEmpty()) bases->add(trimCopy(text));
    }

    Expected<ASTTypeDecl*> CXXParser::parseCClass()
    {
        if (!consumeIdentifier("rkclass"))
            return Error::failure("Expected cclass", ERRR());

        CHECK_RESULT(nameResult, parseDeclName());
        ScopeGuard type = TkMemory::newObject<ASTTypeDecl>(TkMemory::getDefault(), nameResult.value(), ASTTypeDecl::DeclType::kClass);
        applyPendingAttributes(type.operator->());

        if (match(TokenType::kColon))
        {
            advance();
            parseBaseList(&type->baseClasses);
        }
        if (!consume(TokenType::kLCurlyBrace))
            return Error::failure("Expected cclass body", ERRR());

        bool isStatic = false;
        while (!match(TokenType::kRCurlyBrace) && !match(TokenType::kEOF))
        {
            if (match(TokenType::kEOL) || match(TokenType::kSemiColon)) { advance(); continue; }
            const i32 tokenIndexBefore = m_tk_idx;
            if (isIdentifier("rkctor"))
            {
                advance();
                while (match(TokenType::kIdentifier) && isConstructorSpecifier(current().value))
                    advance();
                CHECK_RESULT(r, parseCtor(type->name, true));
                applyPendingAttributes(r.value());
                type->members.add(r.value());
            }
            else
            if (isIdentifier(type->name.getText()) && peek().type == TokenType::kLParen) { CHECK_RESULT(r, parseCtor(type->name)); applyPendingAttributes(r.value()); type->members.add(r.value()); }
            else if (isIdentifier("explicit") && peek().type == TokenType::kIdentifier && peek().value == type->name)
            {
                advance();
                CHECK_RESULT(r, parseCtor(type->name));
                applyPendingAttributes(r.value());
                type->members.add(r.value());
            }
            else if (isIdentifier("static")) { isStatic = true; advance(); continue; }
            else if (isIdentifier("rkattr")) { CHECK_RESULT(r, parsePendingAttributes()); }
            else if (isIdentifier("rkfunc")) { CHECK_RESULT(r, parseCFunction(isStatic)); applyPendingAttributes(r.value()); type->members.add(r.value()); }
            else if (isIdentifier("rkfield")) { CHECK_RESULT(r, parseCField(isStatic)); applyPendingAttributes(r.value()); type->members.add(r.value()); }
            else if (isIdentifier("rkenum")) { CHECK_RESULT(r, parseCEnum()); type->members.add(r.value()); }
            else if (isIdentifier("rkstruct")) { CHECK_RESULT(r, parseCStruct()); type->members.add(r.value()); }
            else if (isIdentifier("rkclass")) { CHECK_RESULT(r, parseCClass()); type->members.add(r.value()); }
            else if (isIdentifier("public") || isIdentifier("private") || isIdentifier("protected"))
            {
                advance();
                consume(TokenType::kColon);
            }
            else
            {
                // Detect R_OBJECT_NO_SPAWN(X) and similar macros that signal no-spawn
                if (match(TokenType::kIdentifier) && current().value.find("NO_SPAWN") >= 0)
                    type->noSpawn = true;
                CHECK_RESULT(r, jumpUnneed());
            }
            if (m_tk_idx == tokenIndexBefore)
                advance();
            isStatic = false;
        }
        consume(TokenType::kRCurlyBrace);
        consume(TokenType::kSemiColon);
        return type.getPtr();
    }

    Expected<ASTTypeDecl*> CXXParser::parseCStruct()
    {
        if (!consumeIdentifier("rkstruct"))
            return Error::failure("Expected cstruct", ERRR());
        CHECK_RESULT(nameResult, parseDeclName());
        ScopeGuard type = TkMemory::newObject<ASTTypeDecl>(TkMemory::getDefault(), nameResult.value(), ASTTypeDecl::DeclType::kStruct);
        applyPendingAttributes(type.operator->());
        if (match(TokenType::kColon))
        {
            advance();
            parseBaseList(&type->baseClasses);
        }
        if (!consume(TokenType::kLCurlyBrace))
            return Error::failure("Expected cstruct body", ERRR());

        bool isStatic = false;
        while (!match(TokenType::kRCurlyBrace) && !match(TokenType::kEOF))
        {
            if (match(TokenType::kEOL) || match(TokenType::kSemiColon)) { advance(); continue; }
            const i32 tokenIndexBefore = m_tk_idx;
            if (isIdentifier("rkctor"))
            {
                advance();
                while (match(TokenType::kIdentifier) && isConstructorSpecifier(current().value))
                    advance();
                CHECK_RESULT(r, parseCtor(type->name, true));
                applyPendingAttributes(r.value());
                type->members.add(r.value());
            }
            else if (isIdentifier(type->name.getText()) && peek().type == TokenType::kLParen) { CHECK_RESULT(r, parseCtor(type->name)); applyPendingAttributes(r.value()); type->members.add(r.value()); }
            else if (isIdentifier("explicit") && peek().type == TokenType::kIdentifier && peek().value == type->name)
            {
                advance();
                CHECK_RESULT(r, parseCtor(type->name));
                applyPendingAttributes(r.value());
                type->members.add(r.value());
            }
            else if (isIdentifier("static")) { isStatic = true; advance(); continue; }
            else if (isIdentifier("rkattr")) { CHECK_RESULT(r, parsePendingAttributes()); }
            else if (isIdentifier("rkfunc")) { CHECK_RESULT(r, parseCFunction(isStatic)); applyPendingAttributes(r.value()); type->members.add(r.value()); }
            else if (isIdentifier("rkfield")) { CHECK_RESULT(r, parseCField(isStatic)); applyPendingAttributes(r.value()); type->members.add(r.value()); }
            else if (isIdentifier("rkenum")) { CHECK_RESULT(r, parseCEnum()); type->members.add(r.value()); }
            else if (isIdentifier("rkstruct")) { CHECK_RESULT(r, parseCStruct()); type->members.add(r.value()); }
            else if (isIdentifier("rkclass")) { CHECK_RESULT(r, parseCClass()); type->members.add(r.value()); }
            else if (isIdentifier("public") || isIdentifier("private") || isIdentifier("protected"))
            {
                advance();
                consume(TokenType::kColon);
            }
            else if (match(TokenType::kIdentifier) && isBuiltinSkippedMember(current().value)) { CHECK_RESULT(r, jumpUnneed()); }
            else { CHECK_RESULT(r, parseImplicitStructField(isStatic)); applyPendingAttributes(r.value()); type->members.add(r.value()); }
            if (m_tk_idx == tokenIndexBefore)
                advance();
            isStatic = false;
        }
        consume(TokenType::kRCurlyBrace);
        consume(TokenType::kSemiColon);
        return type.getPtr();
    }

    Expected<ASTEnum*> CXXParser::parseCEnum()
    {
        if (!consumeIdentifier("rkenum"))
            return Error::failure("Expected cenum", ERRR());

        bool enumClass = false;
        if (isIdentifier("class") || isIdentifier("struct"))
        {
            enumClass = true;
            advance();
        }
        if (!match(TokenType::kIdentifier))
            return Error::failure("Expected enum name", ERRR());
        ScopeGuard e = TkMemory::newObject<ASTEnum>(TkMemory::getDefault(), current().value);
        applyPendingAttributes(e.operator->());
        e->isEnumClass = enumClass;
        advance();
        while (match(TokenType::kEOL))
            advance();

        if (match(TokenType::kColon))
        {
            advance();
            TkArray<Token> underlying;
            while (!match(TokenType::kLCurlyBrace) && !match(TokenType::kEOF))
            {
                underlying.add(current());
                advance();
            }
            e->underlyingType = tokensToText(underlying, 0, underlying.count());
        }
        if (!consume(TokenType::kLCurlyBrace))
            return Error::failure("Expected enum body", ERRR());

        i64 nextValue = 0;
        while (!match(TokenType::kRCurlyBrace) && !match(TokenType::kEOF))
        {
            if (match(TokenType::kEOL) || match(TokenType::kComma)) { advance(); continue; }
            if (!match(TokenType::kIdentifier)) { advance(); continue; }
            ASTEnumValue item;
            item.name = current().value;
            advance();
            if (match(TokenType::kEqual))
            {
                advance();
                TkArray<Token> expr;
                while (!match(TokenType::kComma) && !match(TokenType::kRCurlyBrace) && !match(TokenType::kEOF))
                {
                    expr.add(current());
                    advance();
                }
                item.rawValue = tokensToText(expr, 0, expr.count());
                i64 parsed = 0;
                if (expr.count() == 1 && parseIntegerLiteral(expr[0].value, parsed))
                {
                    item.value = parsed;
                    nextValue = parsed + 1;
                }
            }
            else
            {
                item.value = nextValue++;
            }
            e->values.add(item);
        }
        consume(TokenType::kRCurlyBrace);
        consume(TokenType::kSemiColon);
        return e.getPtr();
    }

    Expected<ASTFunction*> CXXParser::parseCFunction(bool isStatic)
    {
        if (!consumeIdentifier("rkfunc"))
            return Error::failure("Invalid scope for parsing cfunc", ERRR());

        if (match(TokenType::kLParen))
        {
            i32 depth = 1;
            advance();
            while (depth > 0 && !match(TokenType::kEOF))
            {
                if (depth == 1 && isIdentifier("static"))
                    isStatic = true;
                if (match(TokenType::kLParen)) depth++;
                else if (match(TokenType::kRParen)) depth--;
                advance();
            }
        }

        TkString type;
        TkString name;
        CHECK_RESULT(r1, parseFuncTypeAndName(&type, &name, isStatic));
        ScopeGuard f = TkMemory::newObject<ASTFunction>();
        f->name = name;
        f->returnType = type;
        f->isStatic = isStatic;

        TkArray<ASTParam> args;
        CHECK_RESULT(r2, parseFuncArgs(&args));
        args.swap(f->params);

        while (!match(TokenType::kEOF))
        {
            if (match(TokenType::kEOL))
            {
                advance();
                continue;
            }
            if (isIdentifier("const")) { f->isConst = true; advance(); continue; }
            if (isIdentifier("noexcept")) { f->isNoexcept = true; advance(); continue; }
            if (match(TokenType::kSemiColon)) { advance(); break; }
            if (match(TokenType::kLCurlyBrace)) { CHECK_RESULT(r, jumpUnneed()); break; }
            advance();
        }
        return f.getPtr();
    }

    Expected<ASTFunction*> CXXParser::parseCtor(const TkString& name, bool isReflectConstructor)
    {
        if (!consumeIdentifier(name.getText()))
            return Error::failure("Expected constructor name", ERRR());
        ScopeGuard f = TkMemory::newObject<ASTFunction>();
        f->name = name;
        f->isConstructor = true;
        f->isReflectConstructor = isReflectConstructor;
        TkArray<ASTParam> args;
        CHECK_RESULT(result, parseFuncArgs(&args));
        args.swap(f->params);

        while (!match(TokenType::kSemiColon) && !match(TokenType::kEOF))
        {
            if (match(TokenType::kLCurlyBrace))
            {
                CHECK_RESULT(r, jumpUnneed());
                break;
            }
            advance();
        }
        consume(TokenType::kSemiColon);
        return f.getPtr();
    }

    Expected<void> CXXParser::parseFuncTypeAndName(TkString* type, TkString* name, bool& isStatic)
    {
        TkArray<Token> sig;
        i32 angleDepth = 0;
        while (!match(TokenType::kLParen) && !match(TokenType::kEOF))
        {
            if (isIdentifier("static")) { isStatic = true; advance(); continue; }
            if (isIdentifier("inline") || isIdentifier("constexpr") || isIdentifier("virtual")) { advance(); continue; }
            if (match(TokenType::kIdentifier) && isMarco(current().value)) { advance(); continue; }
            if (match(TokenType::kLAngleBracket)) angleDepth++;
            else if (match(TokenType::kRAngleBracket) && angleDepth > 0) angleDepth--;
            sig.add(current());
            advance();
        }
        i32 nameIndex = -1;
        for (i32 i = sig.count() - 1; i >= 0; i--)
        {
            if (sig[i].type == TokenType::kIdentifier)
            {
                nameIndex = i;
                break;
            }
        }
        if (nameIndex <= 0)
            return Error::failure("Invalid function declaration, need [type] [name]", ERRR());
        *name = sig[nameIndex].value;
        *type = tokensToText(sig, 0, nameIndex);
        return Error::success();
    }

    Expected<void> CXXParser::parseFuncArgs(TkArray<ASTParam>* out)
    {
        out->clear();
        if (!consume(TokenType::kLParen))
            return Error::failure("Invalid scope for parsing function arguments", ERRR());
        i32 argIndex = 0;
        while (!match(TokenType::kRParen) && !match(TokenType::kEOF))
        {
            if (match(TokenType::kComma)) { advance(); continue; }
            TkArray<Token> arg;
            i32 angleDepth = 0, parenDepth = 0, bracketDepth = 0;
            while (!match(TokenType::kEOF))
            {
                if (angleDepth == 0 && parenDepth == 0 && bracketDepth == 0 && (match(TokenType::kComma) || match(TokenType::kRParen)))
                    break;
                if (match(TokenType::kEqual) && angleDepth == 0 && parenDepth == 0 && bracketDepth == 0)
                {
                    while (!match(TokenType::kEOF) && !(angleDepth == 0 && parenDepth == 0 && bracketDepth == 0 && (match(TokenType::kComma) || match(TokenType::kRParen))))
                    {
                        advance();
                    }
                    break;
                }
                if (match(TokenType::kLAngleBracket)) angleDepth++;
                else if (match(TokenType::kRAngleBracket) && angleDepth > 0) angleDepth--;
                else if (match(TokenType::kLParen)) parenDepth++;
                else if (match(TokenType::kRParen) && parenDepth > 0) parenDepth--;
                else if (match(TokenType::kLBracket)) bracketDepth++;
                else if (match(TokenType::kRBracket) && bracketDepth > 0) bracketDepth--;
                arg.add(current());
                advance();
            }
            if (arg.count() > 0)
            {
                if (arg.count() == 1 && arg[0].type == TokenType::kIdentifier && arg[0].value == "void")
                    continue;
                i32 nameIndex = -1;
                for (i32 i = arg.count() - 1; i >= 0; i--)
                {
                    if (arg[i].type == TokenType::kIdentifier)
                    {
                        nameIndex = i;
                        break;
                    }
                }
                TkString argName = TkString::format("_arg%i", argIndex);
                i32 typeEnd = arg.count();
                if (nameIndex > 0)
                {
                    argName = arg[nameIndex].value;
                    typeEnd = nameIndex;
                }
                ASTParam param;
                param.type = tokensToText(arg, 0, typeEnd);
                param.name = argName;
                out->add(param);
                argIndex++;
            }
            consume(TokenType::kComma);
        }
        consume(TokenType::kRParen);
        return Error::success();
    }

    Expected<void> CXXParser::parsePendingAttributes()
    {
        if (!consumeIdentifier("rkattr"))
            return Error::failure("Expected rkattr", ERRR());
        if (!consume(TokenType::kLParen))
            return Error::failure("Expected rkattr body", ERRR());

        TkArray<Token> chunk;
        i32 depth = 0;
        while (!match(TokenType::kEOF))
        {
            if (match(TokenType::kLParen))
                depth++;
            else if (match(TokenType::kRParen))
            {
                if (depth == 0)
                {
                    const TkString attr = trimCopy(tokensToText(chunk, 0, chunk.count()));
                    if (!attr.isEmpty())
                        m_pendingAttributes.add(attr);
                    advance();
                    break;
                }
                depth--;
            }

            if (match(TokenType::kComma) && depth == 0)
            {
                const TkString attr = trimCopy(tokensToText(chunk, 0, chunk.count()));
                if (!attr.isEmpty())
                    m_pendingAttributes.add(attr);
                chunk.clear();
                advance();
                continue;
            }

            chunk.add(current());
            advance();
        }

        while (!match(TokenType::kSemiColon) && !match(TokenType::kEOL) && !match(TokenType::kEOF))
            advance();
        consume(TokenType::kSemiColon);
        if (match(TokenType::kEOL))
            advance();
        return Error::success();
    }

    void CXXParser::applyPendingAttributes(ASTTypeDecl* type)
    {
        if (type != nullptr && !m_pendingAttributes.isEmpty())
            type->attributes.add(m_pendingAttributes);
        m_pendingAttributes.clear();
    }

    void CXXParser::applyPendingAttributes(ASTField* field)
    {
        if (field != nullptr && !m_pendingAttributes.isEmpty())
            field->attributes.add(m_pendingAttributes);
        m_pendingAttributes.clear();
    }

    void CXXParser::applyPendingAttributes(ASTFunction* function)
    {
        if (function != nullptr && !m_pendingAttributes.isEmpty())
            function->attributes.add(m_pendingAttributes);
        m_pendingAttributes.clear();
    }

    static void applyFieldMeta(ASTField* f, const TkString& raw, i32 positional)
    {
        TkString item = trimCopy(raw);
        if (item.isEmpty())
            return;
        const i32 eq = item.find('=');
        if (eq >= 0)
        {
            TkString key = trimCopy(item.substring(0, eq));
            TkString value = trimCopy(item.substring(eq + 1));
            if (key == "get") f->getFunc = value;
            else if (key == "set") f->setFunc = value;
            else f->attributes.add(item);
            return;
        }
        if (item == "readonly") f->isReadonly = true;
        else if (item == "transient") f->isTransient = true;
        else if (item.startsWith(TkStringView{"attr"})) f->attributes.add(item);
        else if (positional == 0) f->getFunc = item;
        else if (positional == 1) f->setFunc = item;
        else f->attributes.add(item);
    }

    Expected<ASTField*> CXXParser::parseCField(bool isStatic)
    {
        if (!consumeIdentifier("rkfield"))
            return Error::failure("Expected cfield", ERRR());
        ScopeGuard f = TkMemory::newObject<ASTField>();
        f->isStatic = isStatic;

        if (match(TokenType::kLParen))
        {
            advance();
            TkArray<Token> chunk;
            i32 depth = 0;
            i32 positional = 0;
            while (!match(TokenType::kEOF))
            {
                if (match(TokenType::kLParen)) depth++;
                else if (match(TokenType::kRParen))
                {
                    if (depth == 0)
                    {
                        applyFieldMeta(f.operator->(), tokensToText(chunk, 0, chunk.count()), positional);
                        advance();
                        break;
                    }
                    depth--;
                }
                if (match(TokenType::kComma) && depth == 0)
                {
                    applyFieldMeta(f.operator->(), tokensToText(chunk, 0, chunk.count()), positional);
                    chunk.clear();
                    positional++;
                    advance();
                    continue;
                }
                chunk.add(current());
                advance();
            }
        }

        TkArray<Token> decl;
        while (!match(TokenType::kSemiColon) && !match(TokenType::kEOF))
        {
            if (isIdentifier("static"))
            {
                f->isStatic = true;
                advance();
                continue;
            }
            if (match(TokenType::kEqual))
            {
                while (!match(TokenType::kSemiColon) && !match(TokenType::kEOF))
                    advance();
                break;
            }
            decl.add(current());
            advance();
        }
        consume(TokenType::kSemiColon);
        if (decl.isEmpty())
            return Error::failure("Empty cfield declaration", ERRR());

        i32 nameIndex = -1;
        for (i32 i = decl.count() - 1; i >= 0; i--)
        {
            if (decl[i].type == TokenType::kIdentifier)
            {
                nameIndex = i;
                break;
            }
        }
        if (nameIndex <= 0)
            return Error::failure("Invalid cfield declaration", ERRR());
        f->name = decl[nameIndex].value;
        f->type = tokensToText(decl, 0, nameIndex);
        f->isConst = f->type.find("const") >= 0;
        f->isPointer = f->type.find("*") >= 0;
        f->isReference = f->type.find("&") >= 0;
        return f.getPtr();
    }

    Expected<ASTField*> CXXParser::parseImplicitStructField(bool isStatic)
    {
        ScopeGuard f = TkMemory::newObject<ASTField>();
        f->isStatic = isStatic;
        TkArray<Token> decl;
        bool looksLikeFunction = false;
        while (!match(TokenType::kSemiColon) && !match(TokenType::kEOF))
        {
            if (isIdentifier("operator") || match(TokenType::kBitNot))
                looksLikeFunction = true;
            if (!looksLikeFunction && match(TokenType::kEqual))
            {
                while (!match(TokenType::kSemiColon) && !match(TokenType::kEOF))
                    advance();
                break;
            }
            if (match(TokenType::kLParen))
                looksLikeFunction = true;
            if (looksLikeFunction && match(TokenType::kLCurlyBrace))
            {
                CHECK_RESULT(r, jumpUnneed());
                return f.getPtr();
            }
            decl.add(current());
            advance();
        }
        consume(TokenType::kSemiColon);
        if (looksLikeFunction)
            return f.getPtr();
        i32 nameIndex = -1;
        for (i32 i = decl.count() - 1; i >= 0; i--)
        {
            if (decl[i].type == TokenType::kIdentifier)
            {
                nameIndex = i;
                break;
            }
        }
        if (nameIndex <= 0)
            return f.getPtr();
        f->name = decl[nameIndex].value;
        f->type = tokensToText(decl, 0, nameIndex);
        f->isConst = f->type.find("const") >= 0;
        f->isPointer = f->type.find("*") >= 0;
        f->isReference = f->type.find("&") >= 0;
        return f.getPtr();
    }

    // Evaluate a preprocessor condition token after the directive keyword.
    // Returns true only for "#ifdef X" / "#ifndef X" / "#if X" / "#if 1" / "#if defined(X)"
    // where X is a known macro. Everything else is conservatively false.
    bool CXXParser::evalPreprocessorCondition()
    {
        if (isIdentifier("defined"))
        {
            advance();
            if (consume(TokenType::kLParen))
            {
                bool r = false;
                if (match(TokenType::kIdentifier))
                    r = isMarco(current().value);
                while (!match(TokenType::kRParen) && !match(TokenType::kEOL) && !match(TokenType::kEOF))
                    advance();
                consume(TokenType::kRParen);
                return r;
            }
            return false;
        }
        if (match(TokenType::kIdentifier))
            return isMarco(current().value);
        // numeric literal "1" → true; "0" / anything else → false
        if (!current().value.isEmpty() && current().value == "1")
            return true;
        return false;
    }

    // Skip tokens inside a false preprocessor block.
    // If stopAtElse=true, stops at the matching #else/#elif (so the else-block gets parsed normally).
    // If stopAtElse=false, skips all the way to #endif.
    void CXXParser::skipConditionalBlock(bool stopAtElse)
    {
        jumpLine();
        if (match(TokenType::kEOL))
            advance();

        i32 depth = 1;
        while (!match(TokenType::kEOF))
        {
            if (match(TokenType::kPreprocessor))
            {
                advance();
                if (isIdentifier("if") || isIdentifier("ifdef") || isIdentifier("ifndef"))
                {
                    depth++;
                }
                else if (isIdentifier("endif"))
                {
                    depth--;
                    if (depth == 0)
                    {
                        jumpLine();
                        if (match(TokenType::kEOL)) advance();
                        return;
                    }
                }
                else if (stopAtElse && depth == 1 && (isIdentifier("else") || isIdentifier("elif")))
                {
                    jumpLine();
                    if (match(TokenType::kEOL)) advance();
                    return;
                }
                jumpLine();
                if (match(TokenType::kEOL)) advance();
                continue;
            }
            advance();
        }
    }

    Expected<void> CXXParser::jumpUnneed()
    {
        if (match(TokenType::kPreprocessor))
        {
            advance();
            if (isIdentifier("define"))
            {
                advance();
                if (match(TokenType::kIdentifier))
                    m_marcos.add(current().value);
            }
            else if (isIdentifier("undef"))
            {
                advance();
                if (match(TokenType::kIdentifier))
                    m_marcos.remove(current().value);
            }
            else if (isIdentifier("ifdef") || isIdentifier("ifndef"))
            {
                const bool isIfdef = isIdentifier("ifdef");
                advance();
                bool condTrue = false;
                if (match(TokenType::kIdentifier))
                {
                    condTrue = isMarco(current().value);
                    if (!isIfdef) condTrue = !condTrue;
                }
                if (!condTrue)
                {
                    skipConditionalBlock(true);
                    return Error::success();
                }
                // condTrue: skip rest of directive line and let the block parse normally
            }
            else if (isIdentifier("if"))
            {
                advance();
                if (!evalPreprocessorCondition())
                {
                    skipConditionalBlock(true);
                    return Error::success();
                }
            }
            else if (isIdentifier("else") || isIdentifier("elif"))
            {
                // We finished parsing a true-block; skip to #endif
                skipConditionalBlock(false);
                return Error::success();
            }
            else if (isIdentifier("endif"))
            {
                // End of a conditional we already parsed; just consume it
            }
            jumpLine();
            if (match(TokenType::kEOL))
                advance();
            return Error::success();
        }

        i32 curly = 0;
        i32 paren = 0;
        do
        {
            if (match(TokenType::kLCurlyBrace)) curly++;
            else if (match(TokenType::kRCurlyBrace))
            {
                if (curly == 0)
                    return Error::success();
                curly--;
                advance();
                if (curly == 0)
                    return Error::success();
                continue;
            }
            else if (match(TokenType::kLParen)) paren++;
            else if (match(TokenType::kRParen) && paren > 0) paren--;
            else if ((match(TokenType::kSemiColon) || match(TokenType::kEOL)) && curly == 0 && paren == 0)
            {
                advance();
                return Error::success();
            }
            else if (match(TokenType::kEOF))
            {
                return Error::success();
            }
            advance();
        } while (true);
    }

    void CXXParser::jumpLine()
    {
        while (!match(TokenType::kEOL) && !match(TokenType::kEOF))
            advance();
    }

    bool CXXParser::isMarco(const TkStringView& c) const
    {
        if (m_marcos.find(c) != -1)
            return true;
        return m_src->m_ctx->hasMarco(c);
    }

} // namespace toolkit
