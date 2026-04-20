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
        MarcoPicker(const TkStringView&, class PurcCtx*);
        ~MarcoPicker();

        bool tryParse();

        bool hasMarco(const TkStringView&) const;
        void debugPrint();

    private:
        TkString m_data;
        PurcCtx* m_ctx;
        TkArray<TkString> m_marcos;
        TkStringView m_path;
    };

}