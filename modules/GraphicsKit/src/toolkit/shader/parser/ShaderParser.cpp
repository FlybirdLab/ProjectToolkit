/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/parser/ShaderParser.hpp>
#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>

namespace toolkit
{
    namespace
    {
        bool isStageKeyword(ShaderTokenKind k)
        {
            return k == ShaderTokenKind::KeywordVertex
                || k == ShaderTokenKind::KeywordPixel
                || k == ShaderTokenKind::KeywordCompute;
        }

        ShaderStage stageFromToken(ShaderTokenKind k)
        {
            switch (k)
            {
            case ShaderTokenKind::KeywordVertex:  return ShaderStage::kVertex;
            case ShaderTokenKind::KeywordPixel:   return ShaderStage::kPixel;
            case ShaderTokenKind::KeywordCompute: return ShaderStage::kCompute;
            default:                              return ShaderStage::kVertex;
            }
        }

        bool isRenderStateKeyword(ShaderTokenKind k)
        {
            return k == ShaderTokenKind::KeywordBlend
                || k == ShaderTokenKind::KeywordCull
                || k == ShaderTokenKind::KeywordZWrite
                || k == ShaderTokenKind::KeywordZTest;
        }
    }

    ShaderParser::ShaderParser(ShaderLexer &lexer, DiagnosticSink *sink)
        : m_lexer(lexer), m_sink(sink)
    {
        m_current = m_lexer.next();
    }

    ShaderToken ShaderParser::consume()
    {
        ShaderToken prev = m_current;
        m_current = m_lexer.next();
        return prev;
    }

    bool ShaderParser::accept(ShaderTokenKind kind)
    {
        if (m_current.kind == kind)
        {
            consume();
            return true;
        }
        return false;
    }

    bool ShaderParser::expect(ShaderTokenKind kind, ShaderToken &out, TkStringView code, TkStringView what)
    {
        if (m_current.kind == kind)
        {
            out = consume();
            return true;
        }
        const TkString msg = TkString::format(
            "expected %s but got %s '%s'",
            what.length() > 0 ? what.get() : toString(kind),
            toString(m_current.kind),
            m_current.text.isEmpty() ? "" : m_current.text.get());
        error(m_current.range, code, msg);
        return false;
    }

    bool ShaderParser::expect(ShaderTokenKind kind, TkStringView code, TkStringView what)
    {
        ShaderToken discarded;
        return expect(kind, discarded, code, what);
    }

    void ShaderParser::error(const SourceRange &range, TkStringView code, TkStringView message)
    {
        if (m_sink != nullptr)
            m_sink->error(range, code, message);
        m_panic = true;
    }

    void ShaderParser::error(const SourceRange &range, TkStringView code, const TkString &message)
    {
        error(range, code, TkStringView(message));
    }

    void ShaderParser::recoverToBlockEnd()
    {
        // Consume tokens until we leave the current { ... } block. Used after a
        // parse error inside a block so we can resume at the next sibling.
        i32 depth = 1;
        while (m_current.kind != ShaderTokenKind::EndOfFile && depth > 0)
        {
            if (m_current.kind == ShaderTokenKind::LBrace) depth++;
            else if (m_current.kind == ShaderTokenKind::RBrace) depth--;
            if (depth == 0)
            {
                consume();
                break;
            }
            consume();
        }
        m_panic = false;
    }

    void ShaderParser::recoverToTopLevel()
    {
        while (m_current.kind != ShaderTokenKind::EndOfFile
            && m_current.kind != ShaderTokenKind::KeywordShader)
        {
            consume();
        }
        m_panic = false;
    }

    bool ShaderParser::parse(ShaderDecl &outShader)
    {
        if (m_current.kind != ShaderTokenKind::KeywordShader)
        {
            error(m_current.range, TkStringView("TKSHD1001"),
                  TkStringView("expected 'Shader' at the top of the manifest"));
            return false;
        }
        return parseShader(outShader);
    }

