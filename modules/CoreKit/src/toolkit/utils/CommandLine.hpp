/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit
{
    class TkCommandLine final
    {
    public:
        struct Option
        {
            TkString name;
            TkString value;
            bool hasValue = false;
        };

    private:
        TkArray<Option> m_options;
        TkArray<TkString> m_positionals;

    public:
        TkCommandLine() = default;
        TkCommandLine(int argc, char **argv);

        void parse(int argc, char **argv);
        bool has(TkStringView name) const;
        bool get(TkStringView name, TkString &value) const;
        TkString get(TkStringView name, TkStringView defaultValue = TkStringView()) const;
        // Collect every value supplied for `name` in the order they appear on
        // the command line. Used for repeatable options such as `-I` / `--include-dir`.
        void getAll(TkStringView name, TkArray<TkString> &out) const;
        const TkArray<TkString> &getPositionals() const;

    private:
        static bool isOption(TkStringView arg);
        static TkString normalizeName(TkStringView arg);
        void addOption(TkStringView name, TkStringView value, bool hasValue);
    };
}
