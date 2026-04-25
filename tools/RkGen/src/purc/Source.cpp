#include "Source.hpp"

#include "toolkit/base/Assert.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "purc/Lexer.hpp"
#include "purc/Parser.hpp"
#include <purc/ASTNodes.hpp>

namespace toolkit
{

    CXXSourceFile::CXXSourceFile(const TkStringView &path, PurcCtx *ctx)
        : m_path{path}, m_lexer{nullptr}, m_parser{nullptr}, m_rootNode{nullptr}, m_ctx{ctx}
    {
        TK_ASSERT(!path.isEmpty());
    }

    CXXSourceFile::~CXXSourceFile()
    {
        if (m_lexer)
        {
            TkMemory::deletePtr(m_lexer);
        }
        if (m_parser)
        {
            TkMemory::deletePtr(m_parser);
        }
        if (m_rootNode)
        {
            TkMemory::deletePtr(m_rootNode);
        }

        m_includes.clear();
    }

    bool CXXSourceFile::tryRead()
    {
        OpenFileParams p{.path = m_path};
        auto *file = TkFileSystem::openFile(p);
        if (!TkFileSystem::readAsText(file, m_src))
        {
            LogError("Failed to read source file -> {0}", m_path);
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
        {
            TkMemory::deletePtr(m_lexer);
        }
        if (m_parser)
        {
            TkMemory::deletePtr(m_parser);
        }
        if (m_rootNode)
        {
            TkMemory::deletePtr(m_rootNode);
        }

        // tokenize process
        m_lexer = TkMemory::newObject<CXXLexer>(TkMemory::getDefault(), m_src);
        if (!m_lexer->lex())
        {
            LogError("Failed to tokenize file -> {0}", m_path);
            return false;
        }

        for (auto &ptr : m_lexer->getTokens())
        {
            LogDebug("- {0}", ptr.toString());
        }

        // parse process
        m_parser = TkMemory::newObject<CXXParser>(TkMemory::getDefault(), m_lexer, this);
        if (!m_parser->parse())
        {
            LogError("Failed to parse file -> {0}", m_path);
            return false;
        }

        auto *root = m_parser->getRoot();
        if (!root)
        {
            LogError("Error result received from parser");
            return false;
        }

        m_isProcessed = true;
        return true;
    }

} // namespace cobalt