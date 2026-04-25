/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/StringView.hpp"
#include "toolkit/utils/Convertor.hpp"
#include <toolkit/input/InputDevice.hpp>

namespace toolkit
{
    // Represent a single hardware keyboard device. Used by the Input to report raw keyboard input events.
    class TK_API Keyboard : public InputDevice
    {
    protected:
        struct State
        {
            u16 textLen;
            wchar text[32];
            bool keys[(i32)KeyboardKeys::kMax];
            void clear();
        };

        State m_state;
        State m_prevState;
        mutable TkString m_inputTextUtf8;

        explicit Keyboard();

    public:
        // Get the text entered during the current frame (Unicode format).
        TkStringView getInputText() const;

        // Get keyboard key state.
        TK_FORCE_INLINE bool getKey(KeyboardKeys key) const
        {
            if (!isValidKey(key))
                return false;
            return m_state.keys[static_cast<i32>(key)];
        }

        // Get keyboard key down state.
        TK_FORCE_INLINE bool getKeyDown(KeyboardKeys key) const
        {
            if (!isValidKey(key))
                return false;
            return m_state.keys[static_cast<i32>(key)] && !m_prevState.keys[static_cast<i32>(key)];
        }

        // Get keyboard key up state.
        TK_FORCE_INLINE bool getKeyUp(KeyboardKeys key) const
        {
            if (!isValidKey(key))
                return false;
            return !m_state.keys[static_cast<i32>(key)] && m_prevState.keys[static_cast<i32>(key)];
        }

        // Check if any keyboard key is currently pressed.
        bool isAnyKeyDown() const;

    public:
        // Called when keyboard enters input character.
        void onCharInput(wchar c, TkWindow *target = nullptr);
        // Called when key goes up.
        void onKeyUp(KeyboardKeys key, TkWindow *target = nullptr);
        // Called when key goes down.
        void onKeyDown(KeyboardKeys key, TkWindow *target = nullptr);

    public:
        void resetState() override;
        bool update(EventQueue &queue) final override;
    };

}
