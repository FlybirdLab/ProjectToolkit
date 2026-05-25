/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "MarcoPicker.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/FileSystem.hpp"

namespace toolkit {

    MarcoPicker::MarcoPicker()
        : m_ctx(nullptr)
    {
    }
    MarcoPicker::MarcoPicker(const TkString& v, ParserContext* ctx)
        : m_path{ v }
        , m_ctx{ ctx }
    {
    }
    MarcoPicker::~MarcoPicker() = default;


    bool MarcoPicker::tryParse()
    {
        if (!TkFileSystem::isFileExist(m_path)) {
            LogError("File %s not exist", m_path.getText());
            return false;
        }

        OpenFileParams p{.path = m_path};
        auto* f = TkFileSystem::openFile(p);
        if (!TkFileSystem::readAsText(f, m_data)) {
            LogError("File %s read failed", m_path.getText());
            return false;
        }

        i32 idx = 0;
        const size_t max = m_data.length();

        auto skipWhitespace = [](const TkStringView& data, i32& i) {
            while (i < data.length() && TkStringUtils::isWhitespace(data[i])) ++i;
        };

        while (idx < max) {
            skipWhitespace(m_data, idx);
            if (idx >= max)
                break;

            if (m_data[idx] == '#') {
                ++idx;
                skipWhitespace(m_data, idx);

                constexpr const char* kDefine = "define";
                constexpr const char* kUndef = "undef";

                i32 kwStart = idx;
                while (idx < max && TkStringUtils::isAlpha(m_data[idx]))
                    ++idx;

                TkStringView keyword(&m_data[kwStart], idx - kwStart);
                skipWhitespace(m_data, idx);

                const i32 nameStart = idx;
                while (idx < max && !TkStringUtils::isWhitespace(m_data[idx]) && m_data[idx] != '(')
                    ++idx;

                TkString macroName(&m_data[nameStart], idx - nameStart);

                if (keyword == kDefine) {
                    m_marcos.add(macroName);
                }
                else if (keyword == kUndef) {
                    m_marcos.remove(macroName);
                }
            }
            else {
                // skip invalid statement
                while (idx < max && m_data[idx] != '\n')
                    ++idx;
            }
        }

        m_data.clear();
        return true;
    }


    bool MarcoPicker::hasMarco(const TkStringView& v) const {
        return m_marcos.find(v) != -1;
    }


    void MarcoPicker::debugPrint() {
#if CB_DEBUG
        for (auto& m : m_marcos) {
            LogDebug("[MARCO: %s]", m.getText());
        }
#endif
    }

} // namespace cobalt