    bool ShaderParser::parseShader(ShaderDecl &out)
    {
        const ShaderToken shaderKw = consume();
        out.range = shaderKw.range;

        ShaderToken nameTok;
        if (!expect(ShaderTokenKind::StringLiteral, nameTok,
                    TkStringView("TKSHD1003"),
                    TkStringView("shader name (string literal)")))
        {
            return false;
        }
        out.name = nameTok.text;

        if (!expect(ShaderTokenKind::LBrace, TkStringView("TKSHD1003"), TkStringView("'{'")))
            return false;

        while (m_current.kind != ShaderTokenKind::RBrace
            && m_current.kind != ShaderTokenKind::EndOfFile)
        {
            const ShaderToken sectionTok = m_current;
            switch (sectionTok.kind)
            {
            case ShaderTokenKind::KeywordProperties:
                if (!parseProperties(out)) recoverToBlockEnd();
                break;
            case ShaderTokenKind::KeywordTags:
                consume();
                if (!parseTags(out.tags)) recoverToBlockEnd();
                break;
            case ShaderTokenKind::KeywordVariants:
                if (!parseVariants(out)) recoverToBlockEnd();
                break;
            case ShaderTokenKind::KeywordSubShader:
            {
                SubShaderDecl sub;
                if (parseSubShader(sub))
                    out.subShaders.add(sub);
                else
                    recoverToBlockEnd();
                break;
            }
            default:
                error(sectionTok.range, TkStringView("TKSHD1002"),
                      TkString::format("unknown keyword '%s' inside Shader block",
                                       sectionTok.text.isEmpty() ? toString(sectionTok.kind) : sectionTok.text.get()));
                recoverToBlockEnd();
                return false;
            }
        }

        if (!expect(ShaderTokenKind::RBrace, TkStringView("TKSHD1003"), TkStringView("'}'")))
            return false;
        return true;
    }

    bool ShaderParser::parseProperties(ShaderDecl &out)
    {
        const ShaderToken kw = consume(); // 'Properties'
        if (!expect(ShaderTokenKind::LBrace, TkStringView("TKSHD1003"), TkStringView("'{'")))
            return false;

        while (m_current.kind != ShaderTokenKind::RBrace
            && m_current.kind != ShaderTokenKind::EndOfFile)
        {
            PropertyDecl prop;
            prop.range.begin = m_current.range.begin;

            ShaderToken typeTok;
            if (!expect(ShaderTokenKind::Identifier, typeTok,
                        TkStringView("TKSHD1004"),
                        TkStringView("property type")))
            {
                return false;
            }
            prop.type = typeTok.text;

            ShaderToken nameTok;
            if (!expect(ShaderTokenKind::Identifier, nameTok,
                        TkStringView("TKSHD1004"),
                        TkStringView("property name")))
            {
                return false;
            }
            prop.name = nameTok.text;

            if (!expect(ShaderTokenKind::Equal, TkStringView("TKSHD1004"), TkStringView("'='")))
                return false;

            // Capture the default expression as a single token's text for now.
            // Tuple defaults like "(1, 1, 1, 1)" are accepted but stored as raw
            // textual concatenation -- value parsing is left to Phase 5.
            if (m_current.kind == ShaderTokenKind::LParen)
            {
                TkString buffer("(");
                consume();
                while (m_current.kind != ShaderTokenKind::RParen
                    && m_current.kind != ShaderTokenKind::EndOfFile)
                {
                    if (!buffer.isEmpty() && buffer[buffer.length() - 1] != '(')
                        buffer += TkString(", ");
                    if (m_current.kind == ShaderTokenKind::NumberLiteral
                        || m_current.kind == ShaderTokenKind::Identifier
                        || m_current.kind == ShaderTokenKind::StringLiteral)
                    {
                        buffer += m_current.text;
                    }
                    consume();
                    if (m_current.kind == ShaderTokenKind::Comma)
                        consume();
                }
                buffer += TkString(")");
                if (!expect(ShaderTokenKind::RParen, TkStringView("TKSHD1004"), TkStringView("')'")))
                    return false;
                prop.defaultValue = buffer;
            }
            else if (m_current.kind == ShaderTokenKind::NumberLiteral
                  || m_current.kind == ShaderTokenKind::StringLiteral
                  || m_current.kind == ShaderTokenKind::Identifier)
            {
                prop.defaultValue = m_current.text;
                consume();
            }
            else
            {
                error(m_current.range, TkStringView("TKSHD1004"),
                      TkStringView("expected property default value"));
                return false;
            }

            prop.range.end = m_current.range.begin;
            out.properties.add(prop);
        }

        return expect(ShaderTokenKind::RBrace, TkStringView("TKSHD1003"), TkStringView("'}'"));
    }

    bool ShaderParser::parseTags(TkArray<TagDecl> &outTags)
    {
        if (!expect(ShaderTokenKind::LBrace, TkStringView("TKSHD1003"), TkStringView("'{'")))
            return false;

        while (m_current.kind != ShaderTokenKind::RBrace
            && m_current.kind != ShaderTokenKind::EndOfFile)
        {
            TagDecl tag;
            tag.range.begin = m_current.range.begin;

            ShaderToken keyTok;
            if (!expect(ShaderTokenKind::Identifier, keyTok,
                        TkStringView("TKSHD1003"),
                        TkStringView("tag key")))
            {
                return false;
            }
            tag.key = keyTok.text;

            if (!expect(ShaderTokenKind::Equal, TkStringView("TKSHD1003"), TkStringView("'='")))
                return false;

            ShaderToken valueTok;
            if (!expect(ShaderTokenKind::StringLiteral, valueTok,
                        TkStringView("TKSHD1003"),
                        TkStringView("tag value (string)")))
            {
                return false;
            }
            tag.value = valueTok.text;
            tag.range.end = valueTok.range.end;
            outTags.add(tag);
        }

        return expect(ShaderTokenKind::RBrace, TkStringView("TKSHD1003"), TkStringView("'}'"));
    }

