/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 * References:
 *      https://github.com/Neargye/nameof
 */

#pragma once

#include <toolkit/types/StringView.hpp>

namespace toolkit {
    
    constexpr TkStringView getTypeName(TkStringView name) noexcept
    {
        if (name.length() >= 1 && (name.get()[0] == '"' || name.get()[0] == '\''))
            return {};
        if (name.length() >= 2 && name.get()[0] == 'R' && (name.get()[1] == '"' || name.get()[1] == '\''))
            return {};
        if (name.length() >= 2 && name.get()[0] == 'L' && (name.get()[1] == '"' || name.get()[1] == '\''))
            return {};
        if (name.length() >= 2 && name.get()[0] == 'U' && (name.get()[1] == '"' || name.get()[1] == '\''))
            return {};
        if (name.length() >= 2 && name.get()[0] == 'u' && (name.get()[1] == '"' || name.get()[1] == '\''))
            return {};
        if (name.length() >= 3 && name.get()[0] == 'u' && name.get()[1] == '8' && (name.get()[2] == '"' || name.get()[2] == '\''))
            return {};
        if (name.length() >= 1 && (name.get()[0] >= '0' && name.get()[0] <= '9'))
            return {};
        for (i32 i = name.length(), h = 0, s = 0; i > 0; --i)
        {
            if (name.get()[i - 1] == ')')
            {
                ++h;
                ++s;
                continue;
            }
            if (name.get()[i - 1] == '(')
            {
                --h;
                ++s;
                continue;
            }
            if (h == 0)
            {
                name = TkStringView(*name, name.length() - s);
                break;
            }
            ++s;
        }
        i32 s = 0;
        for (i32 i = name.length(), h = 0; i > 0; --i)
        {
            if (name.get()[i - 1] == '>')
            {
                ++h;
                ++s;
                continue;
            }
            if (name.get()[i - 1] == '<')
            {
                --h;
                ++s;
                continue;
            }
            if (h == 0)
                break;
            ++s;
        }
        for (i32 i = name.length() - s; i > 0; --i)
        {
            if (!((name.get()[i - 1] >= '0' && name.get()[i - 1] <= '9') || (name.get()[i - 1] >= 'a' && name.get()[i - 1] <= 'z') || (name.get()[i - 1] >= 'A' && name.get()[i - 1] <= 'Z') || (name.get()[i - 1] == '_')))
            {
                name = TkStringView(*name + i, name.length() - i);
                break;
            }
        }
        name = TkStringView(*name, name.length() - s);
        if (name.length() > 0 && ((name.get()[0] >= 'a' && name.get()[0] <= 'z') || (name.get()[0] >= 'A' && name.get()[0] <= 'Z') || (name.get()[0] == '_')))
            return name;
        return {};
    }

    // Gets the type name as constant expression (compile-time string)
    template<typename T>
    constexpr TkStringView get() noexcept
    {
#if defined(__clang__) || defined(__GNUC__)
        return getTypeName(TkStringView(__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2));
#elif defined(_MSC_VER)
        return getTypeName(TkStringView(__FUNCSIG__, sizeof(__FUNCSIG__) - 17));
#else
#error "Unsupported compiler."
#endif
    }
}