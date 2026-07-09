/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/lexer/ShaderLexer.hpp>
#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>

namespace toolkit
{
    const char *toString(ShaderTokenKind kind)
    {
        switch (kind)
        {
        case ShaderTokenKind::EndOfFile:           return "<eof>";
        case ShaderTokenKind::Unknown:             return "<unknown>";
        case ShaderTokenKind::Identifier:          return "identifier";
        case ShaderTokenKind::StringLiteral:       return "string";
        case ShaderTokenKind::NumberLiteral:       return "number";
        case ShaderTokenKind::LBrace:              return "'{'";
        case ShaderTokenKind::RBrace:              return "'}'";
        case ShaderTokenKind::LParen:              return "'('";
        case ShaderTokenKind::RParen:              return "')'";
        case ShaderTokenKind::LBracket:            return "'['";
        case ShaderTokenKind::RBracket:            return "']'";
        case ShaderTokenKind::Equal:               return "'='";
        case ShaderTokenKind::Comma:               return "','";
        case ShaderTokenKind::Dot:                 return "'.'";
        case ShaderTokenKind::Colon:               return "':'";
        case ShaderTokenKind::Semicolon:           return "';'";
        case ShaderTokenKind::KeywordShader:       return "'Shader'";
        case ShaderTokenKind::KeywordProperties:   return "'Properties'";
        case ShaderTokenKind::KeywordTags:         return "'Tags'";
        case ShaderTokenKind::KeywordVariants:     return "'Variants'";
        case ShaderTokenKind::KeywordSubShader:    return "'SubShader'";
        case ShaderTokenKind::KeywordPass:         return "'Pass'";
        case ShaderTokenKind::KeywordTarget:       return "'Target'";
        case ShaderTokenKind::KeywordVertex:       return "'Vertex'";
        case ShaderTokenKind::KeywordPixel:        return "'Pixel'";
        case ShaderTokenKind::KeywordCompute:      return "'Compute'";
        case ShaderTokenKind::KeywordBlend:        return "'Blend'";
        case ShaderTokenKind::KeywordCull:         return "'Cull'";
        case ShaderTokenKind::KeywordZWrite:       return "'ZWrite'";
        case ShaderTokenKind::KeywordZTest:        return "'ZTest'";
        case ShaderTokenKind::KeywordHLSLProgram:  return "'HLSLPROGRAM'";
        case ShaderTokenKind::KeywordGLSLProgram:  return "'GLSLPROGRAM'";
        case ShaderTokenKind::KeywordMSLProgram:   return "'MSLPROGRAM'";
        case ShaderTokenKind::EmbeddedSourceBlock: return "<embedded source>";
        }
        return "<token>";
    }

    const char *toString(ShaderSourceLanguage lang)
    {
        switch (lang)
        {
        case ShaderSourceLanguage::HLSL: return "hlsl";
        case ShaderSourceLanguage::GLSL: return "glsl";
        case ShaderSourceLanguage::MSL:  return "msl";
        case ShaderSourceLanguage::Unknown: return "unknown";
        }
        return "unknown";
    }

    namespace
    {
        bool isAlpha(char c)        { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'; }
        bool isDigit(char c)        { return c >= '0' && c <= '9'; }
        bool isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }

        struct KeywordEntry
        {
            const char *text;
            ShaderTokenKind kind;
        };

        // Linear search is fine for the small keyword set; trigger order matters
        // only for case-sensitive matching, which we already require below.
        constexpr KeywordEntry kKeywords[] = {
            {"Shader",      ShaderTokenKind::KeywordShader},
            {"Properties",  ShaderTokenKind::KeywordProperties},
            {"Tags",        ShaderTokenKind::KeywordTags},
            {"Variants",    ShaderTokenKind::KeywordVariants},
            {"SubShader",   ShaderTokenKind::KeywordSubShader},
            {"Pass",        ShaderTokenKind::KeywordPass},
            {"Target",      ShaderTokenKind::KeywordTarget},
            {"Vertex",      ShaderTokenKind::KeywordVertex},
            {"Pixel",       ShaderTokenKind::KeywordPixel},
            {"Compute",     ShaderTokenKind::KeywordCompute},
            {"Blend",       ShaderTokenKind::KeywordBlend},
            {"Cull",        ShaderTokenKind::KeywordCull},
            {"ZWrite",      ShaderTokenKind::KeywordZWrite},
            {"ZTest",       ShaderTokenKind::KeywordZTest},
            {"HLSLPROGRAM", ShaderTokenKind::KeywordHLSLProgram},
            {"GLSLPROGRAM", ShaderTokenKind::KeywordGLSLProgram},
            {"MSLPROGRAM",  ShaderTokenKind::KeywordMSLProgram},
        };