    bool ShaderParser::parseVariants(ShaderDecl &out)
    {
        consume(); // 'Variants'
        if (!expect(ShaderTokenKind::LBrace, TkStringView("TKSHD1003"), TkStringView("'{'")))
            return false;

        while (m_current.kind != ShaderTokenKind::RBrace
            && m_current.kind != ShaderTokenKind::EndOfFile)
        {
            VariantDecl variant;
            variant.range.begin = m_current.range.begin;

            ShaderToken nameTok;
            if (!expect(ShaderTokenKind::Identifier, nameTok,
                        TkStringView("TKSHD1003"),
                        TkStringView("variant name")))
            {
                return false;
            }
            variant.name = nameTok.text;

            if (!expect(ShaderTokenKind::Equal, TkStringView("TKSHD1003"), TkStringView("'='")))
                return false;
            if (!expect(ShaderTokenKind::LBracket, TkStringView("TKSHD1003"), TkStringView("'['")))
                return false;

            while (m_current.kind != ShaderTokenKind::RBracket
                && m_current.kind != ShaderTokenKind::EndOfFile)
            {
                if (m_current.kind == ShaderTokenKind::NumberLiteral
                    || m_current.kind == ShaderTokenKind::StringLiteral
                    || m_current.kind == ShaderTokenKind::Identifier)
                {
                    variant.values.add(m_current.text);
                }
                consume();
                if (m_current.kind == ShaderTokenKind::Comma)
                    consume();
            }
            if (!expect(ShaderTokenKind::RBracket, TkStringView("TKSHD1003"), TkStringView("']'")))
                return false;

            variant.range.end = m_current.range.begin;
            out.variants.add(variant);
        }

        return expect(ShaderTokenKind::RBrace, TkStringView("TKSHD1003"), TkStringView("'}'"));
    }

    bool ShaderParser::parseSubShader(SubShaderDecl &out)
    {
        const ShaderToken kw = consume();
        out.range = kw.range;

        if (!expect(ShaderTokenKind::LBrace, TkStringView("TKSHD1003"), TkStringView("'{'")))
            return false;

        while (m_current.kind != ShaderTokenKind::RBrace
            && m_current.kind != ShaderTokenKind::EndOfFile)
        {
            switch (m_current.kind)
            {
            case ShaderTokenKind::KeywordTarget:
                consume();
                if (!parseTarget(out.target))
                    return false;
                out.hasTarget = true;
                break;
            case ShaderTokenKind::KeywordTags:
                consume();
                if (!parseTags(out.tags))
                    return false;
                break;
            case ShaderTokenKind::KeywordPass:
            {
                PassDecl pass;
                if (parsePass(pass))
                    out.passes.add(pass);
                else
                    return false;
                break;
            }
            default:
                error(m_current.range, TkStringView("TKSHD1002"),
                      TkString::format("unknown keyword inside SubShader: %s",
                                       m_current.text.isEmpty() ? toString(m_current.kind) : m_current.text.get()));
                return false;
            }
        }

        return expect(ShaderTokenKind::RBrace, TkStringView("TKSHD1003"), TkStringView("'}'"));
    }

    bool ShaderParser::parseTarget(TargetDecl &out)
    {
        if (!expect(ShaderTokenKind::LBrace, TkStringView("TKSHD1003"), TkStringView("'{'")))
            return false;

        while (m_current.kind != ShaderTokenKind::RBrace
            && m_current.kind != ShaderTokenKind::EndOfFile)
        {
            TargetEntryDecl entry;
            entry.range.begin = m_current.range.begin;

            ShaderToken backendTok;
            if (!expect(ShaderTokenKind::Identifier, backendTok,
                        TkStringView("TKSHD1006"),
                        TkStringView("backend identifier")))
            {
                return false;
            }
            entry.backend = backendTok.text;

            if (!expect(ShaderTokenKind::Equal, TkStringView("TKSHD1006"), TkStringView("'='")))
                return false;

            ShaderToken profileTok;
            if (!expect(ShaderTokenKind::StringLiteral, profileTok,
                        TkStringView("TKSHD1006"),
                        TkStringView("profile string")))
            {
                return false;
            }
            entry.profile = profileTok.text;
            entry.range.end = profileTok.range.end;
            out.entries.add(entry);
        }

        return expect(ShaderTokenKind::RBrace, TkStringView("TKSHD1003"), TkStringView("'}'"));
    }

