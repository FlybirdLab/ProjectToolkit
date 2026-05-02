/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "CommandLine.hpp"

namespace toolkit
{
    TkCommandLine::TkCommandLine(int argc, char **argv)
    {
        parse(argc, argv);
    }

    void TkCommandLine::parse(int argc, char **argv)
    {
        m_options.clear();
        m_positionals.clear();

        for (int i = 1; i < argc; i++)
        {
            TkString arg(argv[i]);
            if (!isOption(arg))
            {
                m_positionals.add(arg);
                continue;
            }

            TkString name;
            TkString value;
            bool hasValue = false;
            const i32 eq = arg.find('=');
            if (eq >= 0)
            {
                name = normalizeName(arg.substring(0, eq));
                value = arg.substring(eq + 1);
                hasValue = true;
            }
            else
            {
                name = normalizeName(arg);
                if (i + 1 < argc && !isOption(TkStringView(argv[i + 1])))
                {
                    value = argv[++i];
                    hasValue = true;
                }
            }

            addOption(name, value, hasValue);
        }
    }

    bool TkCommandLine::has(TkStringView name) const
    {
        const TkString normalized = normalizeName(name);
        for (const auto &option : m_options)
        {
            if (option.name == normalized)
                return true;
        }
        return false;
    }

    bool TkCommandLine::get(TkStringView name, TkString &value) const
    {
        const TkString normalized = normalizeName(name);
        for (i32 i = m_options.count() - 1; i >= 0; i--)
        {
            const auto &option = m_options[i];
            if (option.name == normalized && option.hasValue)
            {
                value = option.value;
                return true;
            }
        }
        return false;
    }

    TkString TkCommandLine::get(TkStringView name, TkStringView defaultValue) const
    {
        TkString result;
        if (get(name, result))
            return result;
        return TkString(defaultValue);
    }

    const TkArray<TkString> &TkCommandLine::getPositionals() const
    {
        return m_positionals;
    }

    bool TkCommandLine::isOption(TkStringView arg)
    {
        return arg.length() > 1 && arg[0] == '-';
    }

    TkString TkCommandLine::normalizeName(TkStringView arg)
    {
        i32 start = 0;
        while (start < arg.length() && arg[start] == '-')
            start++;
        if (start >= arg.length())
            return TkString();
        TkString result = arg.substring(start);
        return result.toLowerAscii();
    }

    void TkCommandLine::addOption(TkStringView name, TkStringView value, bool hasValue)
    {
        Option &option = m_options.addOne();
        option.name = normalizeName(name);
        option.value = value;
        option.hasValue = hasValue;
    }
}