        ShaderTokenKind classifyKeyword(TkStringView text)
        {
            for (const auto &entry : kKeywords)
            {
                // Length-aware comparison so a partial match (e.g. "Pass" vs
                // "Passive") doesn't incorrectly classify as a keyword.
                const char *kw = entry.text;
                i32 i = 0;
                while (kw[i] != 0 && i < text.length() && kw[i] == text[i])
                    i++;
                if (kw[i] == 0 && i == text.length())
                    return entry.kind;
            }
            return ShaderTokenKind::Identifier;
        }

        bool startsWithKeyword(const TkStringView &src, u32 offset, const char *kw)
        {
            const i32 len = static_cast<i32>(src.length());
            i32 i = 0;
            while (kw[i] != 0)
            {
                const i32 pos = static_cast<i32>(offset) + i;
                if (pos >= len || src[pos] != kw[i])
                    return false;
                i++;
            }
            // End must be followed by a non-identifier char (or eof) so we
            // don't accidentally match a prefix of a longer identifier.
            const i32 end = static_cast<i32>(offset) + i;
            if (end < len && isAlphaNumeric(src[end]))
                return false;
            return true;
        }
    }

    ShaderLexer::ShaderLexer(const SourceFile *file, DiagnosticSink *sink)
        : m_file(file), m_sink(sink), m_content(file->getContent())
    {
    }

    SourceLocation ShaderLexer::locationAt(u32 offset) const
    {
        return m_file->getLocation(offset);
    }

    SourceRange ShaderLexer::rangeBetween(u32 begin, u32 end) const
    {
        return SourceRange{locationAt(begin), locationAt(end)};
    }

    char ShaderLexer::peek(i32 ahead) const
    {
        const i32 pos = static_cast<i32>(m_offset) + ahead;
        if (pos < 0 || pos >= m_content.length())
            return '\0';
        return m_content[pos];
    }

    void ShaderLexer::reportLexerError(const SourceRange &range, TkStringView code, TkStringView message)
    {
        if (m_sink != nullptr)
            m_sink->error(range, code, message);
    }

