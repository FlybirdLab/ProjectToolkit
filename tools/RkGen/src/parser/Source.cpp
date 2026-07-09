#include "Source.hpp"

#include "toolkit/base/Assert.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "parser/Lexer.hpp"
#include "parser/Parser.hpp"
#include <parser/ASTNodes.hpp>

namespace toolkit
{

    CXXSourceFile::CXXSourceFile(const TkStringView &path, ParserContext *ctx)
        : m_path{path}, m_lexer{nullptr}, m_parser{nullptr}, m_rootNode{nullptr}, m_ctx{ctx}, m_isProcessed{false}
    {
        TK_ASSERT(!path.isEmpty());
    }

    CXXSourceFile::~CXXSourceFile()
    {
        if (m_lexer)
        {
            TkMemory::deletePtr(m_lexer);
            m_lexer = nullptr;
        }
        if (m_parser)
        {
            TkMemory::deletePtr(m_parser);
            m_parser = nullptr;
        }
        if (m_rootNode)
        {
            TkMemory::deletePtr(m_rootNode);
            m_rootNode = nullptr;
        }

        m_includes.clear();
    }

    bool CXXSourceFile::tryRead()
    {
        OpenFileParams p{.path = m_path};
        auto *file = TkFileSystem::openFile(p);
        if (!TkFileSystem::readAsText(file, m_src))
        {
            LogError("Failed to read source file -> %s", m_path.getText());
            return false;
        }
        return true;
    }

    bool CXXSourceFile::processFile()
    {
        // avoid process again
        if (m_isProcessed)
        {
            return m_isProcessed;
        }

        // clean up first
        if (m_lexer)
            TkMemory::deletePtr(m_lexer);
        if (m_parser)
            TkMemory::deletePtr(m_parser);
        if (m_rootNode)
            TkMemory::deletePtr(m_rootNode);

        // tokenize process
        m_lexer = TkMemory::newObject<CXXLexer>(TkMemory::getDefault(), m_src);
        if (!m_lexer->lex())
        {
            LogError("Failed to tokenize file -> %s", m_path.getText());
            return false;
        }

        // for (auto &ptr : m_lexer->getTokens())
        // {
        //     LogDebug("- %s", ptr.toString().getText());
        // }

        // parse process
        m_parser = TkMemory::newObject<CXXParser>(TkMemory::getDefault(), m_lexer, this);
        if (!m_parser->parse())
        {
            LogError("Failed to parse file -> %s", m_path.getText());
            return false;
        }

        auto *root = m_parser->takeRoot();
        if (!root)
        {
            LogError("Error result received from parser");
            return false;
        }

        m_rootNode = root;
        m_isProcessed = true;
        return true;
    }

} // namespace cobalt
