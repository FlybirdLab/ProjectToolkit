/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include "ITokenReader.hpp"

namespace toolkit {
    
    // Interface for objects that can have child token readers
    template<typename Type>
    class ITokenReadersContainerBase
    {
    protected:
        TkArray<Type*> m_childReaders;

    public:
        virtual ~ITokenReadersContainerBase()
        {
            // Cleanup
            m_childReaders.clearDelete();
        }

    protected:
        // Try to process given token by any child reader
        virtual bool processChildren(const Token& token, IShaderParser* parser)
        {
            for (i32 i = 0; i < m_childReaders.count(); i++)
            {
                if (m_childReaders[i]->checkStartToken(token))
                {
                    m_childReaders[i]->process(parser, parser->getReader());
                    return false;
                }
            }

            return true;
        }
    };

    // Interface for objects that can have child ITokenReader objects
    class ITokenReadersContainer : public ITokenReadersContainerBase<ITokenReader>
    {
    public:
        virtual ~ITokenReadersContainer()
        {
            // Cleanup
            m_childReaders.clearDelete();
        }
    };
}