#pragma once

#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit {

    // Light weight method to get marco defination informations
    class MarcoPicker
    {
        friend class CXXParser;

    public:
        MarcoPicker();
        MarcoPicker(const TkString&, class ParserContext*);
        ~MarcoPicker();

        bool tryParse();

        bool hasMarco(const TkStringView&) const;
        void debugPrint();

    private:
        TkString m_data;
        ParserContext* m_ctx;
        TkArray<TkString> m_marcos;
        TkString m_path;
    };

}