    bool ShaderParser::parsePass(PassDecl &out)
    {
        const ShaderToken kw = consume();
        out.range = kw.range;

        ShaderToken nameTok;
        if (!expect(ShaderTokenKind::StringLiteral, nameTok,
                    TkStringView("TKSHD1005"),
                    TkStringView("pass name (string)")))
        {
            return false;
        }
        out.name = nameTok.text;

        if (!expect(ShaderTokenKind::LBrace, TkStringView("TKSHD1003"), TkStringView("'{'")))
            return false;

        while (m_current.kind != ShaderTokenKind::RBrace
            && m_current.kind != ShaderTokenKind::EndOfFile)
        {
            const ShaderTokenKind k = m_current.kind;

            if (k == ShaderTokenKind::KeywordTags)
            {
                consume();
                if (!parseTags(out.tags))
                    return false;
                continue;
            }

            if (isStageKeyword(k))
            {
                const ShaderStage stage = stageFromToken(k);
                consume();
                if (!parseStage(stage, out))
                    return false;
                continue;
            }

            if (isRenderStateKeyword(k))
            {
                if (!parseRenderState(out))
                    return false;
                continue;
            }

            if (k == ShaderTokenKind::KeywordHLSLProgram
                || k == ShaderTokenKind::KeywordGLSLProgram
                || k == ShaderTokenKind::KeywordMSLProgram)
            {
                const ShaderToken marker = consume();
                // Lexer immediately follows up with an EmbeddedSourceBlock token.
                if (m_current.kind != ShaderTokenKind::EmbeddedSourceBlock)
                {
                    error(m_current.range, TkStringView("TKSHD1005"),
                          TkStringView("expected embedded source block after PROGRAM marker"));
                    return false;
                }
                EmbeddedSourceDecl emb;
                emb.range = SourceRange{marker.range.begin, m_current.range.end};
                emb.language = m_current.language;
                emb.sourceText = m_current.text;
                out.embeddedSources.add(emb);
                consume();
                continue;
            }

            error(m_current.range, TkStringView("TKSHD1005"),
                  TkString::format("unexpected token inside Pass: '%s'",
                                   m_current.text.isEmpty() ? toString(m_current.kind) : m_current.text.get()));
            return false;
        }

        return expect(ShaderTokenKind::RBrace, TkStringView("TKSHD1003"), TkStringView("'}'"));
    }

    bool ShaderParser::parseStage(ShaderStage stage, PassDecl &outPass)
    {
        StageDecl decl;
        decl.stage = stage;
        decl.range.begin = m_current.range.begin;

        if (!expect(ShaderTokenKind::LBrace, TkStringView("TKSHD1003"), TkStringView("'{'")))
            return false;

        while (m_current.kind != ShaderTokenKind::RBrace
            && m_current.kind != ShaderTokenKind::EndOfFile)
        {
            ShaderToken keyTok;
            if (!expect(ShaderTokenKind::Identifier, keyTok,
                        TkStringView("TKSHD1005"),
                        TkStringView("'Source' or 'Entry'")))
            {
                return false;
            }

            if (!expect(ShaderTokenKind::Equal, TkStringView("TKSHD1005"), TkStringView("'='")))
                return false;

            ShaderToken valueTok;
            if (!expect(ShaderTokenKind::StringLiteral, valueTok,
                        TkStringView("TKSHD1005"),
                        TkStringView("string value")))
            {
                return false;
            }

            const TkString key = keyTok.text;
            if (key == "Source")
                decl.source = valueTok.text;
            else if (key == "Entry")
                decl.entry = valueTok.text;
            else
            {
                error(keyTok.range, TkStringView("TKSHD1005"),
                      TkString::format("unknown stage option '%s'", key.get()));
                return false;
            }

            // Optional ';' terminator.
            accept(ShaderTokenKind::Semicolon);
        }

        if (!expect(ShaderTokenKind::RBrace, TkStringView("TKSHD1003"), TkStringView("'}'")))
            return false;

        decl.range.end = m_current.range.begin;
        outPass.stages.add(decl);
        return true;
    }

    bool ShaderParser::parseRenderState(PassDecl &outPass)
    {
        RenderStateDecl rs;
        rs.range.begin = m_current.range.begin;
        rs.name = m_current.text;
        consume();

        // Value is a single identifier ("On"/"Off"/"Back"/...) or a string.
        if (m_current.kind == ShaderTokenKind::Identifier
            || m_current.kind == ShaderTokenKind::StringLiteral
            || m_current.kind == ShaderTokenKind::NumberLiteral)
        {
            rs.value = m_current.text;
            rs.range.end = m_current.range.end;
            consume();
        }
        else
        {
            error(m_current.range, TkStringView("TKSHD1007"),
                  TkString::format("expected value after render state '%s'", rs.name.get()));
            return false;
        }

        accept(ShaderTokenKind::Semicolon);
        outPass.renderStates.add(rs);
        return true;
    }
}