    void ShaderLexer::skipWhitespaceAndComments()
    {
        while (m_offset < static_cast<u32>(m_content.length()))
        {
            const char c = m_content[static_cast<i32>(m_offset)];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                m_offset++;
                continue;
            }
            if (c == '/' && peek(1) == '/')
            {
                m_offset += 2;
                while (m_offset < static_cast<u32>(m_content.length()) && m_content[static_cast<i32>(m_offset)] != '\n')
                    m_offset++;
                continue;
            }
            if (c == '/' && peek(1) == '*')
            {
                const u32 commentStart = m_offset;
                m_offset += 2;
                bool closed = false;
                while (m_offset + 1 < static_cast<u32>(m_content.length()))
                {
                    if (m_content[static_cast<i32>(m_offset)] == '*' && m_content[static_cast<i32>(m_offset) + 1] == '/')
                    {
                        m_offset += 2;
                        closed = true;
                        break;
                    }
                    m_offset++;
                }
                if (!closed)
                {
                    reportLexerError(
                        rangeBetween(commentStart, static_cast<u32>(m_content.length())),
                        TkStringView("TKSHD1000"),
                        TkStringView("unterminated block comment"));
                    m_offset = static_cast<u32>(m_content.length());
                }
                continue;
            }
            break;
        }
    }

    ShaderToken ShaderLexer::lexIdentifierOrKeyword()
    {
        const u32 start = m_offset;
        while (m_offset < static_cast<u32>(m_content.length()) && isAlphaNumeric(m_content[static_cast<i32>(m_offset)]))
            m_offset++;

        const i32 length = static_cast<i32>(m_offset - start);
        TkStringView slice(m_content.get() + start, length);
        const ShaderTokenKind kind = classifyKeyword(slice);

        ShaderToken token;
        token.kind = kind;
        token.range = rangeBetween(start, m_offset);
        token.text = TkString(slice);
        return token;
    }

    ShaderToken ShaderLexer::lexNumber()
    {
        const u32 start = m_offset;
        if (m_content[static_cast<i32>(m_offset)] == '-' || m_content[static_cast<i32>(m_offset)] == '+')
            m_offset++;
        while (m_offset < static_cast<u32>(m_content.length()))
        {
            const char c = m_content[static_cast<i32>(m_offset)];
            if (isDigit(c) || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-' || c == 'f')
                m_offset++;
            else
                break;
        }
        const i32 length = static_cast<i32>(m_offset - start);
        ShaderToken token;
        token.kind = ShaderTokenKind::NumberLiteral;
        token.range = rangeBetween(start, m_offset);
        token.text = TkString(m_content.get() + start, length);
        return token;
    }

    ShaderToken ShaderLexer::lexString()
    {
        const u32 start = m_offset;
        m_offset++; // consume opening quote

        TkString value;
        bool terminated = false;
        while (m_offset < static_cast<u32>(m_content.length()))
        {
            const char c = m_content[static_cast<i32>(m_offset)];
            if (c == '"')
            {
                m_offset++;
                terminated = true;
                break;
            }
            if (c == '\\' && m_offset + 1 < static_cast<u32>(m_content.length()))
            {
                const char esc = m_content[static_cast<i32>(m_offset) + 1];
                m_offset += 2;
                switch (esc)
                {
                case 'n':  value += TkString("\n", 1); break;
                case 't':  value += TkString("\t", 1); break;
                case '\\': value += TkString("\\", 1); break;
                case '"':  value += TkString("\"", 1); break;
                default:
                {
                    const char tmp[2] = {esc, 0};
                    value += TkString(tmp, 1);
                    break;
                }
                }
                continue;
            }
            value += TkString(&c, 1);
            m_offset++;
        }

        ShaderToken token;
        token.kind = ShaderTokenKind::StringLiteral;
        token.range = rangeBetween(start, m_offset);
        token.text = value;
        if (!terminated)
        {
            reportLexerError(token.range, TkStringView("TKSHD1000"), TkStringView("unterminated string literal"));
        }
        return token;
    }

    ShaderToken ShaderLexer::lexEmbeddedSourceBody(ShaderSourceLanguage lang, u32 startOffset)
    {
        const char *endMarker;
        switch (lang)
        {
        case ShaderSourceLanguage::HLSL: endMarker = "ENDHLSL"; break;
        case ShaderSourceLanguage::GLSL: endMarker = "ENDGLSL"; break;
        case ShaderSourceLanguage::MSL:  endMarker = "ENDMSL";  break;
        default: endMarker = "ENDHLSL"; break;
        }

        const u32 bodyBegin = startOffset;
        u32 bodyEnd = bodyBegin;
        bool found = false;
        while (m_offset < static_cast<u32>(m_content.length()))
        {
            if (startsWithKeyword(m_content, m_offset, endMarker))
            {
                bodyEnd = m_offset;
                // Trim trailing whitespace from the body so EmbeddedSourceBlock
                // ranges don't include the indent that lead into ENDxxx.
                while (bodyEnd > bodyBegin)
                {
                    const char c = m_content[static_cast<i32>(bodyEnd) - 1];
                    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                        bodyEnd--;
                    else
                        break;
                }

                // Advance past the marker.
                u32 markerLen = 0;
                while (endMarker[markerLen] != 0) markerLen++;
                m_offset += markerLen;
                found = true;
                break;
            }
            m_offset++;
        }

        ShaderToken token;
        token.kind = ShaderTokenKind::EmbeddedSourceBlock;
        token.range = rangeBetween(bodyBegin, bodyEnd);
        token.text = TkString(m_content.get() + bodyBegin, static_cast<i32>(bodyEnd - bodyBegin));
        token.language = lang;

        if (!found)
        {
            const TkString message = TkString::format(
                "missing terminator for embedded source block (expected %s)", endMarker);
            reportLexerError(
                rangeBetween(bodyBegin, static_cast<u32>(m_content.length())),
                TkStringView("TKSHD1000"),
                TkStringView(message));
            m_offset = static_cast<u32>(m_content.length());
        }

        m_mode = Mode::Normal;
        m_pendingEmbedded = ShaderSourceLanguage::Unknown;
        return token;
    }

    ShaderToken ShaderLexer::makeSingleCharToken(ShaderTokenKind kind)
    {
        const u32 start = m_offset;
        m_offset++;
        ShaderToken token;
        token.kind = kind;
        token.range = rangeBetween(start, m_offset);
        token.text = TkString(m_content.get() + start, 1);
        return token;
    }

    ShaderToken ShaderLexer::next()
    {
        if (m_mode == Mode::EmbeddedSource)
        {
            return lexEmbeddedSourceBody(m_pendingEmbedded, m_pendingEmbeddedStartOffset);
        }

        skipWhitespaceAndComments();

        if (m_offset >= static_cast<u32>(m_content.length()))
        {
            ShaderToken token;
            token.kind = ShaderTokenKind::EndOfFile;
            token.range = rangeBetween(m_offset, m_offset);
            return token;
        }

        const char c = m_content[static_cast<i32>(m_offset)];

        if (isAlpha(c))
        {
            ShaderToken token = lexIdentifierOrKeyword();
            // Switch to embedded-source mode on *PROGRAM markers.
            if (token.kind == ShaderTokenKind::KeywordHLSLProgram ||
                token.kind == ShaderTokenKind::KeywordGLSLProgram ||
                token.kind == ShaderTokenKind::KeywordMSLProgram)
            {
                m_mode = Mode::EmbeddedSource;
                switch (token.kind)
                {
                case ShaderTokenKind::KeywordHLSLProgram: m_pendingEmbedded = ShaderSourceLanguage::HLSL; break;
                case ShaderTokenKind::KeywordGLSLProgram: m_pendingEmbedded = ShaderSourceLanguage::GLSL; break;
                case ShaderTokenKind::KeywordMSLProgram:  m_pendingEmbedded = ShaderSourceLanguage::MSL;  break;
                default: break;
                }
                // Body starts right after the marker; skip a single trailing
                // newline so the source range begins at the first content line.
                if (m_offset < static_cast<u32>(m_content.length()) && m_content[static_cast<i32>(m_offset)] == '\r')
                    m_offset++;
                if (m_offset < static_cast<u32>(m_content.length()) && m_content[static_cast<i32>(m_offset)] == '\n')
                    m_offset++;
                m_pendingEmbeddedStartOffset = m_offset;
            }
            return token;
        }

        if (isDigit(c) || ((c == '-' || c == '+') && isDigit(peek(1))))
            return lexNumber();

        if (c == '"')
            return lexString();

        switch (c)
        {
        case '{': return makeSingleCharToken(ShaderTokenKind::LBrace);
        case '}': return makeSingleCharToken(ShaderTokenKind::RBrace);
        case '(': return makeSingleCharToken(ShaderTokenKind::LParen);
        case ')': return makeSingleCharToken(ShaderTokenKind::RParen);
        case '[': return makeSingleCharToken(ShaderTokenKind::LBracket);
        case ']': return makeSingleCharToken(ShaderTokenKind::RBracket);
        case '=': return makeSingleCharToken(ShaderTokenKind::Equal);
        case ',': return makeSingleCharToken(ShaderTokenKind::Comma);
        case '.': return makeSingleCharToken(ShaderTokenKind::Dot);
        case ':': return makeSingleCharToken(ShaderTokenKind::Colon);
        case ';': return makeSingleCharToken(ShaderTokenKind::Semicolon);
        default: break;
        }

        // Unknown character: produce an Unknown token and advance so we can
        // recover. The error is reported here, the parser may report another
        // one for context but the source position is already pinned down.
        const u32 start = m_offset;
        m_offset++;
        ShaderToken token;
        token.kind = ShaderTokenKind::Unknown;
        token.range = rangeBetween(start, m_offset);
        token.text = TkString(&c, 1);
        const TkString unexpectedMsg = TkString::format("unexpected character '%c'", c);
        reportLexerError(
            token.range,
            TkStringView("TKSHD1000"),
            TkStringView(unexpectedMsg));
        return token;
    }
}
